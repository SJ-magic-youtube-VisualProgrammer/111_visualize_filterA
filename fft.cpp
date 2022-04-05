/************************************************************
************************************************************/
#include "fft.h"

/************************************************************
FFT_DATA_SET class
************************************************************/

/******************************
******************************/
FFT_DATA_SET::FFT_DATA_SET(){
}

/******************************
******************************/
FFT_DATA_SET::~FFT_DATA_SET(){
}

/******************************
******************************/
void FFT_DATA_SET::assign(int N, float val){
	Gain.assign(N/2, val);
	Gain_smoothed.assign(N/2, val);
	Gain_smoothed_2.assign(N/2, val);
	phase_rad.assign(N/2, val);
	phase_deg.assign(N/2, val);
	phase_rad_madeFromGain.assign(N/2, val);
	artSin_x1.assign(N, val);
	artSin_y1.assign(N, val);
	artSin_x2.assign(N, val);
	artSin_y2.assign(N, val);
	artSin_x3.assign(N, val);
	artSin_y3.assign(N, val);
}

/******************************
******************************/
void FFT_DATA_SET::cal_Gain(const vector <double>& fft_x, const vector <double>& fft_y, float acf)
{
	for(int i = 0; i < Gain.size(); i++){
		Gain[i] = 2 * sqrt( fft_x[i] * fft_x[i] + fft_y[i] * fft_y[i] ) / acf * Gui_Global->FFT__SoftGain;
	}
}

/******************************
******************************/
void FFT_DATA_SET::cal_Phase(const vector <double>& fft_x, const vector <double>& fft_y)
{
	for(int i = 0; i < phase_rad.size(); i++){
		if(Gain[i] == 0)	phase_rad[i] = 0;
		else				phase_rad[i] = atan2(fft_y[i], fft_x[i]);
		
		if(0 < i) phase_rad[i] /= i;
		
		phase_deg[i] = phase_rad[i] * 180.0 / _PI + 180;
	}
}

/******************************
******************************/
void FFT_DATA_SET::cal_Gain_smoothed(double k)
{
	for(int i = 0; i < Gain.size(); i++){
		Gain_smoothed[i] = max( Gain_smoothed[i] * k, Gain[i] );
	}
}

/******************************
******************************/
void FFT_DATA_SET::cal_Gain_smoothed2(double a)
{
	if(1.0 < a)	a = 1.0;
	if(a < 0)	a = 0.0;
	
	for(int i = 0; i < Gain.size(); i++){
		Gain_smoothed_2[i] = Gain[i] * a + Gain_smoothed_2[i] * (1 - a);
	}
}

/******************************
******************************/
void FFT_DATA_SET::cal_phase_MadeFromGain()
{
	for(int i = 0; i < phase_rad_madeFromGain.size(); i++){
		if(i == 1)	phase_rad_madeFromGain[i] = ofMap( Gain_smoothed_2[i], 0.0, 1.0, 0.0, Gui_Global->ArtSin_PhaseMap_k * _PI );
		else		phase_rad_madeFromGain[i] = ofMap( Gain_smoothed_2[i], 0.0, 1.0, 0.0, 2 * _PI );
	}
	
}

/******************************
******************************/
void FFT_DATA_SET::calArtSin_xy__TranslateToTime(int Band_min, int Band_max)
{
	cal_artSin_xy(artSin_x1, artSin_y1, Band_min, Band_max, 1);
	cal_artSin_xy(artSin_x2, artSin_y2, Band_min, Band_max, -1);
	
	FFT::fft(artSin_x1, artSin_y1, true);
	FFT::fft(artSin_x2, artSin_y2, true);
}

/******************************
******************************/
void FFT_DATA_SET::cal_artSin_xy(vector <double>& artSin_x, vector <double>& artSin_y, int Band_min, int Band_max, int sgn)
{
	if(sgn < 0)	sgn = -1;
	else		sgn = 1;
	
	artSin_x[0] = 0;
	artSin_y[0] = 0;
	artSin_x[artSin_x.size()/2] = 0;
	artSin_y[artSin_y.size()/2] = 0;
	
	for(int i = 1; i < artSin_x.size()/2; i++){
		if( (Band_min <= i) && (i <= Band_max) ){
			artSin_x[i] = Gain_smoothed[i] / 2 * cos(sgn * phase_rad_madeFromGain[i]);
			artSin_y[i] = Gain_smoothed[i] / 2 * sin(sgn * phase_rad_madeFromGain[i]);
		}else{
			artSin_x[i] = 0;
			artSin_y[i] = 0;
		}
		
		artSin_x[artSin_x.size() - i] = artSin_x[i];
		artSin_y[artSin_y.size() - i] = -artSin_y[i]; // 共役
	}
}

