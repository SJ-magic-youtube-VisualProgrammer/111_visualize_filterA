/************************************************************
************************************************************/
#include "DrawFFT.h"

/************************************************************
************************************************************/

/******************************
******************************/
DRAW_FFT::DRAW_FFT()
{
	font_L.load("font/RictyDiminished-Regular.ttf", 35, true, true, false, 0.3f, 72);
	font_S.load("font/RictyDiminished-Regular.ttf", 15, true, true, false, 0.3f, 72);
}

/******************************
******************************/
DRAW_FFT::~DRAW_FFT()
{
}

/******************************
******************************/
void DRAW_FFT::setup(const FFT& fft)
{
	/********************
	********************/
	print_separatoin();
	printf("> setup : Draw FFT\n");
	fflush(stdout);
	
	/********************
	********************/
	VboSet_L.setup(AUDIO_BUF_SIZE/2 * 4);
	VboSet_R.setup(AUDIO_BUF_SIZE/2 * 4);
	VboSet_Afilter.setup(AUDIO_BUF_SIZE/2);
	
	Refresh_vboVerts(fft);
	Refresh_vboColor(fft);
}

/******************************
1--2
|  |
0--3
******************************/
void DRAW_FFT::Refresh_vboVerts(const FFT& fft)
{
	/********************
	********************/
	ofVec2f ofs(100, 1050);
	const float width_of_bar = 2;
	const float space_per_bar = 3;
	const float BarHeight = 200;
	
	for (int i = 0; i < fft.get_sizeof_GainArray(); i++) {
		float _y = ofs.y;
		
		if(int(Gui_Global->DrawFFT_ContentsCombination_id) == CONTENTS_TYPE__N1_A1)			_y = ofs.y - fft.get_GainSmoothed_N(i) * BarHeight;
		else if(int(Gui_Global->DrawFFT_ContentsCombination_id) == CONTENTS_TYPE__N2_A2)	_y = ofs.y - fft.get_GainSmoothed2_N(i) * BarHeight;
		else if(int(Gui_Global->DrawFFT_ContentsCombination_id) == CONTENTS_TYPE__N1_N2)	_y = ofs.y - fft.get_GainSmoothed_N(i) * BarHeight;
		else																				_y = ofs.y - fft.get_GainSmoothed_A(i) * BarHeight; // CONTENTS_TYPE__A1_A2
		
		VboSet_L.set_vboVerts( i * 4 + 0, ofs.x + i * space_per_bar,				ofs.y );
		VboSet_L.set_vboVerts( i * 4 + 1, ofs.x + i * space_per_bar,				_y );
		VboSet_L.set_vboVerts( i * 4 + 2, ofs.x + i * space_per_bar + width_of_bar,	_y);
		VboSet_L.set_vboVerts( i * 4 + 3, ofs.x + i * space_per_bar + width_of_bar,	ofs.y );
	}
	
	VboSet_L.update_vertex();
	
	/********************
	********************/
	ofs = ofVec2f(1052, 1050);
	
	for (int i = 0; i < fft.get_sizeof_GainArray(); i++) {
		float _y = ofs.y;
		
		if(int(Gui_Global->DrawFFT_ContentsCombination_id) == CONTENTS_TYPE__N1_A1)			_y = ofs.y - fft.get_GainSmoothed_A(i) * BarHeight;
		else if(int(Gui_Global->DrawFFT_ContentsCombination_id) == CONTENTS_TYPE__N2_A2)	_y = ofs.y - fft.get_GainSmoothed2_A(i) * BarHeight;
		else if(int(Gui_Global->DrawFFT_ContentsCombination_id) == CONTENTS_TYPE__N1_N2)	_y = ofs.y - fft.get_GainSmoothed2_N(i) * BarHeight;
		else																				_y = ofs.y - fft.get_GainSmoothed2_A(i) * BarHeight; // CONTENTS_TYPE__A1_A2
		
		VboSet_R.set_vboVerts( i * 4 + 0, ofs.x + i * space_per_bar,				ofs.y );
		VboSet_R.set_vboVerts( i * 4 + 1, ofs.x + i * space_per_bar,				_y);
		VboSet_R.set_vboVerts( i * 4 + 2, ofs.x + i * space_per_bar + width_of_bar,	_y);
		VboSet_R.set_vboVerts( i * 4 + 3, ofs.x + i * space_per_bar + width_of_bar,	ofs.y );
	}
	
	VboSet_R.update_vertex();
	
	/********************
	********************/
	for (int i = 0; i < fft.get_sizeof_GainArray(); i++) {
		VboSet_Afilter.set_vboVerts( i, ofs.x + i * space_per_bar, ofs.y - fft.get_val_of_Afilter(i) * BarHeight / fft.get_max_of_Afilter() );
	}
	
	VboSet_Afilter.update_vertex();
}

