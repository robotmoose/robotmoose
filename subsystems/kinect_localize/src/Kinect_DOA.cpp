#include "Kinect_DOA.h"

Kinect_DOA::Kinect_DOA() {
	for(int i=0; i<4; ++i)
		xcor_data[i] = new int32_t[NUMSAMPLES_XCOR];
}

Kinect_DOA::~Kinect_DOA() {
	for(int i=0; i<4; ++i)
		delete [] xcor_data[i];
}

double Kinect_DOA::findAngle() {
	std::vector<std::pair<int32_t,double>> lags_and_x;
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

	std::vector<double> angles;
	for(int i=0; i<6; ++i) {
		double sin_angle = lags_and_x[i].first*SOUND_SPEED/(SAMPLE_FREQUENCY*lags_and_x[i].second);
		if(sin_angle > 1) angles.push_back(90.0);
		else if(sin_angle < -1) angles.push_back(-90.0);
		else angles.push_back(asin(sin_angle)*180.0/M_PI);
	}

	double angle_avg = 0;
	for(int i=0; i<6; ++i) {
		angle_avg += angles[i];
	}
	return angle_avg/6.0;
	// std::sort(angles.begin(), angles.end());
	// for(int i=0; i<6; ++i) {
	// 	printf("angle %d: %f\n", i, angles[i]);
	// }

/*	// Determine the median lag
	std::sort(lags_and_x.begin(), lags_and_x.end(), 
		[](const std::pair<int32_t,double> &a, const std::pair<int32_t, double> & b) {
			return b.first < a.first;
	});

	// Determine the angle using the median lag.
	double sin_angle = lags_and_x[3].first*SOUND_SPEED/(SAMPLE_FREQUENCY*lags_and_x[3].second);
	// Clamp the angle.
	if(sin_angle > 1) return 90.0;
	else if(sin_angle < -1) return -90.0;
	else return asin(sin_angle)*180/M_PI;*/
}

// Linear Prediction Filter