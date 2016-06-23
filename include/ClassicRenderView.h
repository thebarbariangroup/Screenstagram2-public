//
//  ClassicRenderView.h
//  FlickrTestMultithreaded
//
//  Created by Doug Pfeffer on 4/16/12.
//  Copyright (c) 2012 The Barbarian Group. All rights reserved.
//
//  Implements the "classic" view from Screenstagram 1.
//
//

#pragma once
#include "RenderView.h"
#include "SwappableImage.h"
#include <vector>

class ClassicRenderView : public RenderView {
public:
    ClassicRenderView(std::vector<shared_ptr<InstagramImageFace> > images, int desiredNumRows, int desiredNumCols, ci::Area viewportArea);

    void draw();
    void update();
    void start();
    void stop();
    bool canStart();
    bool hasFinished();
    void swapImage();
    bool isReadyForNext();
    void determineImageLocations(); // Arranges images in SwappableImages and assigns them appropriate screen coords.
    float minDistanceBetweenSwappableImages(SwappableImage swappableImage);
    int zIndex();
    
    std::vector<shared_ptr<InstagramImageFace> > mImages;
    
    std::string viewType();
    
    double elapsedTimeSinceStart;
    float mImageLength;
    int mDesiredNumberRows;
    int mDesiredNumberCols;
    ci::Area mViewportArea;
    
    std::vector<SwappableImage> mSwappableImages;
    
protected:

    int mAge;
    
    
};
