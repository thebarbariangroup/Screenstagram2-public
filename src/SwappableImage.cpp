/*
 *  SwappableImage.cpp
 *  instagram_ss
 *
 *  Created by dp on 3/4/11.
 *  Copyright 2011 The Barbarian Group. All rights reserved.
 *
 */

#include "cinder/Cinder.h"
#include "SwappableImage.h"
#include "InstagramImage.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"
#include "Back.h"

static const string SLIDE_DIRECTION_FROM_LEFT   = "left";
static const string SLIDE_DIRECTION_FROM_RIGHT  = "right";
static const string SLIDE_DIRECTION_FROM_TOP    = "top";
static const string SLIDE_DIRECTION_FROM_BOTTOM = "bottom";

SwappableImage::SwappableImage() {
}

SwappableImage::SwappableImage(shared_ptr<InstagramImageFace> image, Vec2f origin, int imageLength, int col, int row) {

	mImage1 = image;
	mCurrentImage = image;
	mOrigin = origin; // This will be used to actually draw the image.
	mOriginalOrigin = origin; // This is a record of where we started, so we remember even after we shift around.
	mSwappingAge = 0;	
	mIsSwapping = false;
	mImageLength = imageLength;
	mRow = row;
	mCol = col;
	
	// The speed of the ease is related to the size of the image.
	// If the image is really tiny we want it to go slower.
//	mEaseDuration = mImageLength * 0.20f;
//	if(mImageLength < 80){
//		mEaseDuration = 80;
//	}
    
    mEaseDuration = 160.0f;
}



// Start the fade blast.



void SwappableImage::swap(shared_ptr<InstagramImageFace> i) {
	mImage2 = i;
	startSwapping();
}



void SwappableImage::startSwapping(){
	
	if(isSwapping()){ return; }
	setSlideDirection();
	setSlideDirectionDifference();
	mIsSwapping = true;	
    
	// Set the new image that's gonna be sliding in.
	mImage2->mOrigin = mOrigin + mSlideDirectionDifference;
	mImage2->setWasPlaced(true);
}

void SwappableImage::update(){
	mAge++;

	if(isSwapping()){
		mSwappingAge++;

		if(mEaseDuration == mSwappingAge){
			finishSwapping();
			return;
		}
		
		updatePositions();

	}
	
}


void SwappableImage::updatePositions(){

	// We'll ease the movements of the two images.
	// The beginning and end values depend on from which direction we're sliding.
	
	float newImageEase, oldImageEase;
	Vec2f newImageOrigin, oldImageOrigin;
	
	
	if(mSlideDirection == SLIDE_DIRECTION_FROM_TOP){
		newImageEase = Back::easeInOut(mSwappingAge, mOriginalOrigin.y - mImageLength, mImageLength, mEaseDuration);
		oldImageEase = Back::easeInOut(mSwappingAge, mOriginalOrigin.y, mImageLength, mEaseDuration);					
		newImageOrigin = Vec2f(mImage2->mOrigin.x, newImageEase);
		oldImageOrigin = Vec2f(mImage1->mOrigin.x, oldImageEase);	
	} else if(mSlideDirection == SLIDE_DIRECTION_FROM_BOTTOM){
		newImageEase = Back::easeInOut(mSwappingAge, mOriginalOrigin.y + mImageLength, -1 * mImageLength, mEaseDuration);
		oldImageEase = Back::easeInOut(mSwappingAge, mOriginalOrigin.y, -1 * mImageLength, mEaseDuration);					
		newImageOrigin = Vec2f(mImage2->mOrigin.x, newImageEase);
		oldImageOrigin = Vec2f(mImage1->mOrigin.x, oldImageEase);	
	} else if(mSlideDirection == SLIDE_DIRECTION_FROM_LEFT){
		newImageEase = Back::easeInOut(mSwappingAge, mOriginalOrigin.x - mImageLength, mImageLength, mEaseDuration);
		oldImageEase = Back::easeInOut(mSwappingAge, mOriginalOrigin.x, mImageLength, mEaseDuration);					
		newImageOrigin = Vec2f(newImageEase, mImage2->mOrigin.y);
		oldImageOrigin = Vec2f(oldImageEase, mImage1->mOrigin.y);	
	} else {
		newImageEase = Back::easeInOut(mSwappingAge, mOriginalOrigin.x + mImageLength, -1 * mImageLength, mEaseDuration);
		oldImageEase = Back::easeInOut(mSwappingAge, mOriginalOrigin.x, -1 * mImageLength, mEaseDuration);					
		newImageOrigin = Vec2f(newImageEase, mImage2->mOrigin.y);
		oldImageOrigin = Vec2f(oldImageEase, mImage1->mOrigin.y);	
	}	

 
    
	mImage1->mOrigin = oldImageOrigin;
	mImage2->mOrigin = newImageOrigin; 
    
}


void SwappableImage::finishSwapping(){
	mIsSwapping = false;
	
	mImage1->setWasPlaced(false);	
	mSwappingAge = 0;
	
	mCurrentImage = mImage2;
	mImage1 = mImage2;	

    // Force the current image into the proper place, otherwise rounding errors has it silly.
    mCurrentImage->mOrigin = mOriginalOrigin;

}

bool SwappableImage::isSwapping(){
	return mIsSwapping;
}

void SwappableImage::setSlideDirection(){
	Rand::randomize();
	int i = Rand::randInt( 0, 4 );
	switch(i){
		case 0:
			mSlideDirection = SLIDE_DIRECTION_FROM_TOP;
			break;
		case 1:
			mSlideDirection = SLIDE_DIRECTION_FROM_BOTTOM;
			break;
		case 2:
			mSlideDirection = SLIDE_DIRECTION_FROM_LEFT;
			break;
		case 3:
			mSlideDirection = SLIDE_DIRECTION_FROM_RIGHT;
			break;
	}
}

void SwappableImage::setSlideDirectionDifference(){
	// It's gonna start 1 length away from where we'll have it end, so we can slide it in.
	// What direction that initial length is in depends on what was deteremined in determineSlideDirection().
	
	if(mSlideDirection == SLIDE_DIRECTION_FROM_TOP){
		mSlideDirectionDifference = Vec2f(0, -mImageLength);
	} else if(mSlideDirection == SLIDE_DIRECTION_FROM_BOTTOM){
		mSlideDirectionDifference = Vec2f(0, 2 * mImageLength);
	} else if(mSlideDirection == SLIDE_DIRECTION_FROM_LEFT){
		mSlideDirectionDifference = Vec2f(-mImageLength, 0);
	} else {
		mSlideDirectionDifference = Vec2f(2 * mImageLength, 0);
	}
}

void SwappableImage::draw(int imageLength){

    if(isSwapping()){
        mImage2->draw(imageLength);
    } 
    mCurrentImage->draw(imageLength);

  
    
    gl::color(Color(1, 1, 1));

}
	

	