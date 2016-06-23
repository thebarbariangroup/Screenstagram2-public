//
//  LoaderView.cpp
//  FlickrTestMultithreaded
//
//  Created by Doug Pfeffer on 4/5/12.
//  Copyright (c) 2012 The Barbarian Group. All rights reserved.
//

#include "LoaderView.h"
#include "Resources.h"
#include "cinder/Utilities.h"
#include "cinder/app/App.h"

#include "FontMeta.h"

#include <sstream>
#include <math.h>
using namespace ci;
using namespace ci::app;
using namespace std;


struct LoaderViewFadeOutFinished {
    LoaderViewFadeOutFinished( LoaderView * l ) : mLoaderView( l ) {}
    void operator()(){ 
        mLoaderView->mIsDone = true;
    }
    LoaderView * mLoaderView;
};


LoaderView::LoaderView(std::string message, double bundleVersion, const FontMeta baseFontMeta){
    mBackgroundAlpha = 1.0f;   
    mForegroundAlpha = 0.0f;
    mRotation = 0.0f;
    mBundleVersion = bundleVersion;
	mSpinner = gl::Texture(loadImage( loadResource( RES_LOADER_PNG ) ));
    mStatusMessage = message;
    mFont = Font( baseFontMeta.family, baseFontMeta.size + 3);
    mLoadingFailed = false;
    mStatusMessage = " ";
    mIsDone = false;
    mShowNewVersionMessage = false;
    getCurrentVersion();    
}

void LoaderView::getCurrentVersion(){
    std::stringstream sstm;
#if defined( CINDER_MAC )
	sstm << "http://screenstagram.s3.amazonaws.com/currentVersion.txt?i=" << time (NULL); // Get the URL with a cache buster.
#elif defined( CINDER_MSW )
	sstm << "http://screenstagram.s3.amazonaws.com/win-currentVersion.txt?i=" << time (NULL);
#endif
    string url = sstm.str();
    string version = loadString(loadUrl(url));
    if(mBundleVersion < ::strtod(version.c_str(), NULL)){
        mShowNewVersionMessage = true;    
    }
}

void LoaderView::loadingFailed(){
    mLoadingFailed = true;
}

void LoaderView::start(){
    if(mForegroundAlpha > 0.0f){
        return;
    }
    timeline().apply(&mForegroundAlpha, 0.0f, 1.0f, 1.0f);
    timeline().apply(&mRotation, 0.0f, 360.0f, 2.0f).loop();    
}

void LoaderView::stop(ci::Timeline * t){
    timeline().apply(&mForegroundAlpha, 1.0f, 0.0f, 1.0f);
    timeline().apply(&mBackgroundAlpha, 1.0f, 0.0f, 1.0f).delay(1).finishFn(LoaderViewFadeOutFinished(this));    
}

bool LoaderView::isDone(){
    return mIsDone;
}

void LoaderView::setStatusMessage(std::string message) {
    mStatusMessage = message;
}

void LoaderView::draw(Area a){

    glPushMatrix();
	
	glColor4f(31.0f/255.0f, 31.0f/255.0f, 31.0f/255.0f, mBackgroundAlpha);
	gl::drawSolidRect(a);
	
	glColor4f(1.0f, 1.0f, 1.0f, mForegroundAlpha);
	
	int x = a.getWidth()/2;
	int y = a.getHeight()/2 - 100;
	
	gl::translate(Vec2f(x, y));		
	if(!mLoadingFailed){	
		gl::rotate(mRotation);
	}
	gl::translate(Vec2f(-mSpinner.getWidth()/2, -mSpinner.getHeight()/2 ));
	
	gl::draw( mSpinner, Vec2f(0, 0) );
	
	glPopMatrix();	
    
    gl::drawStringCentered(mStatusMessage, Vec2f(a.getWidth()/2 + 1, a.getHeight()/2 - 50 + 1 ), ColorA(0.0f, 0.0f, 0.0f, mForegroundAlpha * 0.5f), mFont);
	gl::drawStringCentered(mStatusMessage, Vec2f(a.getWidth()/2, a.getHeight()/2 - 50 ), ColorA(210.0f/255.0f, 210.0f/255.0f, 210.0f/255.0f, mForegroundAlpha), mFont);
    
    if(mShowNewVersionMessage){
        ColorA grey = ColorA(108.0f/255.0f, 108.0f/255.0f, 108.0f/255.0f, mForegroundAlpha);
        gl::drawStringCentered("A new version of Screenstagram is available!", Vec2f(a.getWidth()/2 + 2, a.getHeight() - 255 ), grey, mFont);
        gl::drawStringCentered("Get it at http://barbariangroup.com/software/screenstagram", Vec2f(a.getWidth()/2 + 2, a.getHeight() - 236 ), grey, mFont);
    }    
    
    gl::color(Color(1, 1, 1));
    
}
