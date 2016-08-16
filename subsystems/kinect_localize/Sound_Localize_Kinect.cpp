// Sound_Localize_Kinect.cpp
// Ryker Dial
// UAF ITEST

// Date Created: July 26, 2016
// Last Modified: August 15, 2016

// ***** Overview ***** //
// This program uses the microphone array on the Kinect v1 to estimate the direction to the dominant sound source
//     in a 180 degree FOV in front of the Kinect. This program sends this data to the specified RobotMoose Superstar
//     where it can be viewed.
// The micview and glview examples provided with libfreenect were used as a guide for interfacing with the Kinect.

// ***** License ***** //
// This code is licensed to you under the terms of the Apache License, version
// 2.0, or, at your option, the terms of the GNU General Public License,
// version 2.0. See the APACHE20 and GPL2 files for the text of the licenses,
// or the following URLs:
// http://www.apache.org/licenses/LICENSE-2.0
// http://www.gnu.org/licenses/gpl-2.0.txt

// General libraries
#include <cstdio> // for printf
#include <iostream>
#include <csignal> // for capturig Ctrl-C

// Libraries for libfreenect
#include <libfreenect/libfreenect.h>
#include <libfreenect/libfreenect_audio.h>
#include "ofxKinectExtras.h"
#include <pthread.h>

// Libraries needed for robotmoose integration
#include <string>
#include "superstar/superstar.hpp"
//#include "robot.hpp"
#include "robot_config.h"

// Libraries needed for DSP
#include <deque>
#include "Kinect_DOA.h"

// ********** Constants and Variables ********** //

// ***** Freenect variables ***** //
pthread_t freenect_thread;
volatile int die = 0;

static freenect_context* f_ctx;
static freenect_device* f_dev;


static const bool ENABLE_DEPTH = false;
uint16_t t_gamma[2048];
uint8_t depth_buffer[640*480*3];
// ********** //

// ***** Constants and Variables needed for DOA Estimation ***** //
Kinect_DOA kinect_DOA;
static const bool KINECT_1473 = true; // Need to upload special firmware if using Kinect Model #1473
static const bool KINECT_UPSIDE_DOWN = true; // If Kinect is mounted upside down, flip angles.

// Used to signal main thread that data is ready for processing.
pthread_mutex_t batch_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t batch_cond = PTHREAD_COND_INITIALIZER;

// ******************** //

// Function Prototypes
struct sigaction sig_init();
void handle_signal(int signal);
void audio_in_callback(freenect_device* dev, int num_samples,
				 int32_t* mic1, int32_t* mic2,
				 int32_t* mic3, int32_t* mic4,
				 int16_t* cancelled, void *unknown);
void depth_in_callback(freenect_device* dev, void *v_depth, uint32_t timestamp);
void* freenect_threadfunc(void* arg);

int main(int argc, char* argv[]) {

	struct sigaction sa = sig_init(); // Catch Ctrl-C

	// ***** Begin Libfreenect Setup ***** //
	// Initialize gamma correction
	if(ENABLE_DEPTH) {
		for (int i=0; i<2048; i++) {
			float v = i/2048.0;
			v = powf(v, 3)* 6;
			t_gamma[i] = v*6*256;
		}
	}

	if (freenect_init(&f_ctx, NULL) < 0) {
		printf("freenect_init() failed\n");
		return 1;
	}

	if(KINECT_1473) {
		// Need to upload audio firmware for Kinect model #1473
		freenect_set_fw_address_nui(f_ctx, ofxKinectExtras::getFWData1473(), ofxKinectExtras::getFWSize1473());
		freenect_set_fw_address_k4w(f_ctx, ofxKinectExtras::getFWDatak4w(), ofxKinectExtras::getFWSizek4w());
	}

	freenect_set_log_level(f_ctx, FREENECT_LOG_INFO);
	if(ENABLE_DEPTH) {
		freenect_select_subdevices(f_ctx, (freenect_device_flags)(FREENECT_DEVICE_AUDIO | FREENECT_DEVICE_CAMERA));
		printf("Depth camera is enabled\n");
	}
	else {
		freenect_select_subdevices(f_ctx, (freenect_device_flags)(FREENECT_DEVICE_AUDIO));
		printf("Depth camera is disabled\n");
	}

	int nr_devices = freenect_num_devices (f_ctx);
	printf ("Number of devices found: %d\n", nr_devices);
	if (nr_devices < 1) {
		freenect_shutdown(f_ctx);
		return 1;
	}

	int user_device_number = 0;
	if (freenect_open_device(f_ctx, &f_dev, user_device_number) < 0) {
		printf("Could not open device\n");
		freenect_shutdown(f_ctx);
		return 1;
	}

	int res = pthread_create(&freenect_thread, NULL, freenect_threadfunc, NULL);
	if (res) {
		printf("pthread_create failed\n");
		freenect_shutdown(f_ctx);
		return 1;
	}


	// ***** Begin RobotMoose Integration Setup ***** //
	robot_config_t robot_config;
	if(argc > 1) 
		robot_config.from_cli(argc, argv);
	else 
		robot_config.from_file("robot.ini");

	superstar_t superstar(robot_config.get("superstar"));
	std::string starpath = "robots/" + robot_config.get("robot") + "/kinect";
	std::string auth = robot_config.get("auth");
	std::cout << "Superstar is: " << robot_config.get("superstar") << std::endl;
	std::cout << "Robot is " << robot_config.get("robot") << std::endl;
	Json::Value kinect;
	double angle = 0;

	printf("This is the Kinect localization subsystem. Press Ctrl-C to exit.\n");

	// Main loop: wait for data collection, calculate DOA, then send to Superstar
	while(!die) {
		pthread_cond_wait(&batch_cond, &batch_mutex);
		if(!kinect_DOA.isNoise()) {
			angle = kinect_DOA.findAngle();
			angle = KINECT_UPSIDE_DOWN ? -angle : angle;
			printf("The estimated angle to the source is %f degrees\n", angle);
			kinect["angle"] = angle;
			superstar.set(starpath, kinect, auth);
			superstar.flush();
		}
	}
	return 0;
}