/******************************
******************************/
void DRAW_FFT::Refresh_vboColor(const FFT& fft)
{
	/********************
	********************/
	ofColor col;
	
	for (int i = 0; i < fft.get_sizeof_GainArray(); i++) {
		ofColor col(255, 255, 255, 150);
		
		if(int(Gui_Global->DrawFFT_ContentsCombination_id) == CONTENTS_TYPE__N1_A1){
			if( (Gui_Global->ArtSin_Band_min__N <= i) && (i <= Gui_Global->ArtSin_Band_max__N) )	col = ofColor( 0, 255, 0, 150 );
		}else if(int(Gui_Global->DrawFFT_ContentsCombination_id) == CONTENTS_TYPE__N2_A2){
			if( (Gui_Global->ArtSin_Band_min__N <= i) && (i <= Gui_Global->ArtSin_Band_max__N) )	col = ofColor( 0, 255, 0, 150 );
		}else if(int(Gui_Global->DrawFFT_ContentsCombination_id) == CONTENTS_TYPE__N1_N2){
			if( (Gui_Global->ArtSin_Band_min__N <= i) && (i <= Gui_Global->ArtSin_Band_max__N) )	col = ofColor( 0, 255, 0, 150 );
		}else{ // CONTENTS_TYPE__A1_A2
			if( (Gui_Global->ArtSin_Band_min__A <= i) && (i <= Gui_Global->ArtSin_Band_max__A) )	col = ofColor( 0, 255, 0, 150 );
		}
		
		VboSet_L.setColor_perShape(4, i, col); // setColor_perShape(int NumPerShape, int id, const ofColor& color);
	}
	
	VboSet_L.update_color();
	
	/********************
	********************/
	for (int i = 0; i < fft.get_sizeof_GainArray(); i++) {
		ofColor col(255, 255, 255, 150);
		
		if(int(Gui_Global->DrawFFT_ContentsCombination_id) == CONTENTS_TYPE__N1_A1){
			if( (Gui_Global->ArtSin_Band_min__A <= i) && (i <= Gui_Global->ArtSin_Band_max__A) )	col = ofColor( 0, 255, 0, 150 );
		}else if(int(Gui_Global->DrawFFT_ContentsCombination_id) == CONTENTS_TYPE__N2_A2){
			if( (Gui_Global->ArtSin_Band_min__A <= i) && (i <= Gui_Global->ArtSin_Band_max__A) )	col = ofColor( 0, 255, 0, 150 );
		}else if(int(Gui_Global->DrawFFT_ContentsCombination_id) == CONTENTS_TYPE__N1_N2){
			if( (Gui_Global->ArtSin_Band_min__N <= i) && (i <= Gui_Global->ArtSin_Band_max__N) )	col = ofColor( 0, 255, 0, 150 );
		}else{ // CONTENTS_TYPE__A1_A2
			if( (Gui_Global->ArtSin_Band_min__A <= i) && (i <= Gui_Global->ArtSin_Band_max__A) )	col = ofColor( 0, 255, 0, 150 );
		}
		
		VboSet_R.setColor_perShape(4, i, col); // setColor_perShape(int NumPerShape, int id, const ofColor& color);
	}
	
	VboSet_R.update_color();
	
	/********************
	********************/
	VboSet_Afilter.set_singleColor(ofColor(255, 255, 255, 150));
	VboSet_Afilter.update_color();
}

