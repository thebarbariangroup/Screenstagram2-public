//
//  LoaderView.h
//  FlickrTestMultithreaded
//
//  Created by Doug Pfeffer on 4/5/12.
//  Copyright (c) 2012 The Barbarian Group. All rights reserved.
//

#pragma once
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/Font.h"
#include "cinder/Timeline.h"

#include "FontMeta.h"

class LoaderView {
public:
    LoaderView(std::string message, double bundleVersion, const FontMeta baseFontMeta);
    
    void draw(cinder::Area a);
    void start();
    void stop(ci::Timeline * t);
    bool isDone();
    void getCurrentVersion();
    void setStatusMessage(std::string message);
    void showNewVersionMessage();
    void loadingFailed();
    
    ci::gl::Texture mSpinner;
    ci::Font mFont;
    
    bool mIsDone;
    bool mShowNewVersionMessage;
    double mBundleVersion;
    ci::Anim<float> mBackgroundAlpha;
    ci::Anim<float> mForegroundAlpha;    
    ci::Anim<float> mRotation;
    std::string mStatusMessage;
    bool mLoadingFailed;
};