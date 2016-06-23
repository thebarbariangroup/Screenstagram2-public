//
//  WindowData.h
//  Screenstagram2
//
//  Created by Doug Pfeffer on 10/10/12.
//  Copyright (c) 2012 The Barbarian Group. All rights reserved.
//
// We need to have a set of views per screen, so each knows its own dimensions.
// This contains that info.
// AFB suggested i name it this, and who am i to argue?
//


#pragma once

#include "ClassicRenderView.h"
#include "TileTakeoverRenderView.h"
#include "BarOverlayView.h"
#include "PhotoFlashRenderView.h"
#include "InstagramImageManager.h"

class WindowData {
public:
    WindowData(ci::Area viewportArea, int minCols, int minRows, int requiredImages,     InstagramImageManager * instagramImageManager);
    void resize(ci::Area viewportArea);
    void update();
    void draw();
    void addNewClassicRenderView();
    void addNewTileTakeOverView();    
    void addNewRenderView();
    
    size_t mWindowIndex;
    ci::Area mViewportArea;
    int mMinRows;
    int mMinCols;
    int mRequiredImages;
    InstagramImageManager * mInstagramImageManager;
    
    BarOverlayView  * mBarOverlayView;  
    RenderView * mPhotoFlashRenderView;
  
    
    // We maintain a pair of render views in this list. The first is the current render view, the 2nd is the next render view.
    // When the first one is done we shift the next view into 1st place, and bring in a new view for the 2nd place.
    deque<shared_ptr<RenderView> >               mRenderViews;    
    
};