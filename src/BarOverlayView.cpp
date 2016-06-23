//
//  BarOverlayView.cpp
//  FlickrTestMultithreaded
//
//  Created by Doug Pfeffer on 4/30/12.
//  Copyright (c) 2012 The Barbarian Group. All rights reserved.
//

#include "BarOverlayView.h"

#include "cinder/app/App.h"

using namespace ci;
using namespace ci::app;
using namespace std;


BarOverlayView::BarOverlayView(int totalImages, int desiredNumRows, int desiredNumCols, ci::Area viewportArea) : RenderView(desiredNumRows, desiredNumCols, viewportArea) {
    mTotalImages = totalImages;
}

void BarOverlayView::draw() {
    
    // Draw the background color bars between each image.
    
	gl::color(Color(0, 0, 0));    
    int borderPadding = PADDING;
	for(int r = 0; r < mDesiredNumberRows + 2; r++){		
		Vec2f p = determineCoordsForColAndRow(0, r, mImageLength);
		gl::drawSolidRect(Rectf(Area(p.x - (borderPadding), p.y - (borderPadding), mViewportArea.getWidth(), p.y)));
	}
    
    
	for(int c = 0; c < mDesiredNumberCols + 2; c++){		
		Vec2f p = determineCoordsForColAndRow(c, 0, mImageLength);
		gl::drawSolidRect(Rectf(Area(p.x - (borderPadding), p.y - (borderPadding), p.x, mViewportArea.getHeight())));
	}        
	gl::color(Color(1, 1, 1));    
}


// Stubbed out the following methods to jive with some RenderView reqs. Don't need them here.
string BarOverlayView::viewType(){ return "BarOverlayView"; }
void BarOverlayView::stop() {}
bool BarOverlayView::hasFinished() { return true; }
void BarOverlayView::start() {}
bool BarOverlayView::canStart() { return true; }
bool BarOverlayView::isReadyForNext() { return true; }
void BarOverlayView::update() {}
int BarOverlayView::zIndex() {return 0; }

