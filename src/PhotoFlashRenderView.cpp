//
//  PhotoFlashRenderView.cpp
//  FlickrTestMultithreaded
//
//  Created by Doug Pfeffer on 4/23/12.
//  Copyright (c) 2012 The Barbarian Group. All rights reserved.
//


#include "PhotoFlashRenderView.h"
#include "cinder/Rand.h"
#include "cinder/app/App.h"

using namespace ci;
using namespace ci::app;
using namespace std;


PhotoFlashRenderView::PhotoFlashRenderView(int desiredNumRows, int desiredNumCols, ci::Area viewportArea) : RenderView(desiredNumRows, desiredNumCols, viewportArea) {
    mImageLength = (float)viewportArea.getWidth()/(float)desiredNumCols - PADDING;
    
}

void PhotoFlashRenderView::addFlash(){
	// Start on the first row, a random column.
	Rand::randomize();	
	int col = Rand::randInt(0, mDesiredNumberCols);	
	Rand::randomize();	
	int row = Rand::randInt(0, mDesiredNumberRows);	
 
	Vec2f origin = determineCoordsForColAndRow(col, row, mImageLength);
	
	
	// Bail if there's already a FlashView with this coord.
	// While we're looping, remove any inactive fellas.
	for(int i = 0; i < mFlashViews.size(); i++){
		
		if(!mFlashViews[i]->mIsActive){
			mFlashViews.erase(mFlashViews.begin() + i);
		}		
		
		else if(mFlashViews[i]->mX == origin.x && mFlashViews[i]->mY == origin.y){
			return;
		}
	}
	
	shared_ptr<FlashView> c(new FlashView(origin.x, origin.y, mImageLength, mImageLength));
	mFlashViews.push_back(c);    
}

bool PhotoFlashRenderView::isReadyForNext() {
    return true;
}

int PhotoFlashRenderView::zIndex(){
    return 100; // This gets drawn way up on top of everything else.
}

void PhotoFlashRenderView::update(){
    Rand::randomize();
	if(Rand::randInt(0, 3) == 0){
		addFlash();	
	}
}

string PhotoFlashRenderView::viewType(){
    return "PhotoFlashRenderView";
}

void PhotoFlashRenderView::draw(){
    
	for(int i = 0; i < mFlashViews.size(); i++){
		mFlashViews[i]->draw();
	}

}


// Stubbed out just to adhere to the RenderView interface.
bool PhotoFlashRenderView::canStart() { return true; }
// Stubbed out just to adhere to the RenderView interface.
void PhotoFlashRenderView::start() {}
// Stubbed out just to adhere to the RenderView interface.
void PhotoFlashRenderView::stop() {}
// Stubbed out just to adhere to the RenderView interface.
bool PhotoFlashRenderView::hasFinished() { return true; }