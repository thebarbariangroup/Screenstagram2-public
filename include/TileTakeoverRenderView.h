//
//  TileTakeoverRenderView.h
//  FlickrTestMultithreaded
//
//  Created by Doug Pfeffer on 4/24/12.
//  Copyright (c) 2012 The Barbarian Group. All rights reserved.
//

#pragma once
#include "RenderView.h"
#include "cinder/Timeline.h"
#include <vector>


class TileTakeoverTile {
public:
    TileTakeoverTile(ci::Vec2f pos, float imageLength, InstagramImageRef image, float offset);
    ci::Vec2f mPos;
    float mImageLength;
    ci::Anim<float> mAlpha;
    bool mIsActive; // Keep track of whether we're actively being visible in some manner or another.
    bool mHasStarted; // Will be true once we've started the drawing process.
    float mOffset;
    InstagramImageRef mImage;
    ci::Rectf mPositionRect;

    bool mHasFadedIn;
    void draw();    
    void start();
    void stop();
    bool hasFinishedFadingOut();    
    bool hasFinishedFadingIn();        
};

class TileTakeoverRenderView : public RenderView {
public:
    TileTakeoverRenderView(InstagramImageRef image, int totalImages, int desiredNumRows, int desiredNumCols, ci::Area viewportArea);
    
    void draw();
    void stop();
    bool hasFinished();
    void start();
    bool canStart();
    bool isReadyForNext();
    void update();
    std::string viewType();    
    int zIndex();    
    bool mIsEnding;
    int mTotalImages;
    bool mHasStarted;
    bool mIsReadyForNext;
    InstagramImageRef mImage;
    std::vector<TileTakeoverTile> mTiles;
    
};


