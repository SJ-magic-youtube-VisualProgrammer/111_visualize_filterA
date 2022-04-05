/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include <ofMain.h>
#include "sj_common.h"

/************************************************************
************************************************************/

/**************************************************
**************************************************/
class CAL__A_FILTER{
private:
	const double fr = 1000;			// Hz
	const double fL = pow(10, 1.5);	// Hz
	const double fH = pow(10, 3.9);	// Hz
	const double D = sqrt(0.5);
	const double fA = pow(10, 2.45);	// Hz
	
	const double b = 1.0 / (1.0 - D) * (fr*fr + (fL*fL * fH*fH)/(fr*fr) - D * (fL*fL + fH*fH));
	const double c = fL * fL * fH * fH;
	
	const double f1 = sqrt( (-b - sqrt(b*b - 4*c))/2 );
	const double f4 = sqrt( (-b + sqrt(b*b - 4*c))/2 );
	const double f2 = (3 - sqrt(5))/2 * fA;
	const double f3 = (3 + sqrt(5))/2 * fA;
	
	double A_ofs = cal_Basic(1000);
	
	double cal_Basic(double f){
		return 20 * log10( (f4*f4 * f*f*f*f) / ( (f*f + f1*f1) * sqrt(f*f + f2*f2) * sqrt(f*f + f3*f3) * (f*f + f4*f4)  ) );
	}
	
public:
	CAL__A_FILTER()	{}
	~CAL__A_FILTER(){}
	
	void print_param(){
		printf("fr = %f\n", fr);
		printf("fL = %f\n", fL);
		printf("fH = %f\n", fH);
		printf("D  = %f\n", D);
		printf("fA = %f\n", fA);
		printf("b  = %f\n", b);
		printf("c  = %f\n", c);
		printf("f1 = %f\n", f1);
		printf("f2 = %f\n", f2);
		printf("f3 = %f\n", f3);
		printf("f4 = %f\n", f4);
	}
	
	void set__Zero_dB_at_Hz(double val) { A_ofs = cal_Basic(val); }
	
	double get_dB(double f){
		return cal_Basic(f) - A_ofs;
	}
	
	double get_X(double f){
		double dB = cal_Basic(f) - A_ofs;
		return pow(10, dB/20);
	}
};

/**************************************************
**************************************************/
class FFT_DATA_SET{
private:
	/****************************************
	****************************************/
	void cal_artSin_xy(vector <double>& artSin_x, vector <double>& artSin_y, int Band_min, int Band_max, int sgn);
	
public:
	/****************************************
	****************************************/
	vector <double> Gain;
	vector <double> Gain_smoothed;
	vector <double> Gain_smoothed_2;
	vector <double> phase_rad;
	vector <double> phase_deg;
	vector <double> phase_rad_madeFromGain;
	vector <double> artSin_x1;
	vector <double> artSin_y1;
	vector <double> artSin_x2;
	vector <double> artSin_y2;
	vector <double> artSin_x3;
	vector <double> artSin_y3;
	
	/****************************************
	****************************************/
	FFT_DATA_SET();
	~FFT_DATA_SET();
	void assign(int N, float val);
	
	void cal_Gain(const vector <double>& fft_x, const vector <double>& fft_y, float acf);
	void cal_Phase(const vector <double>& fft_x, const vector <double>& fft_y);
	void cal_Gain_smoothed(double k);
	void cal_Gain_smoothed2(double a);
	void cal_phase_MadeFromGain();
	void calArtSin_xy__TranslateToTime(int Band_min, int Band_max);
	void cal_SumOf_artSin();
};

/**************************************************
**************************************************/
class FFT : public ofThread{
private:
	/****************************************
	****************************************/
	enum{
		THREAD_SLEEP_MS = 1,
	};
	enum ANALYZE_CH{
		ANALYZE_CH__STEREO,
		ANALYZE_CH__L,
		ANALYZE_CH__R,
	};
	
	/****************************************
	****************************************/
	int t_LastUpdate = 0;
	
	/********************
	********************/
	vector <float> vol_l;
	vector <float> vol_r;
	
	/********************
	********************/
	static int N;
	static bool b_PrepParamfor_fftCal;
	static vector <double> sintbl;
	static vector <int> bitrev;
	
	/********************
	********************/
	vector <double> Hanning_window;
	vector <double> Tukey_window;
	vector <double> fft_x;
	vector <double> fft_y;
	
	FFT_DATA_SET fftDataSet_N;
	
	float acf; // Amplitude Correction Factor
	
	ANALYZE_CH AnalyzeCh = ANALYZE_CH__STEREO;
	
	/********************
	********************/
	double max_of_Afilter = 0;
	vector <double> Afilter;
	CAL__A_FILTER Cal_Afilter;
	FFT_DATA_SET fftDataSet_A;
	
	vector <double> artSin_x3_MixDown;
	
	
	/****************************************
	****************************************/
	static void PrepParamfor_fftCal();
	static void make_sintbl(void);
	static void make_bitrev(void);
	
	bool Is_Factorial_of_2(double val);
	void make_Hanning_window();
	void make_tukey_window();
	void make_Afilter();
	
	void copy_vol_to_analyzeArray();
	void multiply_HanningWindow(vector <double>& _x);
	void multiply_TukeyWindow(vector <double>& _x);
	void multiply_FilterA(vector <double>& Array);
	void MixDown_artSin();
	
public:
	/****************************************
	****************************************/
	FFT();
	~FFT();
	
	void setup();
	void update();
	
	void threadedFunction();
	
	void SetVol(ofSoundBuffer & buffer);
	void GetVol(ofSoundBuffer & buffer, bool b_EnableAudioOut);
	
	double get_val_of_Afilter(int id) const;
	double get_max_of_Afilter() const;
	double get_GainSmoothed_N(int id) const;
	double get_GainSmoothed_A(int id) const;
	double get_GainSmoothed2_N(int id) const;
	double get_GainSmoothed2_A(int id) const;
	double get_artSin_N(int id) const;
	double get_artSin_A(int id) const;
	double get_artSin_MixDown(int id) const;
	int get_sizeof_GainArray() const;
	int get_sizeof_artSinArray() const;
	
	/********************
	********************/
	// int fft(double x[], double y[], int IsReverse = 0);
	static int fft(vector <double>& x, vector <double>& y, int IsReverse = 0);
};



