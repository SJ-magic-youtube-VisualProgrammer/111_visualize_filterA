/************************************************************
************************************************************/
#include "DrawArtSin.h"

/************************************************************
************************************************************/

/******************************
******************************/
DRAW_ARTSIN::DRAW_ARTSIN()
{
	font_L.load("font/RictyDiminished-Regular.ttf", 30, true, true, false, 0.3f, 72);
	font_S.load("font/RictyDiminished-Regular.ttf", 20, true, true, false, 0.3f, 72);
}

/******************************
******************************/
DRAW_ARTSIN::~DRAW_ARTSIN()
{
}

/******************************
******************************/
void DRAW_ARTSIN::setup(const FFT& fft)
{
	/********************
	********************/
	print_separatoin();
	printf("> setup : Draw ArtSin 2D\n");
	fflush(stdout);
	
	// img_Frame.load("img_frame/Frame.png");
	
	/********************
	********************/
	VboSet_N.setup(AUDIO_BUF_SIZE * 4);
	VboSet_A.setup(AUDIO_BUF_SIZE * 4);
	VboSet_MixDown.setup(AUDIO_BUF_SIZE * 4);
	
	if(Gui_Global->b_DrawArtSin_Overlay)	Refresh_vboVerts(fft, true);
	else									Refresh_vboVerts(fft);
	
	Refresh_vboColor();
}

/******************************
1--2
|  |
0--3
******************************/
void DRAW_ARTSIN::Refresh_vboVerts(const FFT& fft, bool b_Overlay)
{
	/********************
	VboSet_A
	********************/
	const float width_of_bar = 1;
	const float space_per_bar = 3;
	float ScreenHeightOfOne = Gui_Global->DrawArtSin_ScreenHeightOfOne;
	glm::vec2 ofs = glm::vec2(192, 400);
	
	for(int i = 0; i < AUDIO_BUF_SIZE; i++){
		float _y = sj_max( float(ofs.y - fft.get_artSin_A(i) * ScreenHeightOfOne), float(ofs.y - 350) );
		
		VboSet_A.set_vboVerts( i * 4 + 0, ofs.x + i * space_per_bar,				ofs.y );
		VboSet_A.set_vboVerts( i * 4 + 1, ofs.x + i * space_per_bar,				_y );
		VboSet_A.set_vboVerts( i * 4 + 2, ofs.x + i * space_per_bar + width_of_bar,	_y);
		VboSet_A.set_vboVerts( i * 4 + 3, ofs.x + i * space_per_bar + width_of_bar,	ofs.y );
	}
	
	VboSet_A.update_vertex();
	
	/********************
	VboSet_N
	********************/
	if(!b_Overlay) ofs = glm::vec2(192, 800);
	
	for(int i = 0; i < AUDIO_BUF_SIZE; i++){
		float _y = sj_max( float(ofs.y - fft.get_artSin_N(i) * ScreenHeightOfOne), float(ofs.y - 350) );
		
		VboSet_N.set_vboVerts( i * 4 + 0, ofs.x + i * space_per_bar,				ofs.y );
		VboSet_N.set_vboVerts( i * 4 + 1, ofs.x + i * space_per_bar,				_y );
		VboSet_N.set_vboVerts( i * 4 + 2, ofs.x + i * space_per_bar + width_of_bar,	_y);
		VboSet_N.set_vboVerts( i * 4 + 3, ofs.x + i * space_per_bar + width_of_bar,	ofs.y );
	}
	
	VboSet_N.update_vertex();
	
	/********************
	VboSet_MixDown
	********************/
	if(b_Overlay) ofs = glm::vec2(192, 800);
	
	for(int i = 0; i < AUDIO_BUF_SIZE; i++){
		float _y;
		if(b_Overlay)	_y = sj_max( float(ofs.y - fft.get_artSin_MixDown(i) * ScreenHeightOfOne), float(ofs.y - 350) );
		else 			_y = ofs.y;
		
		VboSet_MixDown.set_vboVerts( i * 4 + 0, ofs.x + i * space_per_bar,					ofs.y );
		VboSet_MixDown.set_vboVerts( i * 4 + 1, ofs.x + i * space_per_bar,					_y );
		VboSet_MixDown.set_vboVerts( i * 4 + 2, ofs.x + i * space_per_bar + width_of_bar,	_y);
		VboSet_MixDown.set_vboVerts( i * 4 + 3, ofs.x + i * space_per_bar + width_of_bar,	ofs.y );
	}
	
	VboSet_MixDown.update_vertex();
}