/******************************
******************************/
void FFT_DATA_SET::cal_SumOf_artSin()
{
	for(int i = 0; i < artSin_x3.size(); i++){
		if(Gui_Global->b_ArtSin_abs)	artSin_x3[i] = max( max( abs(artSin_x1[i]), abs(artSin_x2[i]) ), 0.0 );
		else							artSin_x3[i] = max( max( artSin_x1[i], artSin_x2[i] ), 0.0 );
	}
}


/************************************************************
FFT class
************************************************************/
/****************************************
static
****************************************/
int FFT::N;
bool FFT::b_PrepParamfor_fftCal = false;
vector <double> FFT::sintbl;
vector <int> FFT::bitrev;

/****************************************
****************************************/

/******************************
******************************/
FFT::FFT()
{
}

/******************************
******************************/
FFT::~FFT()
{
}

/******************************
******************************/
void FFT::threadedFunction()
{
	while(isThreadRunning()) {
		lock();
		
		unlock();
		
		sleep(THREAD_SLEEP_MS);
	}
}

/******************************
******************************/
void FFT::setup()
{
	/********************
	********************/
	print_separatoin();
	printf("> setup : FFT\n");
	fflush(stdout);
	
	/********************
	********************/
	if(!Is_Factorial_of_2(AUDIO_BUF_SIZE)) { ERROR_MSG(); std::exit(1); }
	
	/********************
	■std::vectorのresizeとassignの違い (C++)
		https://minus9d.hatenablog.com/entry/2021/02/07/175159
	********************/
	vol_l.assign(AUDIO_BUF_SIZE, 0.0);
	vol_r.assign(AUDIO_BUF_SIZE, 0.0);
	
	/**************************
	**************************/
	N = AUDIO_BUF_SIZE;
	
	if(!b_PrepParamfor_fftCal) PrepParamfor_fftCal();
	
	Hanning_window.resize(N);
	Tukey_window.resize(N);
	fft_x.assign(N, 0.0);
	fft_y.assign(N, 0.0);
	
	// A Filter
	fftDataSet_N.assign(N, 0.0);
	fftDataSet_A.assign(N, 0.0);
	Afilter.assign(N/2, 0.0);
	Cal_Afilter.set__Zero_dB_at_Hz(Gui_Global->FFT__Afilter_0dB_at_Hz);
	make_Afilter();
	
	artSin_x3_MixDown.assign(N, 0.0);
	
	// 窓関数
	make_Hanning_window();
	make_tukey_window();
	
	// 補正係数
	float sum_window = 0;
	for(int i = 0; i < N; i++){
		sum_window += Hanning_window[i];
	}
	acf = sum_window / N;
	printf("> acf = %f\n", acf);
	fflush(stdout);
}

/******************************
******************************/
void FFT::make_Afilter(){
	double Base_Freq = double(AUDIO_SAMPLERATE) / AUDIO_BUF_SIZE;
	
	Afilter[0] = 0;
	max_of_Afilter = Afilter[0];
	for(int i = 1; i < Afilter.size(); i++){
		// Afilter[i] = Cal__A_Filter.get_dB(Base_Freq * i);
		Afilter[i] = Cal_Afilter.get_X(Base_Freq * i);
		
		if(max_of_Afilter < Afilter[i]) max_of_Afilter = Afilter[i];
	}
}

/******************************
******************************/
void FFT::make_Hanning_window()
{
	/********************
	********************/
	for(int i = 0; i < N; i++)	Hanning_window[i] = 0.5 - 0.5 * cos(2 * _PI * i / N); // Hanning.
	
	/********************
	********************/
	/*
	static bool b_1st = true;
	if(b_1st){
		fprintf(fp_Log, ",Hanning\n");
		for(int i = 0; i < N; i++){
			fprintf(fp_Log, "%d,%f\n", i,Hanning_window[i]);
		}
		fprintf(fp_Log, "\n\n");
		
		b_1st = false;
	}
	*/
}

