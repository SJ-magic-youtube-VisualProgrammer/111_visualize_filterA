/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include <ofMain.h>
#include "fft.h"
#include <ofxVboSet.h>
#include "sj_common.h"

/************************************************************
************************************************************/

/**************************************************
**************************************************/
class DRAW_FFT{
private:
	/****************************************
	****************************************/
	enum CONTENTS_TYPE{
		CONTENTS_TYPE__N1_A1,
		CONTENTS_TYPE__N2_A2,
		CONTENTS_TYPE__N1_N2,
		CONTENTS_TYPE__A1_A2,
	};
	
	/****************************************
	****************************************/
	ofx__VBO_SET VboSet_L;
	ofx__VBO_SET VboSet_R;
	ofx__VBO_SET VboSet_Afilter;
	
	ofTrueTypeFont font_L;
	ofTrueTypeFont font_S;
	
	/****************************************
	****************************************/
	void Refresh_vboVerts(const FFT& fft);
	void Refresh_vboColor(const FFT& fft);
	
public:
	DRAW_FFT();
	~DRAW_FFT();
	void setup(const FFT& fft);
	void update(const FFT& fft);
	void draw(ofFbo& fbo, const FFT& fft);
};

