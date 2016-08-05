// Sound_Localize_Kinect.cpp
// Ryker Dial
// UAF ITEST

// Date Created: July 26, 2016
// Last Modified: August 3, 2016

// ***** Overview ***** //
// This program uses the microphone array on the Kinect v1 to estimate the direction to the dominant sound source
//     in a 180 degree FOV in front of the Kinect. This program then draws an arrow on the screen that is the estimated
//     angle.
// The micview example provided with libfreenect was used as a guide to interfacing with the Kinect's microphone 
//     array using libfreenect. 

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

// Libraries for libfreenect
#include <libfreenect/libfreenect.h>
#include <libfreenect/libfreenect_audio.h>
#include "ofxKinectExtras.h"
#include <pthread.h>

// Libraries needed for OpenGL graphics
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Libraries needed for robotmoose integration
#include <fstream>
#include <string>
#include "superstar/superstar.hpp"
#include "robot.hpp"

// Libraries needed for DSP
#include <deque>
#include "Kinect_DOA.h"

// ********** Constants and Variables ********** //

// ***** Freenect variables ***** //
pthread_t freenect_thread;
volatile int die = 0;

pthread_mutex_t audiobuf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t audiobuf_cond = PTHREAD_COND_INITIALIZER;

static freenect_context* f_ctx;
static freenect_device* f_dev;
// ********** // 

// ***** Constants and Variables needed for DOA Estimation ***** // 
Kinect_DOA kinect_DOA;
int xcor_counter = 0; // Counts up to NUMSAMPLES_XCOR/2 to trigger xcor. 
std::deque<int32_t> mic1_d, mic2_d, mic3_d, mic4_d; // Store microphone data streams
static float angles[5];
static int angle_counter = 0;
static float angle = 0;

typedef struct {
	int64_t runAbsAve1;
	int64_t runAbsAve2;
} simple_filter;

static simple_filter simple_filters[4];


// ********** // 

robot_t Robot;

static const bool KINECT_1473 = true; // Need to upload special firmware if using Kinect Model #1473
static const bool KINECT_UPSIDE_DOWN = false; // If Kinect is mounted upside down, flip angles.

const GLuint WIDTH = 800, HEIGHT = 600;

// ******************** // 

// Function Prototypes
void parse_config_string(std::string line);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void in_callback(freenect_device* dev, int num_samples,
                 int32_t* mic1, int32_t* mic2,
                 int32_t* mic3, int32_t* mic4,
                 int16_t* cancelled, void *unknown);
void* freenect_threadfunc(void* arg);

