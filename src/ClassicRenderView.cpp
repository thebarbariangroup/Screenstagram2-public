//
//  ClassicRenderView.cpp
//  FlickrTestMultithreaded
//
//  Created by Doug Pfeffer on 4/16/12.
//  Copyright (c) 2012 The Barbarian Group. All rights reserved.
//

#include "ClassicRenderView.h"
#include "InstagramImageManager.h"
#include "InstagramImageFace.h"
#include "cinder/Rand.h"
#include <list>

using namespace ci;
using namespace ci::app;
using namespace std;


// TODO - load up swappable images for all the images we've got.


ClassicRenderView::ClassicRenderView(std::vector<shared_ptr<InstagramImageFace> > images, int desiredNumRows, int desiredNumCols, ci::Area viewportArea) : RenderView(desiredNumRows, desiredNumCols, viewportArea) {
//    mImageLength = (float)gl::getViewport().getWidth() / (float)MINIMUM_COLS;
    // Once we get all the images.

    // Convert the list to a vector.
    for (std::vector<shared_ptr<InstagramImageFace> >::iterator it = images.begin(); it != images.end(); it++){        
        mImages.push_back(*it);
    }

    elapsedTimeSinceStart = 0;
    mDesiredNumberRows = desiredNumRows;
    mDesiredNumberCols = desiredNumCols;
    mViewportArea = viewportArea;
    mImageLength = (float)viewportArea.getWidth()/(float)desiredNumCols - PADDING;
    determineImageLocations();

    mAge = 0;
    
    mIsPhasedIn = true;
    
}

void ClassicRenderView::start(){
    elapsedTimeSinceStart = app::getElapsedSeconds();
}

void ClassicRenderView::stop(){
    
}

bool ClassicRenderView::canStart() {
    return elapsedTimeSinceStart == 0;
}

// We'll randomly decide when we're done after a minimum period of time. Don't
// need to be swapping out too fast.
bool ClassicRenderView::hasFinished() {
    // TODO this timer might need to be linkd somehow to the transition of some other view.
    // What's happening is that the tile take over is taking so long that by the time it's back this is returning true again, causing a new tile view to pop right in.
    if(app::getElapsedSeconds() - elapsedTimeSinceStart > Rand::randInt(20, 25)){
        return true;
    } else {
        return false;
    }
}

void ClassicRenderView::draw() {

    // Draw the swapping images first, to ensure that they'll be behind the static images.
    // This way you don't ever see the business as they slide around.
    for(int i = 0; i < mSwappableImages.size(); i++){
        if(mSwappableImages[i].isSwapping()){
            mSwappableImages[i].draw(mImageLength);				
        }
    }
    for(int i = 0; i < mSwappableImages.size(); i++){
        if(!mSwappableImages[i].isSwapping()){
            mSwappableImages[i].draw(mImageLength);				
        }
    }    
}

void ClassicRenderView::update() {
    mAge++;
    
    Rand::randomize();
    int i = Rand::randInt( 0, 10 );
    if(i == 0){
        swapImage();
    }    
    
    for(int j = 0; j < mSwappableImages.size(); j++){
        mSwappableImages[j].update();
    }    
}


// Given a SwappableImage returns the minimum distance between it and any swappable images that are currently swapping.
// Returns -1 if there are no applicable comparisons to be made. E.g, maybe none are swapping at the moment.
float ClassicRenderView::minDistanceBetweenSwappableImages(SwappableImage swappableImage){
	Vec2f p1 = swappableImage.mOrigin;
	
	float min = -1;
	
	for(int i = 0; i < mSwappableImages.size(); i++){
        
		if(!mSwappableImages[i].isSwapping()){
			continue;
		}
		
		Vec2f p2 = mSwappableImages[i].mOrigin;
		
		if(p2.x == p1.x && p2.y == p1.y){
			continue;
		}	
		
		float distance = (float)sqrt(pow((p2.x - p1.x), 2) + pow((p2.y - p1.y), 2));	
		
		if(min == -1){
			min = distance;
		}
		
		if(min > distance){
			min = distance;	
		}
		
	}
    
	return min;
}


