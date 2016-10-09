#ifndef KINECT_DOA_H_INCLUDED
#define KINECT_DOA_H_INCLUDED

#include <cmath>
#include <vector>
#include <utility>
#include <algorithm>
#include <stdint.h>
#include "xcor_td.h"

class Kinect_DOA {
	private:
		double MIC_POSITIONS[4];

		double SOUND_SPEED;
		double SAMPLE_FREQUENCY;

		int MAX_LAG;
		int XCOR_WIDTH;

	public:
		int NUMSAMPLES_XCOR;
		int32_t * xcor_data[4];
		Kinect_DOA();
		~Kinect_DOA();
		bool isNoise(double white_noise_ratio=0.65);
		double findAngle();

};

#endif // KINECT_DOA_H_INCLUDED