int main(int argc, char* argv[]) {
	if (freenect_init(&f_ctx, NULL) < 0) {
		printf("freenect_init() failed\n");
		return 1;
	}

	// Parse either command line arguments or config file for robot config
	if(argc < 2) {
		std::ifstream robot_conf("robot.conf");
		if(robot_conf.good()) {
			std::string buffer;
			while(!robot_conf.eof()) {
				std::getline(robot_conf, buffer);
				if((buffer[0] != '#') && (buffer[0] != '\0')) // lines that start with # are comments
					parse_config_string(buffer); 
			}
			robot_conf.close();
		}
		else {
			std::cout << "Error: No robot config entered and could not open config file.\n";
			return 1;
		}
	}
	else {
		for(int i = 1; i < argc; ++i) {
			parse_config_string(argv[i]);
		}
	}
	if(Robot.name == "") {
		std::cout << "Error: No robot name entered\n";
		return 1;
	}

	if(KINECT_1473) {
		// Need to upload audio firmware for Kinect model #1473
		freenect_set_fw_address_nui(f_ctx, ofxKinectExtras::getFWData1473(), ofxKinectExtras::getFWSize1473());
		freenect_set_fw_address_k4w(f_ctx, ofxKinectExtras::getFWDatak4w(), ofxKinectExtras::getFWSizek4w());	
	}

	freenect_set_log_level(f_ctx, FREENECT_LOG_INFO);
	freenect_select_subdevices(f_ctx, FREENECT_DEVICE_AUDIO);

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

	freenect_set_user(f_dev, &mic1_d);

	freenect_set_audio_in_callback(f_dev, in_callback);
	freenect_start_audio(f_dev);

	int res = pthread_create(&freenect_thread, NULL, freenect_threadfunc, NULL);
	if (res) {
		printf("pthread_create failed\n");
		freenect_shutdown(f_ctx);
		return 1;
	}
	printf("This is the Kinect DOA viewer. Press 'esc' or 'q' to exit.\n");

	// ***** Begin Graphics Setup ***** //
	glfwInit();
	// Set required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Needed for OSX compatibility. 
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "DOA Estimate", nullptr, nullptr);
	if(window == nullptr) {
		printf("Failed to create GLFW window!\n");
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK) {
		printf("Failed to initialize GLEW!\n");
		return 1;
	}

	Shader shader(".//shaders/transformations.vs", "./shaders/transformations.frag");

    GLfloat arrow_vertices[] = {
    	0.0075f, 0.75f, 0.0f, // Top right of stem
    	0.0075f, 0.0f, 0.0f, // Bottom right of stem
    	-0.0075f, 0.0f, 0.0f, // Bottom left of stem
    	-0.0075f, 0.75f, 0.0f, // Top left of stem
    	0.025f, 0.75f, 0.0f, // Right bottom arrow tip
    	-0.025f, 0.75f, 0.0f, // Left bottom arrow tip
    	0.0f, 0.8f, 0.0f // Tip of arrow
    };
    GLuint arrow_indices[] = {
        0, 1, 3,  // First Triangle
        1, 2, 3,  // Second Triangle
        4, 5, 6   // Third Triangle
    };
    GLuint VBO[2], VAO[2], EBO[2];
    glGenVertexArrays(2, VAO);
    glGenBuffers(2, VBO);
    glGenBuffers(2, EBO);
    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
    glBindVertexArray(VAO[1]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(arrow_vertices), arrow_vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(arrow_indices), arrow_indices, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind

    glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO

	while(!glfwWindowShouldClose(window)) {
		glfwPollEvents();

        // Render
        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw our first triangle
        shader.Use();

        // Create Transformations
		glm::mat4 transform;
		transform = glm::rotate(transform, glm::radians(angle), glm::vec3(0.0, 0.0, 1.0));

		GLint transformLoc = glGetUniformLocation(shader.Program, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

        glBindVertexArray(VAO[1]);
        glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Swap the screen buffers
        glfwSwapBuffers(window);
	}

    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(2, VAO);
    glDeleteBuffers(2, VBO);
    glDeleteBuffers(2, EBO);
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
	die = 1;
	pthread_exit(NULL);
	
	// ***** End Graphics Setup ***** //
	return 0;
}

void parse_config_string(std::string str) {
	std::string key, value;
	bool key_fin = false;
	int idx = 0;
	while(true) {
		if(str[idx] == '\0') break;
		else if(str[idx] == '=') {
			key_fin = true;
			++idx;
		}
		else if(str[idx] == ' ' || str[idx] == '"' || str[idx] == '\'') 
			++idx; // skip spaces, quotations, and apostrophes
		else if(!key_fin) key += str[idx++];
		else value += str[idx++];
	}
	if(key == "robot") Robot.name = value;
	else if(key == "superstar") Robot.superstar = value;
	else if(key == "auth") Robot.auth = value;
	else std::cout << "Unrecognized Input\n";
	std::cout << "Key: " << key << std:: endl;
	std::cout << "Value: " << value << std::endl;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

void in_callback(freenect_device* dev, int num_samples,
                 int32_t* mic1, int32_t* mic2,
                 int32_t* mic3, int32_t* mic4,
                 int16_t* cancelled, void *unknown) {
	pthread_mutex_lock(&audiobuf_mutex);
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
		// static int64_t runAbsAve1[4] = {0};
		// static int64_t runAbsAve2[4] = {0};
		// for(int i=0; i<4; ++i) {
		// 	runAbsAve1[i] = 0;
		// 	runAbsAve2[i] = 0;
		// }
		// for(int i=0; i<4; ++i) {
		// 	simple_filters[i].runAbsAve1 = 0;
		// 	simple_filters[i].runAbsAve2 = 0;
		// }
			

		// // 
		uint64_t sumd0[4] = {0};
		uint64_t sumd1[4] = {0};

		for(int i=0; i<kinect_DOA.NUMSAMPLES_XCOR; ++i) {
			kinect_DOA.xcor_data[0][i] = mic1_d[i];
			kinect_DOA.xcor_data[1][i] = mic2_d[i];
			kinect_DOA.xcor_data[2][i] = mic3_d[i];
			kinect_DOA.xcor_data[3][i] = mic4_d[i];

			//printf("i: %d\n", i);
			if(i>0) {
				for(int j=0; j<4; ++j) {
					sumd0[j] += abs(kinect_DOA.xcor_data[j][i]);
					sumd1[j] += abs(kinect_DOA.xcor_data[j][i]-kinect_DOA.xcor_data[j][i-1]);
				}
			}

		// 	// for(int j=0; j<4; ++j) {
		// 	// 	mic_data << std::to_string(kinect_DOA.xcor_data[j][i]) << ",";
		// 	// }
		// 	// mic_data << "\n";

			if(i >= 24 && i <= 24+16) {
				for(int j=0; j<4; ++j)
					simple_filters[j].runAbsAve1 += (abs(kinect_DOA.xcor_data[j][i]) - abs(kinect_DOA.xcor_data[j][i-24]));
			}
			if(i >= 24 + 16 && i <= 24+32) {
				for(int j=0; j<4; ++j)
					simple_filters[j].runAbsAve2 += (abs(kinect_DOA.xcor_data[j][i]) - abs(kinect_DOA.xcor_data[j][i-24]));
			}
		}

		// http://stackoverflow.com/questions/3881256/can-you-programmatically-detect-white-noise
		double wNoiseRatio = 0.7; // The higher this is, the lower the threshhold for audio to be detected as
								  //     non white noise
		bool whiteNoise = true;
		for(int i=0; i<4; ++i) {
			//printf(sumd1[i]/sumd0[i]);
			//printf("Sumd0[%d]: %lu, Sumd1[%d]: %lu\n", i, sumd0[i], i, sumd1[i]);
			if(((double)sumd1[i])/((double)sumd0[i]) < wNoiseRatio)
				//printf("Channel %d is not like noise\n", i);
				whiteNoise = false;
		}

		// int unusualDif = 1;
		// for(int i=0; i<4; ++i) {
		// 	// printf("runAbsAve1[%d]: %ld\n", i, simple_filters[i].runAbsAve1);
		// 	// printf("runAbsAve2[%d]: %ld\n", i, simple_filters[i].runAbsAve2);
		// 	// printf("runAbsAve1[%d] + runAbsAve2[%d]: %lu\n", i, i, runAbsAve1[i] + runAbsAve2[i]);
		// 	//printf()
		// 	if(abs(simple_filters[i].runAbsAve1-simple_filters[i].runAbsAve2)>(simple_filters[i].runAbsAve1+simple_filters[i].runAbsAve2)/(2*unusualDif)) {
		// 		printf("Channel %d is not like white noise\n", i);
		// 	}
		// }

		xcor_counter=0;

		pthread_cond_signal(&audiobuf_cond);
		pthread_mutex_unlock(&audiobuf_mutex);
		if(!whiteNoise) {	
			angles[angle_counter] = kinect_DOA.findAngle();

			if((++angle_counter >= 2)) {
				 if(!(fabs(angles[0]-angles[1]) > 20.0)) {
					float angle_avg = 0;
					for(int i=0; i<2; ++i)
						angle_avg += angles[i];
					angle = angle_avg/2;
					angle = KINECT_UPSIDE_DOWN ? -angle : angle;
				 }
				angle_counter = 0;
			}
			//printf("The estimated angle to the source is %f degrees\n", angle);
		}
	}
	else {
		pthread_cond_signal(&audiobuf_cond);
		pthread_mutex_unlock(&audiobuf_mutex);		
	}
}

void* freenect_threadfunc(void* arg) {
	while(!die && freenect_process_events(f_ctx) >= 0) {
		// If we did anything else in the freenect thread, it might go here.
	}
	freenect_stop_audio(f_dev);
	freenect_close_device(f_dev);
	freenect_shutdown(f_ctx);
	return NULL;
}