/******************************
******************************/
void DRAW_FFT::update(const FFT& fft)
{
	Refresh_vboVerts(fft);
	Refresh_vboColor(fft);
}

/******************************
******************************/
void DRAW_FFT::draw(ofFbo& fbo, const FFT& fft)
{
	fbo.begin();
		/********************
		********************/
		ofEnableAntiAliasing();
		ofEnableBlendMode(OF_BLENDMODE_ADD); // OF_BLENDMODE_DISABLED, OF_BLENDMODE_ALPHA, OF_BLENDMODE_ADD
		
		/********************
		title of L
		********************/
		char buf[BUF_SIZE_S];
		
		if(int(Gui_Global->DrawFFT_ContentsCombination_id) == CONTENTS_TYPE__N1_A1)			sprintf(buf, "N1");
		else if(int(Gui_Global->DrawFFT_ContentsCombination_id) == CONTENTS_TYPE__N2_A2)	sprintf(buf, "N2");
		else if(int(Gui_Global->DrawFFT_ContentsCombination_id) == CONTENTS_TYPE__N1_N2)	sprintf(buf, "N1");
		else																				sprintf(buf, "A1"); // CONTENTS_TYPE__A1_A2
		
		font_L.drawString(buf, 48, 877);
		
		/********************
		title of R
		********************/
		if(int(Gui_Global->DrawFFT_ContentsCombination_id) == CONTENTS_TYPE__N1_A1)			sprintf(buf, "A1");
		else if(int(Gui_Global->DrawFFT_ContentsCombination_id) == CONTENTS_TYPE__N2_A2)	sprintf(buf, "A2");
		else if(int(Gui_Global->DrawFFT_ContentsCombination_id) == CONTENTS_TYPE__N1_N2)	sprintf(buf, "N2");
		else																				sprintf(buf, "A2"); // CONTENTS_TYPE__A1_A2
		
		font_L.drawString(buf, 1002, 877);
		
		/********************
		Afilter 目盛り
		********************/
		float step = fft.get_max_of_Afilter() / 5;
		glm::vec2 ofs(1835, 1054);
		
		float val = 0.0;
		
		for(int i = 0; i < 6; i++){
			sprintf(buf, "%5.2f", val);
			font_S.drawString(buf, ofs.x, ofs.y);
			
			const float window_H = 200.0;
			val += step;
			ofs.y -= window_H / 5;
		}
		
		/********************
		Afilter 0dBの位置
		********************/
		{
			const float BarHeight = 200;
			const float space_per_bar = 3;
			ofVec2f ofs = ofVec2f(1052, 1050);
			
			double Base_Freq = double(AUDIO_SAMPLERATE) / AUDIO_BUF_SIZE;
			float xX = Gui_Global->FFT__Afilter_0dB_at_Hz / Base_Freq;
			
			ofSetColor(0, 255, 255, 140);
			ofDrawCircle(ofs.x + space_per_bar * xX, ofs.y - BarHeight / fft.get_max_of_Afilter() * 1.0, 3);
		}
		
		/********************
		********************/
		ofFill();
		
		ofSetColor(255);
		
		glPointSize(1);
		glLineWidth(1);
		
		VboSet_L.draw(GL_QUADS);
		VboSet_R.draw(GL_QUADS);
		VboSet_Afilter.draw(GL_POINTS);
		// VboSet_Afilter.draw(GL_LINE_STRIP);
	fbo.end();
}
