/************************************************************
************************************************************/
#include "sj_common.h"

/************************************************************
************************************************************/
/********************
********************/
int GPIO_0 = 0;
int GPIO_1 = 0;

const float _PI = 3.1415;

/********************
********************/
GUI_GLOBAL* Gui_Global = NULL;

FILE* fp_Log = nullptr;


/************************************************************
func
************************************************************/
/******************************
******************************/
double LPF(double LastVal, double NewVal, double Alpha_dt, double dt)
{
	double Alpha;
	if((Alpha_dt <= 0) || (Alpha_dt < dt))	Alpha = 1;
	else									Alpha = 1/Alpha_dt * dt;
	
	return NewVal * Alpha + LastVal * (1 - Alpha);
}

/******************************
******************************/
double LPF(double LastVal, double NewVal, double Alpha)
{
	if(Alpha < 0)		Alpha = 0;
	else if(1 < Alpha)	Alpha = 1;
	
	return NewVal * Alpha + LastVal * (1 - Alpha);
}

/******************************
******************************/
bool checkIf_ContentsExist(char* ret, char* buf)
{
	if( (ret == NULL) || (buf == NULL)) return false;
	
	string str_Line = buf;
	Align_StringOfData(str_Line);
	vector<string> str_vals = ofSplitString(str_Line, ",");
	if( (str_vals.size() == 0) || (str_vals[0] == "") ){ // no_data or exist text but it's",,,,,,,".
		return false;
	}else{
		return true;
	}
}

/******************************
******************************/
void Align_StringOfData(string& s)
{
	size_t pos;
	while((pos = s.find_first_of(" 　\t\n\r")) != string::npos){ // 半角・全角space, \t 改行 削除
		s.erase(pos, 1);
	}
}

/******************************
******************************/
void print_separatoin()
{
	printf("---------------------------------\n");
}

/******************************
******************************/
void ClearFbo(ofFbo& fbo)
{
	fbo.begin();
		ofClear(ofColor(Gui_Global->col_ClearFbo, Gui_Global->col_ClearFbo, Gui_Global->col_ClearFbo, 255));
	fbo.end();
}

/******************************
******************************/
float toRad(float val){
	return val * _PI / 180.0;
}

/******************************
******************************/
float toDeg(float val){
	return val * 180.0 / _PI;
}

/******************************
******************************/
float get_RawVal_top_of_artsin_window(float window_H, float ScreenHeightOfOne){
	if(ScreenHeightOfOne == 0) return 1e4;
	
	return 1.0 * window_H / ScreenHeightOfOne;
}

/************************************************************
class
************************************************************/

/******************************
******************************/
void GUI_GLOBAL::setup(string GuiName, string FileName, float x, float y)
{
	/********************
	********************/
	gui.setup(GuiName.c_str(), FileName.c_str(), x, y);
	
	/********************
	********************/
	Group_Audio.setup("Audio");
		Group_Audio.add(b_Audio_Start.setup("Start", false));
		Group_Audio.add(b_Audio_Stop.setup("Stop", false));
		Group_Audio.add(b_Audio_Reset.setup("Reset", false));
	gui.add(&Group_Audio);
	
	Group_FFT.setup("FFT");
		Group_FFT.add(FFT__SoftGain.setup("FFT__SoftGain", 1.0, 1.0, 5.0));
		Group_FFT.add(FFT__k_smooth.setup("FFT__k_smooth", 0.95, 0.8, 1.0));
		Group_FFT.add(FFT__dt_smooth_2.setup("FFT__dt_smooth_2", 167, 10, 300));
		Group_FFT.add(FFT__b_Window.setup("FFT__b_Window", true));
		Group_FFT.add(FFT__Afilter_0dB_at_Hz.setup("Afilter:0dB @", 1000.0, 80.0, 1000.0));
	gui.add(&Group_FFT);
	
	Group_DrawFFT.setup("DrawFFT");
		Group_DrawFFT.add(DrawFFT_ContentsCombination_id.setup("DrawFFT:ContentsId", 0.0, 0.0, 3.0));
	gui.add(&Group_DrawFFT);
	
	Group_ArtSin.setup("ArtSin");
		Group_ArtSin.add(ArtSin_Band_min__A.setup("ArtSin_Band_min__A", 1.0, 1.0, 255.0));
		Group_ArtSin.add(ArtSin_Band_max__A.setup("ArtSin_Band_max__A", 1.0, 1.0, 255.0));
		Group_ArtSin.add(ArtSin_Band_min__N.setup("ArtSin_Band_min__N", 1.0, 1.0, 255.0));
		Group_ArtSin.add(ArtSin_Band_max__N.setup("ArtSin_Band_max__N", 1.0, 1.0, 255.0));
		Group_ArtSin.add(ArtSin_PhaseMap_k.setup("ArtSin_PhaseMap_k", 1.0, 0.0, 2.0));
		Group_ArtSin.add(b_ArtSin_abs.setup("b_ArtSin_abs", false));
		Group_ArtSin.add(b_Window_artSin.setup("b_Window_artSin", false));
		Group_ArtSin.add(Tukey_alpha.setup("Tukey_alpha", 0.3, 0.0, 1.0));
	gui.add(&Group_ArtSin);
	
	Group_DrawArtSin.setup("DrawArtSin");
		Group_DrawArtSin.add(b_DrawArtSin_Overlay.setup("b_DrawArtSin_Overlay", false));
		Group_DrawArtSin.add(b_DrawArtSin_MaskUpper.setup("b_DrawArtSin_MaskUpper", false));
		Group_DrawArtSin.add(b_DrawArtSin_MaskLower.setup("b_DrawArtSin_MaskLower", false));
		Group_DrawArtSin.add(DrawArtSin_ScreenHeightOfOne.setup("DrawArtSin:H of 1.0", 350, 0.0, 1000));
		{
			ofColor initColor = ofColor(255, 255, 255, 140);
			ofColor minColor = ofColor(0, 0, 0, 0);
			ofColor maxColor = ofColor(255, 255, 255, 255);
			Group_DrawArtSin.add(DrawArtSin_col__A.setup("DrawArtSin:col_A", initColor, minColor, maxColor));
			Group_DrawArtSin.add(DrawArtSin_col__N.setup("DrawArtSin:col_N", initColor, minColor, maxColor));
			Group_DrawArtSin.add(DrawArtSin_col__MixDown.setup("DrawArtSin:col_Mix", initColor, minColor, maxColor));
		}
	gui.add(&Group_DrawArtSin);
	
	Group_misc.setup("misc");
		Group_misc.add(col_ClearFbo.setup("col clear fbo", 20, 0, 255));
	gui.add(&Group_misc);
	
	/********************
	********************/
	gui.minimizeAll();
}

