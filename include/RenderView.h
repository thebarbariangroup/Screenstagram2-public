//
//  RenderView.h
//  FlickrTestMultithreaded
//
//  Created by Doug Pfeffer on 4/16/12.
//  Copyright (c) 2012 The Barbarian Group. All rights reserved.
//
// Superclass for the various renderers. These renderers draw the screen saver at any given time. The ability to swap them in and out lets us more easily include multiple effects in the screen saver.
// Worth noting that these guys only get their images when they're initialized, updates don't really happen. If we need to cycle in new images we'll just re-init a new object.
//

#pragma once

#include "InstagramImageManager.h"
#include <vector>

class RenderView {
public:
    RenderView(int desiredNumRows, int desiredNumCols, ci::Area viewportArea);
	virtual ~RenderView() {}
    
    bool isPhasedIn();
    bool hasStartedPhasingOut();
    void startPhasingOut();
    void resize(ci::Area viewportArea);
    
    ci::Vec2f determineCoordsForColAndRow(int col, int row, float imageLength);
    
    float mImageLength;
    int mDesiredNumberRows;
    int mDesiredNumberCols;
    ci::Area mViewportArea;    
    
    static const int PADDING = 5;
    
    
    // These virtual methods are for the child render view classes to define.
    virtual bool canStart() = 0;    
    virtual void start() = 0;
    virtual void stop() = 0;    
    virtual void update() = 0;   
    virtual void draw() = 0;     
    virtual bool hasFinished() = 0;        
    virtual int zIndex() = 0;            
    virtual bool isReadyForNext() = 0;
    virtual std::string viewType() = 0;
    
    // TODO?
    // SAME THING APPLIES TO IVARS, if i want the child to have one?
    bool mIsPhasedIn;
    
protected:

    int mAge;
    // When the main app loop determines that the current render view is fully phased in it'll start bring in a new view, and set this flag to true.
    // This lets that loop know that it's started that process and it shouldn't double up transitions.
    bool mHasStartedPhasingOut; 
    
    
};