//
//  InstagramImageManager.cpp
//
//  Created by Doug Pfeffer on 4/3/12.
//  Copyright (c) 2012 The Barbarian Group. All rights reserved.
//

#include "InstagramImageManager.h"
#include "cinder/json.h"
#include "cinder/Rand.h"
#include "cinder/app/App.h"
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <sstream>
#if defined( CINDER_MAC )
	#include <objc/objc-auto.h>
#endif

using namespace ci;
using namespace ci::app;
using namespace std;

class ParseExc : public Exception {

};

InstagramImageManager::InstagramImageManager(int minRequiredImages, int desiredNumRows, int desiredNumCols, const FontMeta baseFontMeta){
//    mImageLength = (float)viewportArea.getWidth()/(float)desiredNumCols;
    mImageLength = 0; // Temporary.
    mPendingImages = new ConcurrentCircularBuffer<InstagramImageRef>( 10 );
    mDownloadedImages = new ConcurrentCircularBuffer<InstagramImageRef>( 10 );        
    mFeedUrls = new ConcurrentCircularBuffer<std::string>( 5 );
    mShouldQuit = false;
    mMinRequiredImages = minRequiredImages;
    mDesiredNumRows = desiredNumRows;
    mDesiredNumCols = desiredNumCols;
    mFailed = false;
    mErrorLoading = false;
    mNumberOfFeedsDownloaded = 0;
    mMaxRetries = 30;
    mRetryCount = 0;
    mRetryWaitMilleseconds = 1000;
    
    // Indicate how many pages of each we can get initially.
    // Subsequent loads don't need to bother with pagination, so if we're maxed out it doesn't matter.    
    mNumberOfPagesDownloaded = 0;
    mPaginationLimit = 5;



    
    // This thread watches for new feeds to download.
    mFeedProcessorThread = shared_ptr<thread>( new thread(
 bind( &InstagramImageManager::processFeedThreadFn, this ) ));

    // Set up a bunch of threads for downloading images queued up by processing the JSON feeds.
    for(int i = 0; i < 15; i++){
        shared_ptr<thread> newThread( new thread( bind( &InstagramImageManager::downloadImagesThreadFn, this ) ) );
        mThreads.push_back(newThread);
    }    
    mIsLoading = false;
    mShowUsernames = false;
	mBaseFontMeta = baseFontMeta;
}

void InstagramImageManager::showUsernames(){
    mShowUsernames = true;
}

bool InstagramImageManager::hasAlreadyReceivedId(std::string id){
	for(std::list<std::string>::iterator list_iter = mReceivedPhotoIds.begin(); list_iter != mReceivedPhotoIds.end(); list_iter++) {
		if(*list_iter == id){
			return true;
		}
	}
	return false;
}

void InstagramImageManager::processLikedFeed(int count, std::string token){
    
    std::stringstream out;
    string countString;
    out << count;
    countString = out.str();
    
    std::stringstream out2;
    string countString2;
    out2 << token;
    countString2 = out2.str();    
    
    
    string url = "https://api.instagram.com/v1/users/self/media/liked?client_id=c54e61b2f7554ca5bd7ce61bb371cffb&count=" + countString + "&access_token=" + countString2;
    processFeed(url);
    
}

void InstagramImageManager::processUserFeed(int count, std::string token){

    std::stringstream out;
    string countString;
    out << count;
    countString = out.str();
    
    std::stringstream out2;
    string countString2;
    out2 << token;
    countString2 = out2.str();    
    
    
    string url = "https://api.instagram.com/v1/users/self/feed?client_id=c54e61b2f7554ca5bd7ce61bb371cffb&count=" + countString + "&access_token=" + countString2;
    processFeed(url);

}

void InstagramImageManager::processPopularFeed(int count){
    
    std::stringstream out;
    string countString;
    out << count;
    countString = out.str();    
    
    string url = "https://api.instagram.com/v1/media/popular?client_id=c54e61b2f7554ca5bd7ce61bb371cffb&count=" + countString;
    processFeed(url);
}


void InstagramImageManager::getTaggedPhotos(std::list<string> tags, int count, std::string token) {
    
    std::stringstream out;
    string countString;
    out << count;
    countString = out.str();        
    
    std::stringstream out2;
    string countString2;
    out2 << token;
    countString2 = out2.str();  

    for (list<string>::iterator it = tags.begin(); it != tags.end(); it++){
        string t = string(*it);
        boost::algorithm::trim(t);
		//
		// string url = "https://api.instagram.com/v1/tags/" + t + "/media/recent?client_id=c54e61b2f7554ca5bd7ce61bb371cffb&count=" + countString + "&access_token=" + countString2;
		//
		// Hey - 't' here is null terminated, at least when only a single tag was specified. \0 breaks normal URL concat after the tag;
		// Broken URLs will STB when missing OAuth params and count. The workaround below uses strcat to build the URL:
		//
		char url[500];
		strcpy( url, "https://api.instagram.com/v1/tags/" );
		strcat( url, t.c_str() );
		strcat( url, "/media/recent?client_id=c54e61b2f7554ca5bd7ce61bb371cffb&count=" );
		strcat( url, countString.c_str() );
		strcat( url, "&access_token=" );
		strcat( url, countString2.c_str() );

		processFeed(url);
    }
}

