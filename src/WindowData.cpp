//
//  WindowData.cpp
//  Screenstagram2
//
//  Created by Doug Pfeffer on 10/10/12.
//  Copyright (c) 2012 The Barbarian Group. All rights reserved.
//


#include "WindowData.h"
#include "cinder/app/App.h"


using namespace ci;
using namespace ci::app;
using namespace std;

WindowData::WindowData(ci::Area viewportArea, int minCols, int minRows, int requiredImages, InstagramImageManager * instagramImageManager){
    mViewportArea = viewportArea;
    mMinRows = minRows;
    mMinCols = minCols;
    mRequiredImages = requiredImages;
    mPhotoFlashRenderView       = new PhotoFlashRenderView(mMinRows, minCols, viewportArea);    
    mBarOverlayView             = new BarOverlayView(mRequiredImages, mMinRows, minCols, viewportArea);  
    mInstagramImageManager = instagramImageManager;
    
}

void WindowData::update(){
    
    deque<shared_ptr<RenderView> >::const_iterator renderViewIterator;    
    for (renderViewIterator = mRenderViews.begin(); renderViewIterator != mRenderViews.end(); ++renderViewIterator) {
        renderViewIterator->get()->update();
        if(renderViewIterator->get()->canStart()){
            renderViewIterator->get()->start(); 
        }
    }

    // Only keep adding views when we're in classic render view.
    if(mRenderViews.size() > 0 && mRenderViews.front()->viewType() == "ClassicRenderView" && mRenderViews.back()->viewType() == "ClassicRenderView"){
        mPhotoFlashRenderView->update();
    }
    
    if(mRenderViews.size() > 0 && mRenderViews.size() < 2 && mRenderViews.front()->isReadyForNext()){
        addNewRenderView(); 
    }
    
    
    // Finally kill the first (oldest view) when the 2nd view is fully phased in.
    if(mRenderViews.size() > 1 && mRenderViews[1]->isPhasedIn() && mRenderViews.front()->hasFinished()){
        mRenderViews.pop_front();
    }            
}

void WindowData::resize(ci::Area viewportArea){
    
}

void WindowData::draw(){

    // Draw the first two, ordered by their Z index value. Higher numbers on top.
    if(mRenderViews.size() == 2){
        if(mRenderViews[0]->zIndex() > mRenderViews[1]->zIndex()){
            mRenderViews[1]->draw();
            mRenderViews[0]->draw();            
        } else {
            mRenderViews[0]->draw();
            mRenderViews[1]->draw();                        
        }
    } else if(mRenderViews.size() == 1){
        mRenderViews[0]->draw();                                
    }
    
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);	    
    mPhotoFlashRenderView->draw();    
    glDisable(GL_BLEND);
    gl::enableAlphaBlending();
    
    mBarOverlayView->draw();    
    
}

void WindowData::addNewTileTakeOverView(){
	shared_ptr<TileTakeoverRenderView> t(new TileTakeoverRenderView(mInstagramImageManager->getRandomImage(), mRequiredImages, mMinRows, mMinCols, mViewportArea));
	mRenderViews.push_back(t);
}

void WindowData::addNewClassicRenderView(){
    shared_ptr<RenderView> t(new ClassicRenderView(mInstagramImageManager->getImages(), mMinRows, mMinCols, mViewportArea));
    mRenderViews.push_back(t);    
}

// Adds a new render view to the dequeue.
// Might take the current view into account.
void WindowData::addNewRenderView(){
    // We have some rules to determine what the next view will be.
    if(mRenderViews.size() == 1){
        if(mRenderViews.front()->viewType() == "ClassicRenderView"){
            addNewTileTakeOverView();
        } else if(mRenderViews.front()->viewType() == "TileTakeoverRenderView"){
            addNewClassicRenderView();
        }
    } else if(mRenderViews.size() == 0){
        // For now if it's empty
        addNewClassicRenderView();        
    } else {
        // Shouldn't get here.
        // throw;
    }
    
}