/******************************
******************************/
void DRAW_ARTSIN::Refresh_vboColor()
{
	VboSet_N.set_singleColor(Gui_Global->DrawArtSin_col__N);
	VboSet_N.update_color();
	
	VboSet_A.set_singleColor(Gui_Global->DrawArtSin_col__A);
	VboSet_A.update_color();
	
	VboSet_MixDown.set_singleColor(Gui_Global->DrawArtSin_col__MixDown);
	VboSet_MixDown.update_color();
	
	return;
}

/******************************
******************************/
void DRAW_ARTSIN::update(const FFT& fft)
{
	if(Gui_Global->b_DrawArtSin_Overlay)	Refresh_vboVerts(fft, true);
	else									Refresh_vboVerts(fft);
	
	Refresh_vboColor();
}

/******************************
******************************/
void DRAW_ARTSIN::draw(ofFbo& fbo)
{
	fbo.begin();
		/********************
		********************/
		ofEnableAntiAliasing();
		ofEnableBlendMode(OF_BLENDMODE_ADD); // OF_BLENDMODE_DISABLED, OF_BLENDMODE_ALPHA, OF_BLENDMODE_ADD
		
		/********************
		Graph
		********************/
		ofFill();
		
		ofSetColor(255);
		// glPointSize(Gui_Global->particleSize);
		
		VboSet_N.draw(GL_QUADS);
		VboSet_A.draw(GL_QUADS);
		
		if(Gui_Global->b_DrawArtSin_Overlay)	VboSet_MixDown.draw(GL_QUADS);
		
		/********************
		mask
		********************/
		if(Gui_Global->b_DrawArtSin_MaskUpper){
			ofEnableBlendMode(OF_BLENDMODE_ALPHA); // OF_BLENDMODE_DISABLED, OF_BLENDMODE_ALPHA, OF_BLENDMODE_ADD
			ofSetColor(Gui_Global->col_ClearFbo);
			ofDrawRectangle(192, 50, 1536, 350);
			
			ofEnableBlendMode(OF_BLENDMODE_ADD); // 戻す.
		}
		
		if(Gui_Global->b_DrawArtSin_MaskLower){
			ofEnableBlendMode(OF_BLENDMODE_ALPHA); // OF_BLENDMODE_DISABLED, OF_BLENDMODE_ALPHA, OF_BLENDMODE_ADD
			ofSetColor(Gui_Global->col_ClearFbo);
			ofDrawRectangle(192, 450, 1536, 350);
			
			ofEnableBlendMode(OF_BLENDMODE_ADD); // 戻す.
		}
		
		/********************
		目盛り
		********************/
		{
			const float window_H = 350.0;
			float top_val_of_the_window = get_RawVal_top_of_artsin_window(window_H, Gui_Global->DrawArtSin_ScreenHeightOfOne);
			
			ofSetColor(255);
			char buf[BUF_SIZE_S];
			sprintf(buf, "%.2f", top_val_of_the_window);
			font_S.drawString(buf, 141, 55);
			font_S.drawString(buf, 141, 456);
		}
		
		/********************
		Graph title
		********************/
		if(Gui_Global->b_DrawArtSin_Overlay){
			char buf[BUF_SIZE_S];
			sprintf(buf, "Overlay");
			font_L.drawString(buf, 74, 234);
			
			sprintf(buf, "MixDown");
			font_L.drawString(buf, 74, 634);
		}else{
			char buf[BUF_SIZE_S];
			sprintf(buf, "FilterA");
			font_L.drawString(buf, 74, 234);
			
			sprintf(buf, "Normal");
			font_L.drawString(buf, 74, 634);
		}
		
	fbo.end();
}

