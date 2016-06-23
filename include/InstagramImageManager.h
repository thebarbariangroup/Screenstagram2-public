//
//  InstagramImageManager.h
//
//  Created by Doug Pfeffer on 4/3/12.
//  Copyright (c) 2012 The Barbarian Group. All rights reserved.
//

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <list>

#include "cinder/Thread.h"
#include "cinder/ConcurrentCircularBuffer.h"

#include "InstagramImage.h"
#include "InstagramImageFace.h"
#include "FontMeta.h"

typedef std::shared_ptr<InstagramImage> InstagramImageRef;

class InstagramImageManager {
public:
    InstagramImageManager(int minRequiredImages, int desiredNumRows, int desiredNumCols, const FontMeta baseFontMeta);
    void parseFeed(std::string feedJson);
    void resize(ci::Area viewportArea);
    void processFeed(std::string feedUrl);
    void processPopularFeed(int count);
    void processUserFeed(int count, std::string token);
    void processLikedFeed(int count, std::string token);    
    void getTaggedPhotos(std::list<std::string> tags, int count, std::string token);
    void downloadImagesThreadFn();
    void shutdown();
    bool hasAlreadyReceivedId(std::string id);
    // Returns whether we have the requested number of images available for display.
    bool hasImagesToDisplay(int numberRequested);
    std::string mErrorMessage;
    bool mFailed, mErrorLoading;
    bool mIsLoading;
    int mDesiredNumRows;
    int mDesiredNumCols;
    int mNumberOfFeedsDownloaded;
    bool mShowUsernames;
    std::list<std::string> mReceivedPhotoIds;
    int paginationLimit;
    int mNumberOfPagesDownloaded;
	int mMaxRetries;
	int mRetryCount;
	int mRetryWaitMilleseconds;
    
    
    // Returns the images we have available for display. The actual images returned is up to this manager.
    // Should check if we have enough first, using bool hasImagesToDisplay().
    std::vector<InstagramImageRef> getImagesForDisplay(int numberRequested);
    
    
    int size(); // Returns number of processed images.
    bool isLoadingImages();
    void update();
    int numberOfFeedsDownloaded();
    bool hadErrorLoading();
	bool loadFailed();
    void processFeedThreadFn();
	void queueRetry(const std::string &url);
    void duplicateExistingImages();
    void showUsernames();
    std::vector<std::shared_ptr<InstagramImageFace> > getImages();
    int mPaginationLimit;
    // Images
    InstagramImageRef getRandomImage();
    void pruneImages();
    
    std::vector<std::shared_ptr<std::thread> > mThreads;
    
    std::shared_ptr<std::thread> mFeedProcessorThread;
    
    float mImageLength;
    bool mShouldQuit;
    int mMinRequiredImages;
    ci::ConcurrentCircularBuffer<std::string>           * mFeedUrls;    
    ci::ConcurrentCircularBuffer<InstagramImageRef>     * mPendingImages;
    ci::ConcurrentCircularBuffer<InstagramImageRef>     * mDownloadedImages;    
    std::list<InstagramImageRef>                        mProcessedImages;

private:
	FontMeta mBaseFontMeta;
};

