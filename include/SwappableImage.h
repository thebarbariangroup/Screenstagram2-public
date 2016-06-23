/*
 *  SwappableImage.h
 *  instagram_ss
 *
 *  Created by dp on 3/1/11.
 *  Copyright 2011 The Barbarian Group. All rights reserved.
 *
 */

/*
 Draw these for the swapping image magic.
 They start out with one image, and can be given a secondary image which will be slide in as needed.
 The main controller passes in the new images, and has to trigger the swap animation.
 */

#pragma once

#include "cinder/app/AppBasic.h"
#include "InstagramImage.h"
#include "InstagramImageFace.h"
#include "cinder/gl/Texture.h"
#include "cinder/Timeline.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class SwappableImage {
public:
	SwappableImage();
	SwappableImage(shared_ptr<InstagramImageFace> image, Vec2f origin, int imageLength, int col, int row);
	void update();
	void draw(int imageLength);
	void swap(shared_ptr<InstagramImageFace> image);
	void startSwapping();
	bool isSwapping();
	void setSlideDirection();
	void finishSwapping();
	void setSlideDirectionDifference();
	void updatePositions();    

	int mImageLength;
	int mEaseDuration;
	shared_ptr<InstagramImageFace> mCurrentImage;
	shared_ptr<InstagramImageFace> mImage1;
	shared_ptr<InstagramImageFace> mImage2;
	bool mIsSwapping;
	int mRow;
	int mCol;
	int mAge;
	int mSwappingAge;
	string mSlideDirection;
	Vec2f mOrigin;
	Vec2f mOriginalOrigin;

    Vec2f mImage1Origin;
    Vec2f mImage2Origin;    
    Vec2f mCurrentImageOrigin;
    
    bool mImage1WasPlaced;
    bool mImage2WasPlaced;    
    bool mCurrentImageWasPlaced;
    
	Vec2f mSlideDirectionDifference;


};

