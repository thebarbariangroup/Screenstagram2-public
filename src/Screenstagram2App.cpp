#include "cinder/Cinder.h"
#include "cinder/app/AppScreenSaver.h"
#include "cinder/gl/Texture.h"
#include "cinder/Xml.h"
#include "cinder/Timeline.h"
#include "cinder/ImageIo.h"
#include "cinder/Thread.h"
#include "cinder/ConcurrentCircularBuffer.h"

#include "FontMeta.h"
#include "Config.h"
#include "Resources.h"
#include "InstagramImageManager.h"
#include "LoaderView.h"


#include "WindowData.h"

#include <deque>

#include <boost/algorithm/string.hpp>

#include "Resources.h"
#include "Configuration.h"

#if defined( CINDER_MAC )
	#include "MacConfigDialog.h"
#elif defined( CINDER_MSW )
	// Link and options to enable Msw visual styles:
	#pragma comment( lib, "ComCtl32.lib" ) 
	#pragma comment( linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' \
		version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"" )

	#include "MswVersion.h"
	#include "MswConfigDialog.h"	
#endif

using namespace ci;
using namespace ci::app;
using namespace std;

static const int MINIMUM_ROWS       = 5;
static const int MINIMUM_COLS       = 8;
static const int REQUIRED_IMAGES    = (MINIMUM_ROWS * MINIMUM_COLS) + 5; // Get a few more than required to fill the view so we can animate some swapping.

struct FontMeta PlatformBaseFont = {
#if defined( CINDER_MAC )
	"Helvetica Bold", 10
#elif defined( CINDER_MSW )
	"Arial", 17
#endif
};

class Screenstagram2App : public AppScreenSaver {
 public:
	virtual void prepareSettings( Settings *settings );    
	virtual void setup();
	virtual void update();
	virtual void draw();
    virtual void resize();
#if defined( CINDER_MAC )
	virtual NSWindow* createMacConfigDialog() override {
		return getConfigDialogMac( this, &mConfig ); // defined in MacConfigDialog.cpp
	}
#elif defined( CINDER_MSW )
	static BOOL doConfigureDialog( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) {
		return getConfigDialogMsw( hDlg, message, wParam, lParam ); // MswConfigDialog
	}
#endif
	
//    void	updateSSDefaults();	    
	void shutdown();
	void loadImagesThreadFn();
    void doubleUpImagesIfNecessary();

    void loadFeed(int count);
    void postSetup();
    void updateFeeds(int count);
    
	double                                      mLastImageCheckTime; // Used to keep track of when we last checked for newly downloaded Instagram images.

    double                                      mLastFeedUpdateTime;
    bool                                        mHasStartedDrawingImages;    
    
    bool                                        mHasPerformedPostSetup;
    bool                                        mHasResizedOnce;

    InstagramImageManager                       * mInstagramImageManager;
    LoaderView                                  * mLoaderView;

    WindowData                                  * mWindowData;


    
	// Periodically we'll check how many images we've downloaded.
	// When we do, record that count along with the number of seconds elapsed since the app started.
	// This way we can tell if we're bogging down.
	// The first key is the second, the second is the number of images.
	map<int, int> mImageCountsAtSecond;
    gl::Texture mPreviewImage;
    
 protected:
	Configuration	mConfig;    
};


void Screenstagram2App::prepareSettings( Settings *settings )
{
#if defined( CINDER_MAC )
	settings->setProvidesMacConfigDialog();
#endif
	settings->setFrameRate( 60 );
#if defined( _DEBUG ) && defined( CINDER_MSW )
	settings->enableDebug();
#endif
}





void Screenstagram2App::resize(){
    mHasResizedOnce = true;

    mWindowData = new WindowData(app::getWindowBounds(), MINIMUM_COLS, MINIMUM_ROWS, REQUIRED_IMAGES, mInstagramImageManager);
    getWindow()->setUserData(mWindowData);

}


void Screenstagram2App::setup()
{
    mHasResizedOnce = false;
    
    // For each screen we need to create a ScreenData object.
    

#if defined( CINDER_MAC )
	loadConfigMac( this, &mConfig );
#else
	loadConfigMsw( &mConfig );
#endif
        
    mHasPerformedPostSetup = false; // There are some setup things that should be done there, but due to the screwy screensaver engine we can't, we need to do in update().

    console() << "mUsername:"                   << mConfig.username << endl;
    console() << "mShowUsernames:"              << mConfig.showUsernames << endl;    
    console() << "mConfig.includePopularPhotos:"       << mConfig.includePopularPhotos << endl;        
    console() << "mIncludeTaggedPhotos:"        << mConfig.includeTaggedPhotos << endl;                
    console() << "mPhotoTags:"                  << mConfig.photoTags << endl;
    console() << "onlyIncludeLikedPhotos: " << mConfig.onlyIncludeLikedPhotos << endl;

    postSetup();
    resize();   
}