// Used when we determine that the number of images downloaded is too few.
// In those cases we'll just duplicate what we've got until we're at the right size.
void InstagramImageManager::duplicateExistingImages(){

    int amountToClone = mProcessedImages.size();
    int j = 0;
    for (list<InstagramImageRef>::iterator it = mProcessedImages.begin(); it != mProcessedImages.end(); it++){
        InstagramImageRef i = it->get()->clone();
        mProcessedImages.push_back(i);
        j++;
        if(j > amountToClone){ break; }
    }        
    
}

int InstagramImageManager::numberOfFeedsDownloaded(){
    return mNumberOfFeedsDownloaded;
}

// This is called from a non-primary thread to pull in feeds that we line up.
void InstagramImageManager::processFeedThreadFn(){
#if defined( CINDER_MAC )
	objc_registerThreadWithCollector();
#endif    
    ci::ThreadSetup threadSetup;
    mRetryCount = 0;
    while( ! mShouldQuit ) {
        std::string url;
        mFeedUrls->popBack( &url );

        if( mShouldQuit ) // popBack might have returned if the CCB was canceled, in which case mShouldQuit will be true
            return;
        try {
            string feedJson = loadString(loadUrl(url));
            parseFeed(feedJson);
            mNumberOfFeedsDownloaded++;
        }
        catch(UrlLoadExc &exc) {
            app::console() << "UrlLoadExc, url: " << url << std::endl;
            app::console() << "code: " << exc.statusCode() << ", what: " << exc.what() << std::endl;
            queueRetry(url);
        }
        catch(ParseExc &exc) {
            app::console() << "ParseExc, url: " << url << std::endl;
            queueRetry(url);
        }
        catch( ... ) {
            app::console() << "Exception loading " << url << std::endl;
        }
    }
}

void InstagramImageManager::queueRetry(const std::string &url)
{
    if( mRetryCount < mMaxRetries ) {
        mErrorLoading = true;
        mErrorMessage = "There was a problem loading data from Instagram, retrying..";
        app::console() << "retries: " << mRetryCount << endl;
        ++mRetryCount;
        ci::sleep( mRetryWaitMilleseconds );
        mFeedUrls->pushFront( url );
    }
    else {
        mFailed = true;
        mErrorMessage = "There was a problem loading data from Instagram.";
        app::console() << "Failed to load data from Instagram." << endl;
    }
}

// Removed older images, if we have have any to spare.
void InstagramImageManager::pruneImages(){
    const int imageBuffer = 15; // Need to keep some extra around for swapping.
    while( mProcessedImages.size() > mMinRequiredImages + imageBuffer ){
        
        if( mProcessedImages.front()->mWasPlaced ){ // TODO - this logic should not be here, since it's bound too tightly to the images themselves. Should ask a middle man.
            mProcessedImages.pop_front();    
        } else {
//            console() << "Could not pop placed image" << endl;
            break;
        }
    }
}


std::vector<shared_ptr<InstagramImageFace> > InstagramImageManager::getImages() {
    // Generate a InstagramImageFace for each image, and pass that back.
    std::vector<shared_ptr<InstagramImageFace> > faces;
    
    list<InstagramImageRef>::iterator it;        
    for ( it=mProcessedImages.begin() ; it != mProcessedImages.end(); it++ ){
        InstagramImageRef i = *it;
        shared_ptr<InstagramImageFace> f(new InstagramImageFace(i));
        
        faces.push_back(f);
    }    
    
    return faces;
}

// Called from non-primary threads to download images we've got queued up by URL.
void InstagramImageManager::downloadImagesThreadFn(){
#if defined( CINDER_MAC )
	objc_registerThreadWithCollector();
#endif    
	ci::ThreadSetup threadSetup;
    while( ! mShouldQuit ) {
		InstagramImageRef i;
		mPendingImages->popBack( &i );
		if( mShouldQuit ) // popBack might have returned if the CCB was canceled, in which case mShouldQuit will be true
			return;
		i->downloadImage();
		if( i->mImageDownloaded ) {
			mDownloadedImages->pushFront(i);                
		}
    }
}

bool InstagramImageManager::isLoadingImages(){
    return mPendingImages->isNotEmpty() || mDownloadedImages->isNotEmpty();
}

void InstagramImageManager::update(){
	pruneImages();
    // Periodically chop off a few downloaded images to turn into textures.
    if(mDownloadedImages->isNotEmpty()){
        InstagramImageRef i;
        mDownloadedImages->popBack(&i);
        i->setupTexture();
        mProcessedImages.push_back(i);
    }
}

bool InstagramImageManager::hasImagesToDisplay(int numberRequested){
//    console() << "mProcessedImages.size: " << mProcessedImages.size() << endl;
    return mProcessedImages.size() >= numberRequested;
}

