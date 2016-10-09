// Sound_Localize_Kinect.cpp
// Ryker Dial
// UAF ITEST

// Date Created: July 26, 2016
// Last Modified: August 16, 2016

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

// Libraries for skeleton tracking
#include <skeltrack/skeltrack.h>
#include "skeltrack_helper.h"
#include <glib-object.h> // Unfortunately ...

// Libraries needed for robotmoose integration
#include <string>
#include <superstar.hpp>
#include <robot_config.h>
#include <time_util.hpp>

// Libraries needed for DSP
#include <deque>
#include <vector>
#include "Kinect_DOA.h"

// ********** Constants and Variables ********** //

// ***** Freenect variables ***** //
pthread_t freenect_thread;
volatile int die = 0;

static freenect_context* f_ctx;
static freenect_device* f_dev;

static const bool ENABLE_DEPTH = true;
uint16_t t_gamma[2048];
uint8_t depth_buffer[640*480*3];
// ********** //

// ***** Constants and Variables needed for DOA Estimation ***** //
Kinect_DOA kinect_DOA;
static const bool KINECT_1473 = true; // Need to upload special firmware if using Kinect Model #1473
static const bool KINECT_UPSIDE_DOWN = true; // If Kinect is mounted upside down, flip angles.
std::vector<std::deque<int32_t> > sound_buffers;

// ***** Skeltrack Variables ***** //
static SkeltrackSkeleton *skeleton = NULL;
static SkeltrackJointList list = NULL;

// ***** Constants and Variables for RobotMoose Integration ***** //
static superstar_t * superstar;
static std::string starpath;
static std::string auth;
static Json::Value kinect;

// Used to signal main thread that data is ready for processing.
pthread_mutex_t batch_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t batch_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t kinect_json_mutex = PTHREAD_MUTEX_INITIALIZER;


//Superstar Variables...
pthread_t superstar_thread;
struct superstar_thread_request_t
{
	std::string path;
	Json::Value value;
	std::string auth;
};
std::vector<superstar_thread_request_t> superstar_queue;

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
void* superstar_threadfunc(void* arg);

