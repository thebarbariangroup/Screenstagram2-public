///*
// *  InstagramImage.cpp
// *  instagram_ss
// *
// *  Created by dp on 2/26/11.
// *  Copyright 2011 The Barbarian Group. All rights reserved.
// *
// */
//

#include "InstagramImage.h"
#include "FontMeta.h"
#include "cinder/Text.h"
#include "cinder/ip/Resize.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/app/App.h"

using namespace ci;
using namespace ci::app;
using namespace std;

InstagramImage::InstagramImage(string url, string username, string favorites, string caption, float length, const FontMeta baseFontMeta){
	mUrl = url;
	mUsername = username;
	mFavorites = favorites;

	mCaption = caption;
    mAge = 0;
    mLength = length;
    mImageDownloaded = false;
    mAppTimeBorn = app::getElapsedSeconds();
    mWasPlaced = false;
    mShowUsername = false;
	mBaseFontMeta = baseFontMeta;
}

std::shared_ptr<InstagramImage> InstagramImage::clone() {
    shared_ptr<InstagramImage> i(new InstagramImage(mUrl, mUsername, mFavorites, mCaption, mLength, mBaseFontMeta));
    i->mImage = mImage;
    i->mSurface = mSurface;
    i->mShowUsername = mShowUsername;
	i->mUsernamePixelBounds = mUsernamePixelBounds;

    if(mShowUsername){
        i->mUsernameTexture = mUsernameTexture;
    }
    return i;
}

void InstagramImage::showUsername(){
    if(!canDrawText()){return;}
    mShowUsername = true;
}

void InstagramImage::generateUsernameTexture(){
    TextLayout layout;
	Font baseFont = Font( mBaseFontMeta.family, mBaseFontMeta.size );
    layout.setColor(ColorA(110.0f/255.0f, 110.0f/255.0f, 110.0f/255.0f, 1.0f));
	layout.setLeadingOffset(0.0);
	layout.setFont( baseFont );
	layout.addLine( mUsername );		
	Surface usernameSurface = layout.render(true, false);	
	mUsernameTexture = gl::Texture( usernameSurface );
	// Parse and store true pixel bounds of username text for better rendering
	getTruePixelBounds( &usernameSurface, &mUsernameTexture, mUsernamePixelBounds );
}

bool InstagramImage::canDrawText(){
    return true;
	return mLength >= 150;
}

void InstagramImage::draw(int imageLength, Vec2f origin){
    Rectf mPositionRect = Rectf(origin, origin + Vec2f(imageLength, imageLength));
	gl::draw(mImage, mPositionRect);
    mImage.unbind();    
	
    if(mShowUsername && canDrawText()){
		// Margins, in pixels
		float xPad = 9.0f, yPad = 7.0f;
		// Copy text pixel bounds for background, offset by true area upper left
		Rectf bg( mUsernamePixelBounds );
		bg.offset( -bg.getUpperLeft() );
		// Move background to bottom left of image
		bg.offset( origin + Vec2f( 0.0f, float(imageLength)-bg.getHeight() ) );
		// Enlarge background by defined margins on all sides
		bg.y1 -= yPad * 2.0f;
		bg.x2 += xPad * 2.0f;
		// Find upper left offset between background and true area
		Vec2f trueOffset( bg.getUpperLeft() - mUsernamePixelBounds.getUpperLeft() );
		// Draw background and center username text by offset and left/top margin
		gl::color( ColorA( 0.0f, 0.0f, 0.0f, 0.9f ));
		gl::drawSolidRect( bg );
		gl::color(ColorA( 1.0f, 1.0f, 1.0f, 1.0f ));
		gl::draw( mUsernameTexture, trueOffset + Vec2f( xPad, yPad ) );
    }
    
}

// Downloads the image and resizes it.
// Takes into account the local disk cache. If it exists there we'll return that.
void InstagramImage::downloadImage(){
  
    
    try {
        mSurface = loadImage( loadUrl(mUrl) );
        mImageDownloaded = true;            
//        console() << mUrl << endl;
    } catch (...){
        // Bah.
        console() << "Error downloading " << mUrl << endl;
        mImageDownloaded = false;
    }

    
//    try {
//        mSurface = loadImage(fs::path(localPath()));
//    } catch (ImageIoExceptionFailedLoad e) {
////        try {
//            mSurface = loadImage( loadUrl(mUrl ) );
//    console() << getTemporaryDirectory() << endl;
            // Write the file to disk.
//            DataTargetRef p = writeFile(fs::path(localPath()), true);
//            writeImage(p, mSurface);            
////        } catch (...){
////            mImageDownloaded = false;            
////            return;
////        }
//    }
    
}

// Returns the path to the local file for this image, whether it exists or not.
fs::path InstagramImage::localPath(){
    // We have the URL, what's the filename?
    string baseFilename = getPathFileName(mUrl);
    return getTemporaryDirectory() / "images" / baseFilename;
}

void InstagramImage::setupTexture(){    
	mImage = gl::Texture( mSurface );	
    if(mShowUsername && canDrawText()){
        generateUsernameTexture();
    }
	// we should be done with the surface now that it's a texture
	mSurface.reset();
}

// Uses alpha channel to fetch true pixel boundaries of given texture.
// https://gist.github.com/936110
//
void InstagramImage::getTruePixelBounds( Surface *surf, gl::Texture *tex, cinder::Rectf &r )
{	
	const Area a = tex->getCleanBounds();
	
	r.x1 = a.x2;
	r.y1 = a.y2;
	r.y2 = a.y1;
	r.x2 = a.x1;

	Surface::Iter iter = surf->getIter( a );

	while( iter.line() ) {
		while( iter.pixel() ) {
			float x = 0.0f, y = 0.0f;

			if ( iter.a() > 0 ) {
				x = iter.getPos().x;
				y = iter.getPos().y;
				
				r.x1 = min( x, r.x1 );
				r.y1 = min( y, r.y1 );
				r.x2 = max( x, r.x2 );
				r.y2 = max( y, r.y2 );
			}
		}
	}
}