std::vector<InstagramImageRef> InstagramImageManager::getImagesForDisplay(int numberRequested) {
    // For now just return the last X images.
    // TODO - sort by number of times shown and newest first?
    // Or somehow favor the images that were shown the least # of times?
    // For now just get a random chunk.
    std::vector<InstagramImageRef> imagesForDisplay;
    for(int i = 0; i < numberRequested; i++){
        
        list<InstagramImageRef>::iterator it;        
        for ( it=mProcessedImages.begin() ; it != mProcessedImages.end(); it++ ){
            imagesForDisplay.push_back(*it);    
        }
    }
    
    random_shuffle ( imagesForDisplay.begin(), imagesForDisplay.end() );
    return imagesForDisplay;
}

int InstagramImageManager::size(){
    return mProcessedImages.size();
}

InstagramImageRef InstagramImageManager::getRandomImage(){
    srand ( time(NULL) );
    Rand::randomize();
    int idx = Rand::randInt(0, mProcessedImages.size());
    int i = 0;
    list<InstagramImageRef>::iterator it;        
    for ( it=mProcessedImages.begin() ; it != mProcessedImages.end(); it++ ){    
        i++;
        if(i == idx){
            return *it;
        }
    }
    return mProcessedImages.back();
}

void InstagramImageManager::processFeed(string feedUrl){
            console() << Url::encode(feedUrl) << endl;
    mFeedUrls->pushFront(Url::encode(feedUrl));
}


// Parses a given string of feed JSON and creates new InstagramImages, queueing the up for downloading.
void InstagramImageManager::parseFeed(string feedJson){
    
    // Are we in the process of quitting? Bail.
    if(mShouldQuit){return;}
    
    try {
        ci::JsonTree doc( feedJson );
        
        if(doc.hasChild("meta")){
            ci::JsonTree meta = doc.getChild( "meta" );
            if(meta.hasChild("error_type")){
                mFailed = true;
                mErrorMessage = "There was a problem loading photos from Instagram. It said \"" + meta.getChild("error_message").getValue() + "\".";
                return;
                
            }
        }

        if(doc.hasChild("pagination")) {
            ci::JsonTree pagination = doc.getChild("pagination");
            console() << "pagination" << endl;
            if(pagination.hasChild("next_url") && mNumberOfPagesDownloaded <= mPaginationLimit){
                console() << "Asking for next page..." << endl;
                // Get the next page.
                processFeed(pagination.getChild("next_url").getValue());
                mNumberOfPagesDownloaded++;
            }
        } else {
            // No pagination? The popular feed doesn't offer pagination, nor does it allow a count.
            // So we'll just request it again.
        }
        
        ci::JsonTree photos = doc.getChild( "data" );
        for( ci::JsonTree::ConstIter cIt = photos.begin(); cIt != photos.end(); ++cIt )
        {
            std::string c_username;
            std::string c_url;
            std::string c_likes = "0";
            std::string c_caption = "";
            
            // Ignore photos we already have.
            if(hasAlreadyReceivedId((*cIt)["id"].getValue())){
                continue;
            }
            
            if((*cIt)["caption"].hasChildren()){
                c_caption = (*cIt)["caption.text"].getValue();    
            }
            
            if((*cIt)["user"].hasChildren()){
                c_username = (*cIt)["user.username"].getValue();    
            }        
            
            if((*cIt)["likes"].hasChildren()){
                c_likes = (*cIt)["likes.count"].getValue();    
            }
            
            if((*cIt)["images"].hasChildren()){
                c_url = (*cIt)["images.low_resolution.url"].getValue();    
            }

            InstagramImageRef image(new InstagramImage(c_url, c_username, c_likes, c_caption, mImageLength, mBaseFontMeta));
            if(mShowUsernames){
                image->showUsername();
            }

            mReceivedPhotoIds.push_back((*cIt)["id"].getValue());
            mPendingImages->pushFront(image);
        }
    } catch ( ci::JsonTree::ExcJsonParserError ex ) {
        console() << "INSTAGRAM JsonTree::ExcJsonParserError" << endl;
        throw ParseExc();
    } catch ( ci::JsonTree::ExcChildNotFound ex ){
        console() << "INSTAGRAM PROBLEM JsonTree::ExcChildNotFound" << endl;  
        console() << feedJson << endl;
        throw ParseExc();
    }  catch ( ... ){
        mFailed = true;
        mErrorMessage = "Could not connect to Instagram.";        
        console() << "Some other error" << endl;
        throw ParseExc();
    }
}

bool InstagramImageManager::hadErrorLoading(){
    return mErrorLoading;
}

bool InstagramImageManager::loadFailed() {
    return mFailed;
}


void InstagramImageManager::shutdown(){
	mShouldQuit = true;
    mFeedUrls->cancel();
    mPendingImages->cancel();
    mDownloadedImages->cancel();
    // This was really slowing down shutdown.
    //    mFeedProcessorThread->join();
    //    for(int i = 0; i < mThreads.size(); i++){
    //        mThreads[i]->join();
    //    }
    
}