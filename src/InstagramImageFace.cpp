//
//  InstagramImageFace.cpp
//  Screenstagram2
//
//  Created by Doug Pfeffer on 10/11/12.
//  Copyright (c) 2012 The Barbarian Group. All rights reserved.
//

//#include "InstagramImageManager.h"
#include "InstagramImageFace.h"

using namespace ci;
using namespace ci::app;
using namespace std;

InstagramImageFace::InstagramImageFace(InstagramImageRef instagramImage){
	mWasPlaced = false;
    mInstagramImage = instagramImage;
}

void InstagramImageFace::draw(int imageLength){
    mInstagramImage->draw(imageLength, mOrigin);
}

void InstagramImageFace::setWasPlaced(bool wasPlaced) {
    mWasPlaced = wasPlaced;
    mInstagramImage->mWasPlaced = wasPlaced;
}