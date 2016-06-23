/*
 *  InstagramImage.h
 *  instagram_ss
 *
 *  Created by dp on 2/26/11.
 *  Copyright 2011 The Barbarian Group. All rights reserved.
 *
 */

#pragma once

#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Filesystem.h"

#include "FontMeta.h"

#include <iostream>
#include <string>
#include <vector>

class InstagramImage {
public:
	InstagramImage(std::string url, std::string username, std::string favorites, std::string caption, float length, const FontMeta baseFontMeta);
    // TODO - destructor needed to clear out old image file.
    // TODO - or just don't save them to disk!
    
	void setupTexture();
    void downloadImage();
    void draw(int imageLength, ci::Vec2f origin);
    void showUsername();
    void generateUsernameTexture();
    std::shared_ptr<InstagramImage> clone();
    
    ci::fs::path localPath();
    bool canDrawText();
    ci::gl::Texture mUsernameTexture;    
    ci::gl::Texture mImage;
    
    std::string mUrl;
	std::string mUsername;
	std::string mFavorites;
	std::string mCaption;
    ci::Rectf mPositionRect;
    ci::Vec2f mOrigin;
    bool mWasPlaced;
    bool mShowUsername;
    
	int mAge;
    float mLength;
    bool mImageDownloaded;
    int mAppTimeBorn;  // When was this image created, in app-time?
	
  private:
    ci::Surface mSurface;
	FontMeta mBaseFontMeta;
	ci::Rectf mUsernamePixelBounds;
	void getTruePixelBounds( ci::Surface *surf, ci::gl::Texture *tex, ci::Rectf &r );
};