bool ClassicRenderView::isReadyForNext() {
    return hasFinished();
}

string ClassicRenderView::viewType(){
    return "ClassicRenderView";
}

// This is meant to be shown at the lowest level.
int ClassicRenderView::zIndex(){
    return 0;
}

void ClassicRenderView::swapImage(){
    
	if(mImages.size() <= 2){
        console() << "Bailing on ClassicRenderView::swapImage" << endl;
		return;
	}

	// To initiate an image swap we both have to find a SwappableImage that's not currently swapping
	// and the new image that'll be swapped in. That new image shouldn't be one that's in a SwappableImage.
	// Furthermore, we don't want to trigger a swap on SwappableImages that are near each other, because there's
	// a risk of their animation overlapping and it looking crappy.
	
	// We'll build lists of elements that match our conditions, then we'll randomly pull from each.
	vector<int> viableNewImageIndexes;
	vector<int> viableExistingImageIndexes;
    
	// Only choose an image for swapping out that's not currently swapping.
	// Build a list of those legit options, then we'll shuffle it.
	for(int i = 0; i < mSwappableImages.size(); i++){
		if(!mSwappableImages[i].isSwapping()){
			// See that this potential swappable image's origin isn't near any of the other swapping images.
			float distance = minDistanceBetweenSwappableImages(mSwappableImages[i]);
			if(distance != -1 && distance < (mImageLength * 3)){
				continue;
			}
			viableExistingImageIndexes.push_back(i);
		}
	}	
//    console() << "swapImage2" << endl;
	
	if(viableExistingImageIndexes.size() == 0){
		return;
	}
	
//    console() << "swapImage3" << endl;    
    srand ( unsigned ( time (NULL) ) );
	random_shuffle ( viableExistingImageIndexes.begin(), viableExistingImageIndexes.end() );			
	
	// Only choose an image for swapping in that's not currently visible.
	// We'd prefer a friend's photo.
	// Build a list of those legit options, then we'll shuffle it.
    
	for(int i = 0; i < mImages.size(); i++){
		if(!mImages.at(i)->mWasPlaced){	
			viableNewImageIndexes.push_back(i);
		}
	}

//    console() << "swapImage4" << endl;
	if(viableNewImageIndexes.size() == 0){
		return;
	}	
//    console() << "swapImage5" << endl;	
    srand ( unsigned ( time (NULL) ) );
	random_shuffle ( viableNewImageIndexes.begin(), viableNewImageIndexes.end() );			
	
	// Initiate the swap with our expertly chosen elements.
	mSwappableImages[viableExistingImageIndexes.front()].swap(mImages[viableNewImageIndexes.front()]);			
//    console() << "swapImage6" << endl;
}


// Called during setup. Go through full list of images and place them accordingly.
void ClassicRenderView::determineImageLocations(){
	
	// Track the current row and col we're on.
	int currentRow = 0;
	int currentCol = 0;
	
    for (vector<shared_ptr<InstagramImageFace> >::iterator it = mImages.begin(); it != mImages.end(); it++){
        
        Vec2f newOrigin = determineCoordsForColAndRow(currentCol, currentRow, mImageLength);
        
		// If we're gonna go off the bottom of the screen quit, we don't need to place anymore.
		if(newOrigin.y >= mViewportArea.getHeight()){
			break;
		}		
		
        (*it)->mOrigin = newOrigin;
		(*it)->setWasPlaced(true);
        
		SwappableImage swappableImage = SwappableImage(*it, newOrigin, mImageLength, currentCol, currentRow);
		mSwappableImages.push_back(swappableImage);
		
		currentCol++;
		
		// We need to check if the next image we'll place will fall off the side of the screen.
		Vec2f nextOrigin = determineCoordsForColAndRow(currentCol, currentRow, mImageLength);
		
		// Gonna go off the right edge? Bump it to the next line.
		if(nextOrigin.x >= mViewportArea.getWidth()){
			currentRow++;
			currentCol = 0;
			// Update this next point to take into account the new position we've learned it'll be on.
			nextOrigin = determineCoordsForColAndRow(currentCol, currentRow, mImageLength);
		}
	}
}