int main(int argc, char* argv[]) {

	struct sigaction sa = sig_init(); // Catch Ctrl-C

	for(int i=0; i<4; ++i)
		sound_buffers.push_back(std::deque<int32_t>());

	// ***** Begin Libfreenect Setup ***** //
	// Initialize gamma correction
	// if(ENABLE_DEPTH) {
	// 	for (int i=0; i<2048; i++) {
	// 		float v = i/2048.0;
	// 		v = powf(v, 3)* 6;
	// 		t_gamma[i] = v*6*256;
	// 	}
	// }

	// Need to upload special firmware for Kinect #1473
	// For some reason, if we try to upload this when the camera and motor subdevices are selected, the upload will
	//     not work and the program can't open the audio. So the solution is to open just the audio device, upload,
	//     the firmware, then close it.
	if(KINECT_1473) {
		printf("Uploading firmware to Kinect #1473\n");
		if (freenect_init(&f_ctx, NULL) < 0) {
			printf("freenect_init() failed\n");
			return 1;
		}
		freenect_set_fw_address_nui(f_ctx, ofxKinectExtras::getFWData1473(), ofxKinectExtras::getFWSize1473());
		freenect_set_fw_address_k4w(f_ctx, ofxKinectExtras::getFWDatak4w(), ofxKinectExtras::getFWSizek4w());
		freenect_select_subdevices(f_ctx, (freenect_device_flags)(FREENECT_DEVICE_AUDIO));

		int nr_devices = freenect_num_devices (f_ctx);
		if (nr_devices < 1) {
			printf ("Number of devices found: %d\n", nr_devices);
			freenect_shutdown(f_ctx);
			return 1;
		}

		int user_device_number = 0;
		if (freenect_open_device(f_ctx, &f_dev, user_device_number) < 0) {
			printf("Could not open device\n");
			freenect_shutdown(f_ctx);
			return 1;
		}

		freenect_close_device(f_dev);
		freenect_shutdown(f_ctx);
	}

	if (freenect_init(&f_ctx, NULL) < 0) {
		printf("freenect_init() failed\n");
		return 1;
	}

	freenect_set_log_level(f_ctx, FREENECT_LOG_INFO);
	if(ENABLE_DEPTH) {
		freenect_select_subdevices(f_ctx, (freenect_device_flags)(FREENECT_DEVICE_AUDIO | FREENECT_DEVICE_CAMERA | FREENECT_DEVICE_MOTOR));
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

	int res2 = pthread_create(&superstar_thread, NULL, superstar_threadfunc, NULL);
	if (res2) {
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

	//superstar_t superstar(robot_config.get("superstar"));
	superstar = new superstar_t(robot_config.get("superstar"));
	starpath = "robots/" + robot_config.get("robot") + "/kinect";
	auth = robot_config.get("auth");
	std::cout << "Superstar is: " << robot_config.get("superstar") << std::endl;
	std::cout << "Robot is " << robot_config.get("robot") << std::endl;
	double angle = 0;

	kinect["joints"] = Json::objectValue;
	kinect["joints"]["head"] = Json::objectValue;
	kinect["joints"]["left_hand"] = Json::objectValue;
	kinect["joints"]["right_hand"] = Json::objectValue;
	kinect["joints"]["left_shoulder"] = Json::objectValue;
	kinect["joints"]["right_shoulder"] = Json::objectValue;
	kinect["joints"]["left_elbow"] = Json::objectValue;
	kinect["joints"]["right_elbow"] = Json::objectValue;

	printf("This is the Kinect localization subsystem. Press Ctrl-C to exit.\n");

	// Main loop: wait for data collection, calculate DOA, then send to Superstar
	while(!die) {
		pthread_cond_wait(&batch_cond, &batch_mutex);
		if(!kinect_DOA.isNoise(0.7)) {
			angle = kinect_DOA.findAngle();
			angle = KINECT_UPSIDE_DOWN ? -angle : angle;
			printf("The estimated angle to the source is %f degrees\n", angle);
			pthread_mutex_lock(&kinect_json_mutex);
			kinect["angle"] = angle;
			pthread_mutex_unlock(&kinect_json_mutex);
		}

	}
	return 0;
}

struct sigaction sig_init() {
	// Setup handler to catch ctrl-C. This enforces cleanup before the program exits.
	//     Code from https://gist.github.com/aspyct/3462238
	struct sigaction sa;
	sa.sa_handler = &handle_signal; // Setup the sighub handler
	sa.sa_flags = SA_RESTART; // Restart the system call, if at all possible
	sigfillset(&sa.sa_mask); // Block every signal during the handler

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

	int32_t * sound_packet[4] = {mic1, mic2, mic3, mic4};
	static int xcor_counter = 0; // Counts up to NUMSAMPLES_XCOR/2 to trigger DOA estimate.

	if(sound_buffers[0].size() < kinect_DOA.NUMSAMPLES_XCOR) {
		for(int i=0; i<4; ++i) {
			for(int j=0; j<num_samples; ++j) {
				sound_buffers[i].push_back(sound_packet[i][j]);
			}
		}
	}
	else { // buffers full
		for(int i=0; i<4; ++i) {
			for(int j=0; j<num_samples; ++j) {
				sound_buffers[i].pop_front();
				sound_buffers[i].push_back(sound_packet[i][j]);
			}
		}
	}
	xcor_counter += num_samples;
	// Perform the DOA analysis if we have collected the appropriate number of samples.
	if( (xcor_counter >= kinect_DOA.NUMSAMPLES_XCOR/2) &&
		(sound_buffers[0].size() >= kinect_DOA.NUMSAMPLES_XCOR) ) { // xcors overlap by 50%.

		pthread_mutex_lock(&batch_mutex);

		for(int i=0; i<4; ++i) {
			for(int j=0; j<kinect_DOA.NUMSAMPLES_XCOR; ++j) {
				kinect_DOA.xcor_data[i][j] = sound_buffers[i][j];
			}
		}

		xcor_counter=0;
		pthread_cond_signal(&batch_cond);
		pthread_mutex_unlock(&batch_mutex);
	}
}

std::deque<int16_t> left_elbow_buffer, right_elbow_buffer;
void depth_in_callback(freenect_device* dev, void *v_depth, uint32_t timestamp) {

	uint16_t *depth = (uint16_t*)v_depth;

	// // // Apply gamma correction to the received values
	// // for (int i=0; i<640*480; ++i) {
	// // 	int pval = t_gamma[depth[i]];
	// // 	int lb = pval & 0xff;
	// // 	switch (pval>>8) {
	// // 		case 0:
	// // 			depth_buffer[3*i+0] = 255;
	// // 			depth_buffer[3*i+1] = 255-lb;
	// // 			depth_buffer[3*i+2] = 255-lb;
	// // 			break;
	// // 		case 1:
	// // 			depth_buffer[3*i+0] = 255;
	// // 			depth_buffer[3*i+1] = lb;
	// // 			depth_buffer[3*i+2] = 0;
	// // 			break;
	// // 		case 2:
	// // 			depth_buffer[3*i+0] = 255-lb;
	// // 			depth_buffer[3*i+1] = 255;
	// // 			depth_buffer[3*i+2] = 0;
	// // 			break;
	// // 		case 3:
	// // 			depth_buffer[3*i+0] = 0;
	// // 			depth_buffer[3*i+1] = 255;
	// // 			depth_buffer[3*i+2] = lb;
	// // 			break;
	// // 		case 4:
	// // 			depth_buffer[3*i+0] = 0;
	// // 			depth_buffer[3*i+1] = 255-lb;
	// // 			depth_buffer[3*i+2] = 255;
	// // 			break;
	// // 		case 5:
	// // 			depth_buffer[3*i+0] = 0;
	// // 			depth_buffer[3*i+1] = 0;
	// // 			depth_buffer[3*i+2] = 255-lb;
	// // 			break;
	// // 		default:
	// // 			depth_buffer[3*i+0] = 0;
	// // 			depth_buffer[3*i+1] = 0;
	// // 			depth_buffer[3*i+2] = 0;
	// // 			break;
	// // 	}
	// // }
	BufferInfo *buffer_info;
	uint16_t width=640, height=480;

	uint16_t dimension_factor = 16;

	buffer_info = process_buffer(
		depth,
		width,
		height,
		dimension_factor,
		THRESHOLD_BEGIN,
		THRESHOLD_END,
		KINECT_UPSIDE_DOWN
	);

	list = skeltrack_skeleton_track_joints_sync(
		skeleton,
		buffer_info->reduced_buffer,
		buffer_info->reduced_width,
		buffer_info->reduced_height,
		NULL,
		NULL
	);

	// Get the joint data and store it in JSON format
	SkeltrackJoint * joint_ptrs[7];
	static const std::string joint_names[7] = {
		"head", "left_hand", "right_hand", "left_shoulder",
		"right_shoulder", "left_elbow", "right_elbow"
	};

	if(list) {
		joint_ptrs[0] = skeltrack_joint_list_get_joint (list, SKELTRACK_JOINT_ID_HEAD);
  		joint_ptrs[1] = skeltrack_joint_list_get_joint (list, SKELTRACK_JOINT_ID_LEFT_HAND);
  		joint_ptrs[2] = skeltrack_joint_list_get_joint (list, SKELTRACK_JOINT_ID_RIGHT_HAND);
  		joint_ptrs[3] = skeltrack_joint_list_get_joint (list, SKELTRACK_JOINT_ID_LEFT_SHOULDER);
  		joint_ptrs[4] = skeltrack_joint_list_get_joint (list, SKELTRACK_JOINT_ID_RIGHT_SHOULDER);
  		joint_ptrs[5]= skeltrack_joint_list_get_joint (list, SKELTRACK_JOINT_ID_LEFT_ELBOW);
  		joint_ptrs[6] = skeltrack_joint_list_get_joint (list, SKELTRACK_JOINT_ID_RIGHT_ELBOW);
	}
	else {
		for(int i=0; i<7; ++i) {
			joint_ptrs[i] = NULL;
		}
	}

	pthread_mutex_lock(&kinect_json_mutex);
	for(int i=0; i<7; ++i) {
		if(joint_ptrs[i]) {
			kinect["joints"][joint_names[i]]["x"] = joint_ptrs[i] -> x;
			kinect["joints"][joint_names[i]]["y"] = joint_ptrs[i] -> y;
			kinect["joints"][joint_names[i]]["z"] = joint_ptrs[i] -> z;
			kinect["joints"][joint_names[i]]["screen_x"] = joint_ptrs[i] -> screen_x;
			kinect["joints"][joint_names[i]]["screen_y"] = joint_ptrs[i] -> screen_y;
		}
		else
			kinect["joints"][joint_names[i]] = Json::nullValue;
	}
	pthread_mutex_unlock(&kinect_json_mutex);

	static unsigned long time_curr, time_last_detect;
	// Gesture recognition: Flapping arms
	if(joint_ptrs[1] && joint_ptrs[2]) { // Check elbows because hands aren't always recognized
		static const int buff_size = 30; // Store 1 second's worth of values

		time_last_detect = msl::millis();
		// For flapping arms we really only care about the change in y
		left_elbow_buffer.push_back(joint_ptrs[1] -> y);
		right_elbow_buffer.push_back(joint_ptrs[2] -> y);

		if(left_elbow_buffer.size() > buff_size)
			left_elbow_buffer.pop_front();
		if(right_elbow_buffer.size() > buff_size)
			right_elbow_buffer.pop_front();

		// Copy buffers for sorting
		std::deque<int16_t> temp_left = left_elbow_buffer, temp_right = right_elbow_buffer;
		std::sort(temp_left.begin(), temp_left.end());
		std::sort(temp_right.begin(), temp_right.end());

		int16_t range_left = *(temp_left.end()-1) - *(temp_left.begin());
		int16_t range_right = *(temp_right.end()-1) - *(temp_right.begin());

		static int16_t threshold = 800; // From experimentation
		pthread_mutex_lock(&kinect_json_mutex);
		if(range_left > threshold && range_right > threshold) {
			kinect["flapping"] = true;
		}
		else
			kinect["flapping"] = false;
		pthread_mutex_unlock(&kinect_json_mutex);
	}
	else
		kinect["flapping"] = false; // set to false if we can't see hands

	time_curr = msl::millis();
	unsigned long elapsed_ms = time_curr - time_last_detect;

	if(elapsed_ms >= 250) { // throw out stale data
		left_elbow_buffer.clear();
		right_elbow_buffer.clear();
	}


	delete buffer_info;
}

void* freenect_threadfunc(void* arg) {
	freenect_set_audio_in_callback(f_dev, audio_in_callback);
	freenect_start_audio(f_dev);
	if(ENABLE_DEPTH) {
		freenect_set_depth_callback(f_dev, depth_in_callback);
		freenect_set_depth_mode(f_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_MM));
		freenect_start_depth(f_dev);
		freenect_set_led(f_dev,LED_RED);
		skeleton = skeltrack_skeleton_new();
		g_object_set (skeleton, "enable-smoothing", true, NULL);
		g_object_set (skeleton, "smoothing-factor", 0.5, NULL);
	}

	static unsigned long time_curr, time_last_tx;
	while(!die && freenect_process_events(f_ctx) >= 0) {
		time_curr = msl::millis();
		unsigned long elapsed_ms = time_curr - time_last_tx;

		if(elapsed_ms >= 125)
		{
			superstar_thread_request_t request;
			request.path=starpath;
			request.value=kinect;
			request.auth=auth;
			pthread_mutex_lock(&kinect_json_mutex);
			superstar_queue.push_back(request);
			pthread_mutex_unlock(&kinect_json_mutex);
			time_last_tx = msl::millis();
		}
	}
	freenect_stop_audio(f_dev);
	if(ENABLE_DEPTH) {
		freenect_stop_depth(f_dev);
		freenect_set_led(f_dev,LED_BLINK_GREEN);
	}
	freenect_close_device(f_dev);
	freenect_shutdown(f_ctx);
	exit(0);
}

void* superstar_threadfunc(void* arg)
{
	while(true)
	{
		if(superstar!=NULL)
		{
			std::vector<superstar_thread_request_t> queue;
			pthread_mutex_lock(&kinect_json_mutex);
			queue=superstar_queue;
			superstar_queue.clear();
			pthread_mutex_unlock(&kinect_json_mutex);
			if(queue.size()>0)
				superstar -> set(queue[queue.size()-1].path,queue[queue.size()-1].value,queue[queue.size()-1].auth);
			superstar->flush();
		}
		msl::nsleep(1000);
	}
}