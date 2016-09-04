#include "Kinect_DOA.h"

#include <cstdlib> //for abs

Kinect_DOA::Kinect_DOA()
{
	// Approximate linear coordinates of the Kinect's built in microphones relative to the RGB camera's position (in meters).
	//     Source: http://giampierosalvi.blogspot.com/2013/12/ms-kinect-microphone-array-geometry.html
	MIC_POSITIONS[0]=0.113;
	MIC_POSITIONS[1]=-0.036;
	MIC_POSITIONS[2]=-.076;
	MIC_POSITIONS[3]=-0.113;

	SOUND_SPEED = 338.4; // Sound speed in meters per second.
	SAMPLE_FREQUENCY = 16000.0; // 16 kHz per channel.

	// The microphones will be almost nearly in sync, so only need to compute the xcor for a short window.
	MAX_LAG = fabs(MIC_POSITIONS[0] - MIC_POSITIONS[3])/SOUND_SPEED*SAMPLE_FREQUENCY;
	XCOR_WIDTH = 2*MAX_LAG;

	NUMSAMPLES_XCOR = 256*16; // Width of the sliding window.

	for(int i=0; i<4; ++i)
		xcor_data[i] = new int32_t[NUMSAMPLES_XCOR];
}

Kinect_DOA::~Kinect_DOA() {
	for(int i=0; i<4; ++i)
		delete [] xcor_data[i];
}

// Simple white noise filter. The higher the white noise ratio is, the easier it will be for audio to be
//     identified as non-noise.
// Source: http://stackoverflow.com/questions/3881256/can-you-programmatically-detect-white-noise
bool Kinect_DOA::isNoise(double white_noise_ratio) {
	for(int i=0; i<4; ++i) {
		uint64_t sumd0 = 0, sumd1 = 0;
		for(int j=1; j<NUMSAMPLES_XCOR; ++j) {
			sumd0 += abs(xcor_data[i][j]);
			sumd1 += abs(xcor_data[i][j]-xcor_data[i][j-1]);
		}
		if(((double)sumd1)/((double)sumd0) < white_noise_ratio)
			return false;
	}
	return true;
}

static bool sort_func(const std::pair<int32_t,double> &a, const std::pair<int32_t, double> & b)
{
	return b.first < a.first;
}

double Kinect_DOA::findAngle() {
	std::vector<std::pair<int32_t,double> > lags_and_x;
	lags_and_x.reserve(8);

	// Determine the lags between the microphone pairs.
	// Ideally, this would be done in a loop.
	lags_and_x.push_back(std::make_pair(xcor_td(xcor_data[0], xcor_data[1], NUMSAMPLES_XCOR, XCOR_WIDTH).second,
		fabs(MIC_POSITIONS[0]-MIC_POSITIONS[1])));
	lags_and_x.push_back(std::make_pair(xcor_td(xcor_data[0], xcor_data[2], NUMSAMPLES_XCOR, XCOR_WIDTH).second,
		fabs(MIC_POSITIONS[0]-MIC_POSITIONS[2])));
	lags_and_x.push_back(std::make_pair(xcor_td(xcor_data[0], xcor_data[3], NUMSAMPLES_XCOR, XCOR_WIDTH).second,
		fabs(MIC_POSITIONS[0]-MIC_POSITIONS[3])));
	lags_and_x.push_back(std::make_pair(xcor_td(xcor_data[1], xcor_data[2], NUMSAMPLES_XCOR, XCOR_WIDTH).second,
		fabs(MIC_POSITIONS[1]-MIC_POSITIONS[2])));
	lags_and_x.push_back(std::make_pair(xcor_td(xcor_data[1], xcor_data[3], NUMSAMPLES_XCOR, XCOR_WIDTH).second,
		fabs(MIC_POSITIONS[1]-MIC_POSITIONS[3])));
	lags_and_x.push_back(std::make_pair(xcor_td(xcor_data[2], xcor_data[3], NUMSAMPLES_XCOR, XCOR_WIDTH).second,
		fabs(MIC_POSITIONS[2]-MIC_POSITIONS[3])));

	// Determine the median lag
	std::sort(lags_and_x.begin(), lags_and_x.end(),sort_func);

	// Determine the angle using the median lag.
	double sin_angle = lags_and_x[3].first*SOUND_SPEED/(SAMPLE_FREQUENCY*lags_and_x[3].second);
	// Clamp the angle.
	if(sin_angle > 1) return 90.0;
	else if(sin_angle < -1) return -90.0;
	else return -asin(sin_angle)*180/M_PI;
}