/******************************
******************************/
void FFT::make_tukey_window()
{
	/********************
	********************/
	for(int i = 0; i < N; i++){
		if(i <= float(N) * Gui_Global->Tukey_alpha / 2){
			Tukey_window[i] = 0.5 - 0.5 * cos( 2 * _PI / (N * Gui_Global->Tukey_alpha) * i );
		}else if( float(N) - float(N) * Gui_Global->Tukey_alpha / 2 <= i ){
			Tukey_window[i] = 0.5 - 0.5 * cos( 2 * _PI / (N * Gui_Global->Tukey_alpha) * (i - (N - N * Gui_Global->Tukey_alpha)) );
		}else{
			Tukey_window[i] = 1.0;
		}
	}
	
	/********************
	********************/
	/*
	static bool b_1st = true;
	if(b_1st){
		fprintf(fp_Log, ",Tukey\n");
		for(int i = 0; i < N; i++){
			fprintf(fp_Log, "%d,%f\n", i,Tukey_window[i]);
		}
		fprintf(fp_Log, "\n\n");
		
		b_1st = false;
	}
	*/
}

/******************************
******************************/
bool FFT::Is_Factorial_of_2(double val)
{
	double ret = log2(val);
	
	if( ret - (int)ret  == 0 )	return true;
	else						return false;
}

/******************************
******************************/
void FFT::update()
{
	/********************
	********************/
	int now = ofGetElapsedTimeMillis();
	int dt = now - t_LastUpdate;
	
	/********************
	********************/
	copy_vol_to_analyzeArray();
	
	if(Gui_Global->FFT__b_Window) multiply_HanningWindow(fft_x);
	
	fft(fft_x, fft_y);
	/*
	static int c_sj = 0;
	if(c_sj < 100){
		fprintf(fp_Log, "(,%f, %f,), (,%f, %f,)\n", fft_x[0], fft_y[0], fft_x[256], fft_y[256]);
		c_sj++;
	}
	*/
	
	/********************
	********************/
	fftDataSet_N.cal_Gain(fft_x, fft_y, Gui_Global->FFT__b_Window ? acf : 1.0);
	fftDataSet_N.cal_Phase(fft_x, fft_y);
	
	fftDataSet_A.cal_Gain(fft_x, fft_y, Gui_Global->FFT__b_Window ? acf : 1.0);
	fftDataSet_A.cal_Phase(fft_x, fft_y);
	
	/********************
	********************/
	static float FFT__Afilter_0dB_at_Hz = Gui_Global->FFT__Afilter_0dB_at_Hz;
	if(Gui_Global->FFT__Afilter_0dB_at_Hz != FFT__Afilter_0dB_at_Hz){
		Cal_Afilter.set__Zero_dB_at_Hz(Gui_Global->FFT__Afilter_0dB_at_Hz);
		make_Afilter();
		
		FFT__Afilter_0dB_at_Hz = Gui_Global->FFT__Afilter_0dB_at_Hz;
	}
	
	multiply_FilterA(fftDataSet_A.Gain);
	
	/********************
	********************/
	fftDataSet_N.cal_Gain_smoothed(Gui_Global->FFT__k_smooth);
	fftDataSet_A.cal_Gain_smoothed(Gui_Global->FFT__k_smooth);
	{
		double tan = 1.0 / Gui_Global->FFT__dt_smooth_2;
		double alpha = tan * dt;
		
		fftDataSet_N.cal_Gain_smoothed2(alpha);
		fftDataSet_A.cal_Gain_smoothed2(alpha);
	}
	fftDataSet_N.cal_phase_MadeFromGain();
	fftDataSet_A.cal_phase_MadeFromGain();
	
	fftDataSet_N.calArtSin_xy__TranslateToTime(Gui_Global->ArtSin_Band_min__N, Gui_Global->ArtSin_Band_max__N);
	fftDataSet_A.calArtSin_xy__TranslateToTime(Gui_Global->ArtSin_Band_min__A, Gui_Global->ArtSin_Band_max__A);
	
	make_tukey_window();
	fftDataSet_N.cal_SumOf_artSin();
	fftDataSet_A.cal_SumOf_artSin();
	if(Gui_Global->b_Window_artSin){
		multiply_TukeyWindow(fftDataSet_N.artSin_x3);
		multiply_TukeyWindow(fftDataSet_A.artSin_x3);
	}
	
	/********************
	********************/
	MixDown_artSin();
	
	/********************
	********************/
	t_LastUpdate = now;
}