// Let's run a timer to see if we still don't have enough photos from Instagram after we start loading.
// If after a certain amount of time we do not we'll double up.
void Screenstagram2App::doubleUpImagesIfNecessary(){
    // Only go ahead when we've downloaded enough images.
	// Don't scan the file system on every frame update, every X seconds.
	if(mInstagramImageManager->numberOfFeedsDownloaded() > 0 && !mLoaderView->isDone() && (int)app::getElapsedFrames() != 0 && (int)app::getElapsedFrames() % 20 == 0){ 
		int processedImageCount = mInstagramImageManager->size();

		// Record the second and the number of images we've got so far:
		mImageCountsAtSecond[(int)app::getElapsedSeconds()] = processedImageCount;
		
		// BUT, if we've been waiting for X seconds and the downloaded count hasn't changed, and we're still short,
		// just dupe the existing images.
		// How to tell how long we've been waiting? 
        
		// What's the last element's second?
        
		map<int,int>::iterator lastRecordedSecondIterator = mImageCountsAtSecond.end();
		--lastRecordedSecondIterator;
		int lastRecordedCount = (*lastRecordedSecondIterator).second;
        
		// See how many of those counts exist.
		int numberofInstancesOfLastSecondFound = 0;
		
		map<int,int>::iterator it;
		for ( it=mImageCountsAtSecond.begin(); it != mImageCountsAtSecond.end(); it++ ){
			if( (*it).second == lastRecordedCount ){
				numberofInstancesOfLastSecondFound++;
			}
		}
        
		// If we're still here and we're hanging on 5 seconds of the same number of images that are here,
		// push through and dupe them.
		// This is assuming the number of images already loaded is not 0.
		// If it is it means the feeds failed to load, and that's a whole different story.
		if(numberofInstancesOfLastSecondFound > 5){
			if(lastRecordedCount == 0){
				mLoaderView->setStatusMessage("Unable to load Instagram data!");
				mLoaderView->loadingFailed();
			} else {
                console() << "main update() wants to [dataManager duplicateExistingImages]" << endl;
				mLoaderView->setStatusMessage("Too few images were available for load. Fear not, we'll double up!");
                while(mInstagramImageManager->size() < REQUIRED_IMAGES){
                    mInstagramImageManager->duplicateExistingImages();    
                }
                
			}
		}
	}
}

// There are some setup things that should be done there, but due to the screwy screensaver engine we can't, we need to do in draw().
// Most importantly we can't get the viewport size there.
void Screenstagram2App::postSetup(){
    mHasStartedDrawingImages    = false;
    mInstagramImageManager      = new InstagramImageManager(REQUIRED_IMAGES, MINIMUM_ROWS, MINIMUM_COLS, PlatformBaseFont );
    if(mConfig.showUsernames){
        mInstagramImageManager->showUsernames();
    }
    
        
    // TODO - check if we have popular only, or tags, or authenticated.
    // if just popular (not authenticated) we'll know to dupe the images right away.
    
//    mInstagramImageManager->processPopularFeed(REQUIRED_IMAGES);
    
    updateFeeds(REQUIRED_IMAGES);
    
    //    mInstagramImageManager->processFeed("https://api.instagram.com/v1/media/popular?client_id=c54e61b2f7554ca5bd7ce61bb371cffb");    
    //    mInstagramImageManager->processFeed("https://api.instagram.com/v1/tags/dog/media/recent?count=40&client_id=c54e61b2f7554ca5bd7ce61bb371cffb");    
	
    mLastImageCheckTime = getElapsedFrames();
    mLastFeedUpdateTime = getElapsedFrames();
    
    
	double dVersion = 0.0;
#if defined( CINDER_MAC )
    NSString * version = [[[NSBundle bundleWithIdentifier:@"com.barbariangroup.screenstagram"] infoDictionary] objectForKey:@"CFBundleVersion"];
    dVersion = [version doubleValue];
#elif defined( CINDER_MSW )	&& defined( GET_MSW_VERSION )
	dVersion = atof( GET_MSW_VERSION );
#endif

    mLoaderView = new LoaderView("Loading photos", dVersion, PlatformBaseFont);
    mLoaderView->start();
    
    if(!mConfig.username.empty()){
        std::string msg = "Loading content for";
        msg = msg + " " + mConfig.username;
        mLoaderView->setStatusMessage(msg);
    } else {
        mLoaderView->setStatusMessage("Loading content");    
    }
    
    mHasPerformedPostSetup = true; // Don't want to do this again.
    
}


