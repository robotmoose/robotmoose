#ifndef KINECT_DOA_H_INCLUDED
#define KINECT_DOA_H_INCLUDED

#include <cmath>
#include <vector>
#include <utility>
#include <algorithm>
#include <cstdint>
#include "xcor_td.h"

class Kinect_DOA {
	private:
		// Approximate linear coordinates of the Kinect's built in microphones relative to the RGB camera's position (in meters).
		//     Source: http://giampierosalvi.blogspot.com/2013/12/ms-kinect-microphone-array-geometry.html
		const double MIC_POSITIONS[4] = {0.113, -0.036, -.076, -0.113};

		const double SOUND_SPEED = 343.0; // Sound speed in meters per second.
		const double SAMPLE_FREQUENCY = 16000.0; // 16 kHz per channel.

		// The microphones will be almost nearly in sync, so only need to compute the xcor for a short window.
		const int MAX_LAG = std::round(fabs(MIC_POSITIONS[0] - MIC_POSITIONS[3])/SOUND_SPEED*SAMPLE_FREQUENCY);
		const int XCOR_WIDTH = 2*MAX_LAG;

	public:
		const int NUMSAMPLES_XCOR = 256*16;//16; // Width of the sliding window.
		int32_t * xcor_data[4];
		Kinect_DOA();
		~Kinect_DOA();
		double findAngle();

};

#endif // KINECT_DOA_H_INCLUDED