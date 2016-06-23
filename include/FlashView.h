/*
 *  FlashView.h
 *  instagram_ss
 *
 *  Created by dp on 3/1/11.
 *  Copyright 2011 The Barbarian Group. All rights reserved.
 *
 */

/* These panels start at the top row and sort of trickle down to the bottom of the screen and away. */

#pragma once

#include "cinder/Timeline.h"
#include "cinder/Color.h"

class FlashView {
public:
	FlashView(int x, int y, int width, int height);

	void draw();
	void makeActive();
	void makeInactive();
	
    ci::ColorA getRandomColor();
	
    ci::Anim<float> mAlpha;
	ci::ColorA mColor;
	bool mIsActive;
	int mX;
	int mY;
	int mWidth;
	int mHeight;
    
};