/******************************
******************************/
void FFT::MixDown_artSin()
{
	for(int i = 0; i < artSin_x3_MixDown.size(); i++){
		artSin_x3_MixDown[i] = max( fftDataSet_N.artSin_x3[i], fftDataSet_A.artSin_x3[i] );
	}
}

/******************************
******************************/
double FFT::get_val_of_Afilter(int id) const
{
	if(Afilter.size() <= id)	return 0;
	else						return Afilter[id];
}
/******************************
******************************/
double FFT::get_max_of_Afilter() const
{
	return max_of_Afilter;
}

/******************************
******************************/
double FFT::get_GainSmoothed_N(int id) const
{
	if(fftDataSet_N.Gain_smoothed.size() <= id)	return 0;
	else										return fftDataSet_N.Gain_smoothed[id];
}

/******************************
******************************/
double FFT::get_GainSmoothed_A(int id) const
{
	if(fftDataSet_A.Gain_smoothed.size() <= id)	return 0;
	else										return fftDataSet_A.Gain_smoothed[id];
}

/******************************
******************************/
double FFT::get_GainSmoothed2_N(int id) const
{
	if(fftDataSet_N.Gain_smoothed_2.size() <= id)	return 0;
	else											return fftDataSet_N.Gain_smoothed_2[id];
}

/******************************
******************************/
double FFT::get_GainSmoothed2_A(int id) const
{
	if(fftDataSet_A.Gain_smoothed_2.size() <= id)	return 0;
	else											return fftDataSet_A.Gain_smoothed_2[id];
}

/******************************
******************************/
int FFT::get_sizeof_GainArray() const
{
	return fftDataSet_N.Gain.size();
}

/******************************
******************************/
double FFT::get_artSin_N(int id) const
{
	if(fftDataSet_N.artSin_x3.size() <= id)	return 0;
	else									return fftDataSet_N.artSin_x3[id];
}

/******************************
******************************/
double FFT::get_artSin_A(int id) const
{
	if(fftDataSet_A.artSin_x3.size() <= id)	return 0;
	else									return fftDataSet_A.artSin_x3[id];
}

/******************************
******************************/
double FFT::get_artSin_MixDown(int id) const
{
	if(artSin_x3_MixDown.size() <= id)	return 0;
	else								return artSin_x3_MixDown[id];
}

/******************************
******************************/
int FFT::get_sizeof_artSinArray() const
{
	return fftDataSet_N.artSin_x3.size();
}

/******************************
******************************/
void FFT::multiply_HanningWindow(vector <double>& _x)
{
	if(_x.size() != Hanning_window.size()) return;
	
	for(int i = 0; i < _x.size(); i++){
		_x[i] = _x[i] * Hanning_window[i];
	}
}

/******************************
******************************/
void FFT::multiply_TukeyWindow(vector <double>& _x)
{
	if(_x.size() != Tukey_window.size()) return;
	
	for(int i = 0; i < _x.size(); i++){
		_x[i] = _x[i] * Tukey_window[i];
	}
}

/******************************
******************************/
void FFT::multiply_FilterA(vector <double>& Array)
{
	if(Array.size() != Afilter.size()) return;
	
	for(int i = 0; i < Array.size(); i++){
		Array[i] = Array[i] * Afilter[i];
	}
}

/******************************
******************************/
void FFT::copy_vol_to_analyzeArray()
{
	lock();
		for(int i = 0; i < N; i++){
			/********************
			********************/
			switch(AnalyzeCh){
				case ANALYZE_CH__STEREO:
					fft_x[i] = (vol_l[i] + vol_r[i]) / 2;
					break;
					
				case ANALYZE_CH__L:
					fft_x[i] = vol_l[i];
					break;
					
				case ANALYZE_CH__R:
					fft_x[i] = vol_r[i];
					break;
			}
			
			/********************
			********************/
			
			fft_y[i] = 0;
		}
	unlock();
}

