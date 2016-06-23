//
//  TileTakeoverRenderView.cpp
//  FlickrTestMultithreaded
//
//  Created by Doug Pfeffer on 4/24/12.
//  Copyright (c) 2012 The Barbarian Group. All rights reserved.
//

#include "TileTakeoverRenderView.h"
#include "cinder/Timeline.h"
#include "cinder/app/App.h"

using namespace ci;
using namespace ci::app;
using namespace std;



struct TileFadeInFinished { // Used to flag a TileTakeoverTile as having finished fading out.
    TileFadeInFinished( TileTakeoverTile * tile ) : mTile( tile ) {}
    void operator()(){ 
        mTile->mHasFadedIn = true;
    }
    TileTakeoverTile * mTile;
};


struct TileFadeOutFinished { // Used to flag a TileTakeoverTile as having finished fading out.
    TileFadeOutFinished( TileTakeoverTile * tile ) : mTile( tile ) {}
    void operator()(){ 
        mTile->mIsActive = false;
    }
    TileTakeoverTile * mTile;
};


TileTakeoverTile::TileTakeoverTile(Vec2f pos, float imageLength, InstagramImageRef image, float offset){
    mPos = pos;
    mHasFadedIn = false;
    mImageLength = imageLength;
    mAlpha = 0.0f;
    mImage = image;
    mOffset = offset;
    mPositionRect = Rectf(mPos, mPos + Vec2f(imageLength, imageLength));
    mAlpha = 0.0f;
    mIsActive = true;
    mHasStarted = false;
}

void TileTakeoverTile::start(){
    mHasStarted = true;
    timeline().apply(&mAlpha, 0.0f, 1.0f, 0.65f, EaseInQuad()).delay(mOffset).finishFn(TileFadeInFinished(this));
}

void TileTakeoverTile::stop(){
    timeline().appendTo(&mAlpha, 1.0f, 0.0f, 0.65f, EaseInQuad()).delay(mOffset).finishFn(TileFadeOutFinished(this));    
}

bool TileTakeoverTile::hasFinishedFadingIn(){
    return mHasFadedIn;
}

bool TileTakeoverTile::hasFinishedFadingOut(){
    return !mIsActive && mHasStarted;
}

void TileTakeoverTile::draw(){
    gl::color(ColorA(1, 1, 1, mAlpha));
    gl::draw(mImage->mImage, mPositionRect);
    gl::color(Color(1, 1, 1));    
}

TileTakeoverRenderView::TileTakeoverRenderView(InstagramImageRef image, int totalImages, int desiredNumRows, int desiredNumCols, ci::Area viewportArea) : RenderView(desiredNumRows, desiredNumCols, viewportArea) {
    float offset = 0.0f; // This will determine the delay if fading in any given tile.
    mTotalImages = totalImages;
    mHasStarted = false;
    mIsEnding = false;
    mIsPhasedIn = false;
    // Track the current row and col we're on.
	int currentRow = 0;
	int currentCol = 0;
	
    for (int i = 0; i < mTotalImages; i++){
        
        Vec2f newOrigin = determineCoordsForColAndRow(currentCol, currentRow, mImageLength);
        
		// If we're gonna go off the bottom of the screen quit, we don't need to place anymore.
		if(newOrigin.y >= mViewportArea.getHeight()){
			break;
		}		
 

//        // alternate animation from AFB.
//		offset = ( ( currentRow % 2 == 0 ) ? ( currentCol * 0.15f ) : ( ( desiredNumCols - currentCol ) * 0.15f ) ) + currentRow * 0.5f;
        // I liked this but people (2, anyway) thought it looked buggy. too different than the other effects? - dp

        TileTakeoverTile t(newOrigin, mImageLength, image, offset );
        mTiles.push_back(t);                
				
		currentCol++;
		
		// We need to check if the next image we'll place will fall off the side of the screen.
		Vec2f nextOrigin = determineCoordsForColAndRow(currentCol, currentRow, mImageLength);
        
		// Gonna go off the right edge? Bump it to the next line.
		if(nextOrigin.x >= mViewportArea.getWidth()){
			currentRow++;
			currentCol = 0;
		}

        offset += 0.05f;
	}

}

bool TileTakeoverRenderView::canStart(){
    return !mHasStarted;
}

bool TileTakeoverRenderView::hasFinished(){
    for(int y = 0; y < mTiles.size(); y++){
        if(!mTiles[y].hasFinishedFadingOut()){
            return false;
        }
    }
    return true;
}

std::string TileTakeoverRenderView::viewType(){
    return "TileTakeoverRenderView";
}

int TileTakeoverRenderView::zIndex(){
    return 1;
}

bool TileTakeoverRenderView::isReadyForNext(){
    return mIsPhasedIn;
}

void TileTakeoverRenderView::update(){
    
    // Don't bother if we're already on our way out. 
    if(mIsEnding){ return; }
    
    // If we're all faded in, wait a few then we'll leave.
    bool isEveryoneFadedIn = true;
    for(int y = 0; y < mTiles.size(); y++){
        if(!mTiles[y].hasFinishedFadingIn()){
            isEveryoneFadedIn = false;
        }
    }
    if(isEveryoneFadedIn){
        mIsPhasedIn = true;
        mIsEnding = true;
        for(int y = 0; y < mTiles.size(); y++){    
            mTiles[y].stop();
        }
    }
}

void TileTakeoverRenderView::stop(){
    
}

void TileTakeoverRenderView::start(){
    for(int y = 0; y < mTiles.size(); y++){
        mTiles[y].start();
    }        
    mHasStarted = true;
}

void TileTakeoverRenderView::draw(){
    for(int y = 0; y < mTiles.size(); y++){
        mTiles[y].draw();
    }    
}