struct sigaction sig_init() {
	// Setup handler to catch ctrl-C. This enforces cleanup before the program exits.
	//     Code from https://gist.github.com/aspyct/3462238
	struct sigaction sa;

	// Setup the sighub handler
	sa.sa_handler = &handle_signal;

	// Restart the system call, if at all possible
	sa.sa_flags = SA_RESTART;

	// Block every signal during the handler
	sigfillset(&sa.sa_mask);

	if (sigaction(SIGINT, &sa, NULL) == -1) {
		perror("Error: cannot handle SIGINT"); // Should not happen
	}

	return sa;
}

void handle_signal(int signal) {
	switch(signal) {
		case SIGINT:
			printf("\nCleaning Up\n");
			die = 1;
			pthread_exit(NULL);
			break;
		default:
			printf("Caught wrong signal: %d\n", signal);
			return;
	}
}

void audio_in_callback(freenect_device* dev, int num_samples,
				 int32_t* mic1, int32_t* mic2,
				 int32_t* mic3, int32_t* mic4,
				 int16_t* cancelled, void *unknown) {

	static int xcor_counter = 0; // Counts up to NUMSAMPLES_XCOR/2 to trigger DOA estimate.
	static std::deque<int32_t> mic1_d, mic2_d, mic3_d, mic4_d; // Store microphone data streams

	if(mic1_d.size() < kinect_DOA.NUMSAMPLES_XCOR) { // Still filling up buffers.
		for(int i=0; i<num_samples; ++i) {
			mic1_d.push_back(mic1[i]);
			mic2_d.push_back(mic2[i]);
			mic3_d.push_back(mic3[i]);
			mic4_d.push_back(mic4[i]);
		}
	}
	else { // buffers full.
		for(int i=0; i<num_samples; ++i) {
			mic1_d.pop_front();
			mic1_d.push_back(mic1[i]);
			mic2_d.pop_front();
			mic2_d.push_back(mic2[i]);
			mic3_d.pop_front();
			mic3_d.push_back(mic3[i]);
			mic4_d.pop_front();
			mic4_d.push_back(mic4[i]);
		}
	}
	xcor_counter += num_samples;
	// Perform the DOA analysis if we have collected the appropriate number of samples.
	if( (xcor_counter >= kinect_DOA.NUMSAMPLES_XCOR/2) && (mic1_d.size() >= kinect_DOA.NUMSAMPLES_XCOR) ) { // xcors overlap by 50%.
		pthread_mutex_lock(&batch_mutex);

		uint64_t sumd0[4] = {0};
		uint64_t sumd1[4] = {0};

		for(int i=0; i<kinect_DOA.NUMSAMPLES_XCOR; ++i) {
			kinect_DOA.xcor_data[0][i] = mic1_d[i];
			kinect_DOA.xcor_data[1][i] = mic2_d[i];
			kinect_DOA.xcor_data[2][i] = mic3_d[i];
			kinect_DOA.xcor_data[3][i] = mic4_d[i];
		}

		xcor_counter=0;
		pthread_cond_signal(&batch_cond);
		pthread_mutex_unlock(&batch_mutex);
	}
}

void depth_in_callback(freenect_device* dev, void *v_depth, uint32_t timestamp) {

	uint16_t *depth = (uint16_t*)v_depth;

	for (int i=0; i<640*480; ++i) {
		int pval = t_gamma[depth[i]];
		int lb = pval & 0xff;
		switch (pval>>8) {
			case 0:
				depth_buffer[3*i+0] = 255;
				depth_buffer[3*i+1] = 255-lb;
				depth_buffer[3*i+2] = 255-lb;
				break;
			case 1:
				depth_buffer[3*i+0] = 255;
				depth_buffer[3*i+1] = lb;
				depth_buffer[3*i+2] = 0;
				break;
			case 2:
				depth_buffer[3*i+0] = 255-lb;
				depth_buffer[3*i+1] = 255;
				depth_buffer[3*i+2] = 0;
				break;
			case 3:
				depth_buffer[3*i+0] = 0;
				depth_buffer[3*i+1] = 255;
				depth_buffer[3*i+2] = lb;
				break;
			case 4:
				depth_buffer[3*i+0] = 0;
				depth_buffer[3*i+1] = 255-lb;
				depth_buffer[3*i+2] = 255;
				break;
			case 5:
				depth_buffer[3*i+0] = 0;
				depth_buffer[3*i+1] = 0;
				depth_buffer[3*i+2] = 255-lb;
				break;
			default:
				depth_buffer[3*i+0] = 0;
				depth_buffer[3*i+1] = 0;
				depth_buffer[3*i+2] = 0;
				break;
		}
	}
}

void* freenect_threadfunc(void* arg) {
	freenect_set_audio_in_callback(f_dev, audio_in_callback);
	freenect_start_audio(f_dev);
	if(ENABLE_DEPTH) {
		freenect_set_depth_callback(f_dev, depth_in_callback);
		freenect_set_depth_mode(f_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_11BIT));
		freenect_start_depth(f_dev);
	}

	while(!die && freenect_process_events(f_ctx) >= 0) {
		// If we did anything else in the freenect thread, it might go here.
	}
	freenect_stop_audio(f_dev);
	if(ENABLE_DEPTH)
		freenect_stop_depth(f_dev);
	freenect_close_device(f_dev);
	freenect_shutdown(f_ctx);
	return NULL;
}