/******************************
******************************/
void FFT::SetVol(ofSoundBuffer & buffer)
{
	static int c = 0;
	lock();
		
		if( (vol_l.size() < buffer.getNumFrames()) || (vol_r.size() < buffer.getNumFrames()) ){
			ERROR_MSG();
			std::exit(1);
		}else{
			for (size_t i = 0; i < buffer.getNumFrames(); i++){
				vol_l[i] = buffer[i*2 + 0];
				vol_r[i] = buffer[i*2 + 1];
			}
			if(c < 2){
				printf("Audio in  buffer = %p\n", &buffer[0]); // float & ofSoundBuffer::operator[](std::size_t pos) { return buffer[pos]; } // ofSoundBuffer.cpp
				c++;
			}
		}
	
	unlock();
}

/******************************
******************************/
void FFT::GetVol(ofSoundBuffer & buffer, bool b_EnableAudioOut)
{
	static int c = 0;
	lock();
	
		if( (vol_l.size() < buffer.getNumFrames()) || (vol_r.size() < buffer.getNumFrames()) ){
			ERROR_MSG();
			std::exit(1);
		}else{
			for (size_t i = 0; i < buffer.getNumFrames(); i++){
				if(b_EnableAudioOut){
					buffer[i*2 + 0]	= vol_l[i];
					buffer[i*2 + 1]	= vol_r[i];
				}else{
					buffer[i*2 + 0]	= 0;
					buffer[i*2 + 1]	= 0;
				}
			}
			if(c < 2){
				printf("Audio out buffer = %p\n", &buffer[0]);  // float & ofSoundBuffer::operator[](std::size_t pos) { return buffer[pos]; } // ofSoundBuffer.cpp
				c++;
			}
		}
	
	unlock();

}

/******************************
******************************/
void FFT::PrepParamfor_fftCal()
{
	/*****************
	*****************/
	N = AUDIO_BUF_SIZE;
	sintbl.resize(N + N/4);
	bitrev.resize(N);
	
	/*****************
	*****************/
	// table
	make_sintbl();
	make_bitrev();
	
	/*****************
	*****************/
	b_PrepParamfor_fftCal = true;
}

/******************************
******************************/
int FFT::fft(vector <double>& x, vector <double>& y, int IsReverse)
{
	/*****************
	*****************/
	if(!b_PrepParamfor_fftCal) PrepParamfor_fftCal();
	
	/*****************
		bit反転
	*****************/
	int i, j;
	for(i = 0; i < N; i++){
		j = bitrev[i];
		if(i < j){
			double t;
			t = x[i]; x[i] = x[j]; x[j] = t;
			t = y[i]; y[i] = y[j]; y[j] = t;
		}
	}

	/*****************
		変換
	*****************/
	int n4 = N / 4;
	int k, ik, h, d, k2;
	double s, c, dx, dy;
	for(k = 1; k < N; k = k2){
		h = 0;
		k2 = k + k;
		d = N / k2;

		for(j = 0; j < k; j++){
			c = sintbl[h + n4];
			if(IsReverse)	s = -sintbl[h];
			else			s = sintbl[h];

			for(i = j; i < N; i += k2){
				ik = i + k;
				dx = s * y[ik] + c * x[ik];
				dy = c * y[ik] - s * x[ik];

				x[ik] = x[i] - dx;
				x[i] += dx;

				y[ik] = y[i] - dy;
				y[i] += dy;
			}
			h += d;
		}
	}

	/*****************
	*****************/
	if(!IsReverse){
		for(i = 0; i < N; i++){
			x[i] /= N;
			y[i] /= N;
		}
	}

	return 0;
}

/******************************
******************************/
void FFT::make_bitrev(void)
{
	int i, j, k, n2;

	n2 = N / 2;
	i = j = 0;

	for(;;){
		bitrev[i] = j;
		if(++i >= N)	break;
		k = n2;
		while(k <= j)	{j -= k; k /= 2;}
		j += k;
	}
}

/******************************
******************************/
void FFT::make_sintbl(void)
{
	for(int i = 0; i < N + N/4; i++){
		sintbl[i] = sin(2 * _PI * i / N);
	}
}
