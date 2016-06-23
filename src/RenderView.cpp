//
//  RenderView.cpp
//  FlickrTestMultithreaded
//
//  Created by Doug Pfeffer on 4/16/12.
//  Copyright (c) 2012 The Barbarian Group. All rights reserved.
//


#include "RenderView.h"

using namespace ci;
using namespace std;


RenderView::RenderView(int desiredNumRows, int desiredNumCols, ci::Area viewportArea){
    
    mDesiredNumberRows = desiredNumRows;
    mDesiredNumberCols = desiredNumCols;
    mViewportArea = viewportArea;
    mImageLength = (float)viewportArea.getWidth()/(float)desiredNumCols - PADDING;    
    
    mAge = 0;

    mHasStartedPhasingOut = false;

}


void RenderView::resize(ci::Area viewportArea){
    mViewportArea = viewportArea;
}

void RenderView::update(){
    mAge++;
}

// When a view is fully phased in this should return true.
// This is useed by the main app loop to tell when it can swap in a new view underneath the active one. Don't want to do that until
// the new guy is all the way in, otherwise it'll look shitty.
bool RenderView::isPhasedIn(){
    return mIsPhasedIn;
}


void RenderView::startPhasingOut(){
    mHasStartedPhasingOut = true;
}

bool RenderView::hasStartedPhasingOut(){
    return mHasStartedPhasingOut;
}

// Should this just go into some utils kind of place? Does the RenderView superclass need this list of images?

// Given a row and column position returns a Vec2f for where that image should be placed.
Vec2f RenderView::determineCoordsForColAndRow(int col, int row, float imageLength){
	int x = col * imageLength + (col * PADDING) + (float)PADDING/2.0f;
	int y = row * imageLength + (row * PADDING) + (float)PADDING/2.0f;
	return Vec2f(x, y);
}