void Screenstagram2App::update()
{
    
    if(!mPreviewImage){
        mPreviewImage = loadImage( loadResource( RES_PREVIEW_JPG ) );        
    }
        
    // Let's run a timer to see if we still don't have enough photos from Instagram after we start loading.
    // If after a certain amount of time we do not we'll double up.
    doubleUpImagesIfNecessary();    
    
    if(!mLoaderView->isDone()){
        if(mInstagramImageManager->loadFailed()){
            mLoaderView->setStatusMessage(mInstagramImageManager->mErrorMessage);
	        mLoaderView->loadingFailed();
        }
    }
    
	double timeSinceLastImage = getElapsedFrames() - mLastImageCheckTime;
	if( timeSinceLastImage > 2 ) {
        mInstagramImageManager->update();
        mLastImageCheckTime = getElapsedFrames();
    }	
        
    double timeSinceLastFeedUpdate = getElapsedFrames() - mLastFeedUpdateTime;
    if(timeSinceLastFeedUpdate > 60 * 50){
        console() << "GETTING NEW PHOTOS" << endl;
        updateFeeds(15); // need much fewer to just update.
        mLastFeedUpdateTime = getElapsedFrames();
    }    

}

void Screenstagram2App::draw()
{

    
    WindowData *data = getWindow()->getUserData<WindowData>();
    
    // Can we start drawing? If so toggle that bool so we know what the current state of things is.
    // We'll never revert to not being able to draw.
    if(data->mRenderViews.size() == 0 && mInstagramImageManager->hasImagesToDisplay(REQUIRED_IMAGES)){
        mHasStartedDrawingImages = true;
        // Start us off witht he regular render view.
        data->addNewRenderView();
        
        if(!mLoaderView->isDone()){ mLoaderView->stop(&timeline()); }
    }    
    
    if(mLoaderView->isDone()){
        data->update();
    }
    
    if(isPreview()){
        gl::draw(mPreviewImage, getWindowBounds());
        return;
    }
    
    
    gl::enableAlphaBlending();

	gl::clear(Color(0, 0, 0));
    
    data->draw();
    
    if(mLoaderView->isDone()){    
        
    } else {
        mLoaderView->draw(app::getWindowBounds());
    }
}


void Screenstagram2App::updateFeeds(int count){

	// Liked, exclusive:
    if(mConfig.onlyIncludeLikedPhotos && !mConfig.accessToken.empty()){
        mInstagramImageManager->processLikedFeed(count, mConfig.accessToken);    
        return;       
    }    
    
	// Tagged, exclusive:
    if(mConfig.includeTaggedPhotos && !mConfig.accessToken.empty()){
        // The count param tends to not return as much as it should.
        // So we need to start high.
        int numberOfTaggedPhotos = 90;
        if(!mConfig.photoTags.empty()){
            // Split the tags by comma, since we store it as a CSV string.
            list<string> tags;
            boost::split( tags, mConfig.photoTags, boost::is_any_of( "," ) );
            // Split the number of photos by the number of tags.
            mInstagramImageManager->getTaggedPhotos(tags, numberOfTaggedPhotos/tags.size(), mConfig.accessToken);
            // Id you're including tagged photos that's all you get.
            return;
        } 
		// No tags? Continue to default...
    }    
    
	// Default, with/without token and popular inclusion flag:
    if(!mConfig.accessToken.empty()){
        mInstagramImageManager->processUserFeed(count, mConfig.accessToken);    
    }
    if(mConfig.accessToken.empty()){
        // Triple the fun, since they don't offer pagination or increasing the count here.
        mInstagramImageManager->processPopularFeed(count);
        mInstagramImageManager->processPopularFeed(count);
        mInstagramImageManager->processPopularFeed(count);
    }
}

void Screenstagram2App::shutdown()
{
    mInstagramImageManager->shutdown();
}


CINDER_APP_SCREENSAVER( Screenstagram2App, RendererGl )
