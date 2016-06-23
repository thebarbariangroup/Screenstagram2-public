//
//  InstagramImageFace.h
//  Screenstagram2
//
//  Created by Doug Pfeffer on 10/11/12.
//  Copyright (c) 2012 The Barbarian Group. All rights reserved.
//
/*
 
 Sort of the public face (e.g InstagramImagFace) of an instagram image.
 InstagramImages maintain info about that image (the texture, instagram meta data),
 and this references that but also manages draw states for it.
 
*/


#pragma once
class InstantImageManager;

#include "InstagramImage.h"
#include "cinder/app/App.h"
typedef std::shared_ptr<InstagramImage> InstagramImageRef;

class InstagramImageFace {
public:
	InstagramImageFace(InstagramImageRef i);
    
    InstagramImageRef mInstagramImage;
    ci::Rectf mPositionRect;
    ci::Vec2f mOrigin;
    bool mWasPlaced;    
    
    void setWasPlaced(bool wasPlaced);
    void draw(int imageLength);
    
};
