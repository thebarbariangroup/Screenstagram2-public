//
//  PhotoFlashRenderView.h
//  FlickrTestMultithreaded
//
//  Created by Doug Pfeffer on 4/23/12.
//  Copyright (c) 2012 The Barbarian Group. All rights reserved.
//
/*
 
 This class manages the various flash views - those pulses of light that randomly happen over on screen images.
 
 
*/

#pragma once

#include "FlashView.h"
#include "RenderView.h"
#include "cinder/Timeline.h"
#include <vector>

class PhotoFlashRenderView : public RenderView {
public:
    
    PhotoFlashRenderView(int desiredNumRows, int desiredNumCols, ci::Area viewportArea);
    void update();
    void draw();
    void addFlash();
    
    bool canStart();
    void start();
    void stop();
    bool hasFinished();
    int zIndex();
    bool isReadyForNext();
    std::string viewType();
    
    std::vector<std::shared_ptr<FlashView> > mFlashViews;
    


};
