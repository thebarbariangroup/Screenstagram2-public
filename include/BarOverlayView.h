//
//  BarOverlayView.h
//  FlickrTestMultithreaded
//
//  Created by Doug Pfeffer on 4/30/12.
//  Copyright (c) 2012 The Barbarian Group. All rights reserved.
//

#pragma once
#include "RenderView.h"
#include <vector>

class BarOverlayView : public RenderView {
public:
    BarOverlayView(int totalImages, int desiredNumRows, int desiredNumCols, ci::Area viewportArea);
    
    int mTotalImages;
    
    void draw();
    void stop();
    bool hasFinished();
    void start();
    bool canStart();
    bool isReadyForNext();
    void update();
    int zIndex();    
    std::string viewType();
};

