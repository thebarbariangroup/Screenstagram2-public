/*
 *  FlashView.cpp
 *  instagram_ss
 *
 *  Created by dp on 3/1/11.
 *  Copyright 2011 The Barbarian Group. All rights reserved.
 *
 */

#include "FlashView.h"
#include "cinder/Rand.h"
#include "cinder/Timeline.h"
#include "cinder/app/App.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static const float FADE_DURATION = 0.65f;
static const float MAX_ALPHA = 0.25f;



struct FadeFinished { // Used to indicate when a FlashView has finished animating.
    FadeFinished( FlashView * flashView ) : mFlashView( flashView ) {}
    void operator()(){ 
        mFlashView->makeInactive();
    }
    FlashView * mFlashView;
};



FlashView::FlashView(int x, int y, int width, int height){
	mX = x;
	mY = y;
	mWidth = width;
	mHeight = height;	
	mColor = getRandomColor();
    mAlpha = 0.0f;
	makeActive();    
}


ColorA FlashView::getRandomColor(){
	
	std::vector<ColorA> colors;

	float alpha = 1.0f;
	ColorA red = ColorA(255.0f/255.0f, 60.0f/255.0f, 0.0f/255.0f, alpha);
	ColorA raw_red = ColorA(137.0f/255.0f, 0.0f/255.0f, 0.0f/255.0f, alpha);
	ColorA blue = ColorA(0.0f/255.0f, 132.0f/255.0f, 255.0f/255.0f, alpha);
	ColorA blue_raw = ColorA(0.0f/255.0f, 97.0f/255.0f, 137.0f/255.0f, alpha);
	ColorA orange = ColorA(255.0f/255.0f, 168.0f/255.0f, 0.0f/255.0f, alpha); 
	ColorA pink = ColorA(255.0f/255.0f, 68.0f/255.0f, 187.0f/255.0f, alpha);

	//	colors.push_back(grey);
	colors.push_back(red);
	colors.push_back(raw_red);	
	colors.push_back(blue_raw);
	colors.push_back(orange);
	colors.push_back(pink);

	srand ( time(NULL) );
	Rand::randomize();
	int random_color_index = Rand::randInt(0, colors.size());
	//	return Color(0, 0, 0);
	return colors[random_color_index];
}

void FlashView::makeInactive(){
	mIsActive = false;
}

void FlashView::makeActive(){
	mIsActive = true;
    timeline().apply(&mAlpha, 0.0f, MAX_ALPHA, FADE_DURATION, EaseInQuad()).delay(0.05f);
    timeline().appendTo(&mAlpha, MAX_ALPHA, 0.0f, FADE_DURATION, EaseInQuad()).finishFn(FadeFinished(this));
}

void FlashView::draw(){
    
    
    
//	if(!mIsActive){ return; }


	Area a = Area(mX, mY, mX + mWidth, mY + mHeight);
	mColor.a = mAlpha;
	gl::color(mColor);
	gl::drawSolidRect(Rectf(a));
	gl::color(ColorA(1, 1, 1, 1));			  
	
}