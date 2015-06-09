/*****************************************************************************************
Dr. Lawlor's modified ArUco marker detector:
	- Finds marker 16 in the webcam image
	- Reconstructs the camera's location relative to the marker
	- Writes the camera location and orientation to "marker.bin"
	- Periodically saves an image to vidcap.jpg and vidcaps/<date>.jpg


ArUco example Copyright 2011 Rafael Muñoz Salinas. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Rafael Muñoz Salinas ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Rafael Muñoz Salinas OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Rafael Muñoz Salinas.
********************************************************************************************/
#include <iostream>
#include <fstream>
#include <sstream>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "aruco.h"
#include "cvdrawingutils.h"
#include "errno.h"

using namespace cv;
using namespace aruco;




// Store info about how this marker is scaled, positioned and oriented
struct marker_info_t {
	int id; // marker's ID, from 0-1023 
	float true_size; // side length, in meters, of black part of pattern
	
	enum { FLAG_REFINE=1<<0,  FLAG_PARALLAX=1<<3, FLAG_CHAPMAN=1<<4 };
	int flags; // e.g. refinement
	
	int slot; // location in output array (0 for default)
	
	float x_shift; // translation from origin, in meters, of center of pattern
	float y_shift; 
	float z_shift; 
};

const double tile=4*12*0.0254; // ceiling tile spacing, in meters (4 foot size)

const static marker_info_t marker_info[]={
	{-1,0.508}, // fallback default case
	
/* 2015-05 RMC 3x3 markers 
	{3005, 0.20, marker_info_t::FLAG_PARALLAX,0,  -0.18,-0.04,0.0, 0,90 }, // main front happy
	{3004, 0.20, marker_info_t::FLAG_PARALLAX,1,  0.18, -0.44,0.0,  0,90 }, // main back pirate
	{3002, 0.10, 0,2,  0.0,-0.44,0.0,  0,90 },  // lil cyclops 
*/

/* 2015-06 Chapman 2nd floor markers */
	{0, 0.508, marker_info_t::FLAG_CHAPMAN, 0, 0.0,0.0,0.0},
	{1, 0.508, marker_info_t::FLAG_CHAPMAN, 0, (7)*tile,0.0,0.0 },
	{2, 0.508, marker_info_t::FLAG_CHAPMAN, 0, (7+6)*tile,0.0,0.0 },
	{3, 0.508, marker_info_t::FLAG_CHAPMAN, 0, (7+6+5)*tile,0.0,0.0 },

/* 2015-04 Lathrop high school markers, reinstalled at Chapman
	{0, 0.508, 0, 0, 1.0,0.0,0.0,   0,90 },
	{16,0.508, 0, 0, 4.0,0.0,0.0,   0,90 },
	{1, 0.508, 0, 0, 7.0,0.0,0.0,   0,90 },
	{2, 0.508, 0, 0, 10.0,0.0,0.0,  0,90 },
	{3, 0.508, 0, 0, 13.0,0.0,0.0,  0,90 },
	{4, 0.508, 0, 0, 16.0,0.0,0.0,  0,90 },
	{5, 0.508, 0, 0, 19.0,0.0,0.0,  0,90 },
*/
};

// Look up the calibration parameters for this marker
const marker_info_t &get_marker_info(int id) {
	for (int i=1;i<sizeof(marker_info)/sizeof(marker_info_t);i++) {
		if (marker_info[i].id==id) 
			return marker_info[i];
	}
	return marker_info[0];
}





bool showGUI=false, useRefine=false;
Mat TheInputImage,TheInputImageCopy;




/* Keep the webcam from locking up when you interrupt a frame capture.
http://lawlorcode.wordpress.com/2014/04/08/opencv-fix-for-v4l-vidioc_s_crop-error/ */
volatile int quit_signal=0;
#ifdef __unix__
#include <signal.h>
extern "C" void quit_signal_handler(int signum)
{
	if (quit_signal!=0) exit(0); // just exit already
	quit_signal=1;
	printf("Will quit at next camera frame (repeat to kill now)\n");
}
#endif


/**
  Silly hack to detect camera disconnections.
  When you unplug a video camera, it's actually detected right in:
    opencv/modules/highgui/src/cap_libv4l.cpp
  when the VIDIOC_DQBUF ioctl fails.  However, this function 
  somehow fails to correctly report the problem via the error reporting
  return code.  It does log it using perror, so I'm hooking the global perror
  to figure out if this is what went wrong, and exit appropriately.
*/
extern "C" void perror(const char *str) {
	int e=errno;
	std::cout<<"perror errno="<<e<<": "<<str<<"\n";
	if (e==ENODEV) {
		std::cout<<"ERROR!  Camera no longer connected!\n";
		std::cerr<<"ERROR!  Camera no longer connected!\n";
		exit(1);
	}
}

#include "location_binary.h"


/**
  Convert 3D position to top-down 2D onscreen location
*/
cv::Point2f to_2D(const Marker &m,float x=0.0,int xAxis=2,int yAxis=0)
{
	// Extract 3x3 rotation matrix
	Mat Rot(3,3,CV_32FC1);
	Rodrigues(m.Rvec, Rot); // euler angles to rotation matrix

	cv::Point2f ret;
	const marker_info_t &mi=get_marker_info(m.id);
	float scale=mi.true_size*70; // world meters to screen pixels
	ret.x=scale*(m.Tvec.at<float>(xAxis,0)+x*Rot.at<float>(xAxis,xAxis));
	ret.y=scale*(m.Tvec.at<float>(yAxis,0)+x*Rot.at<float>(yAxis,xAxis));
	
	printf("Screen point: %.2f, %.2f cm\n",ret.x,ret.y);
	ret.y+=240; // approximately centered in Y
	return ret;
}

/**
 Draw top-down image of reconstructed location of marker.
*/
void draw_marker_gui_2D(Mat &img,Scalar color,const Marker &m)
{
	int lineWidth=2;
	
	cv::line(img,
		to_2D(m,-0.5),to_2D(m,0.0),
		color,lineWidth,CV_AA);
	cv::line(img,
		to_2D(m,0.0),to_2D(m,+0.5),
		Scalar(255,255,255)-color,lineWidth,CV_AA);
}


class marker_parallax {
public:
	location_binary loc;
	const Marker *marker;
	float mean_X, mean_Y; // size of marker onscreen, in pixels
	float range; // distance to marker
	cv::Point2f cen; // center of marker onscreen, in pixels
};

marker_parallax parallax[8];


/* Swap these coordinate axis in this matrix */
void swap_axes(Mat &full,int axis1,int axis2) {
	for (int i=0; i<3; i++) {
		std::swap(full.at<float>(i,axis1),full.at<float>(i,axis2));
	}
}
/* Flip this coordinate axis */
void flip_axis(Mat &full,int axis) {
	for (int i=0; i<3; i++) {
		full.at<float>(i,axis) *= -1.0;
	}
}


/* Extract location data from this valid, detected marker. 
   Does not modify the location for an invalid marker.
*/
void extract_location(location_binary &bin,const Marker &marker)
{
	const marker_info_t &mi=get_marker_info(marker.id);

	// Extract 3x3 rotation matrix
	Mat Rot(3,3,CV_32FC1);
	Rodrigues(marker.Rvec, Rot); // euler angles to rotation matrix

	// Full 4x4 output matrix:
	Mat full(4,4,CV_32FC1);

	// Copy rotation 3x3
	for (int i=0; i<3; i++)
		for (int j=0; j<3; j++)
			full.at<float>(i,j)=Rot.at<float>(i,j);
	
	// Copy translation vector
	full.at<float>(0,3)=marker.Tvec.at<float>(0,0);
	full.at<float>(1,3)=marker.Tvec.at<float>(1,0);
	full.at<float>(2,3)=marker.Tvec.at<float>(2,0);
	

	// Final row is identity (nothing happening on W axis)
	full.at<float>(3,0)=0.0;
	full.at<float>(3,1)=0.0;
	full.at<float>(3,2)=0.0;
	full.at<float>(3,3)=1.0;

	if (mi.flags & marker_info_t::FLAG_CHAPMAN) {
		swap_axes(full,1,2); // swap Y and Z
		swap_axes(full,0,1); // swap X and (new) Y
		flip_axis(full,0); // X negative
		flip_axis(full,2); // Z negative
	}
/*
	if (mi.rotate2D==90) {
		for (int i=0; i<3; i++) {
			std::swap(full.at<float>(i,0),full.at<float>(i,2)); // swap X and Z
			full.at<float>(i,0)*=-1; // invert (new) X
		}
	}
	if (mi.rotate3D==90) {
		for (int i=0; i<3; i++) {
			std::swap(full.at<float>(i,1),full.at<float>(i,2)); // swap Y and Z
			//full.at<float>(i,1)*=-1; // invert (new) Y
			full.at<float>(i,0)*=-1; // invert (new) X
		}
	}
*/

	// Invert, to convert marker-from-camera into camera-from-marker
	Mat back=full.inv();

if (false) {
	// Splat to screen, for debugging
	for (int i=0; i<4; i++) {
		for (int j=0; j<4; j++)
			printf("%.2f	",back.at<float>(i,j));
		printf("\n");
	}
}
	
	bin.valid=1;
	double scale=mi.true_size;
	bin.x=back.at<float>(0,3)*scale+mi.x_shift;
	bin.y=back.at<float>(1,3)*scale+mi.y_shift;
	bin.z=back.at<float>(2,3)*scale+mi.z_shift;
	bin.angle=180.0/M_PI*atan2(back.at<float>(0,0),-back.at<float>(1,0));
	bin.marker_ID=marker.id;

	// Print grep-friendly output
	printf("Marker %d: Camera %.3f %.3f %.3f meters, heading %.1f degrees\n",
	       marker.id, bin.x,bin.y,bin.z,bin.angle
	      );

	// Dump parallax output
	if (mi.flags&marker_info_t::FLAG_PARALLAX) {
	/* Marker corners:
	   [1] [2]
	   [0] [3]
	*/
		marker_parallax &P=parallax[mi.slot];
		P.loc=bin;
		P.marker=&marker;
		P.mean_X=0.5*(cv::norm(marker[3]-marker[0])+cv::norm(marker[1]-marker[2]));
		P.mean_Y=0.5*(cv::norm(marker[3]-marker[0])+cv::norm(marker[1]-marker[2]));
		P.cen=0.25*(marker[0]+marker[1]+marker[2]+marker[3]);
		cv::Point3f xyz(bin.x,bin.y,0.0);
		P.range=cv::norm(xyz);
	}
}

/* Use parallax to improve this location */
void refine_parallax(location_binary &loc) {

// Extract parallax from marker locations
	marker_parallax &L=parallax[1], &R=parallax[0];
	float pix=cv::norm(L.cen-R.cen); // center-to-center shift, in pixels
	float markers=pix/R.mean_X; // shift, measured in terms of right markers
	const float norm_markers=37.5/20.0; // true X distance between marker centers, measured in right markers
	float delX=markers-norm_markers; // X shift, measured in markers
	const float norm_delY=1.0; // Y distance between marker centers, measured in right markers
	float range=0.5*(L.range+R.range);
	float scaledX=delX*norm_delY*range; // X shift in real units
	
// Fix up the heading by rotating about the world origin to the new X coordinates
	float old_angle=atan2(R.loc.y,-R.loc.x);
	float new_angle=atan2(range,-scaledX);
	float del_angle=(new_angle-old_angle)*180.0/M_PI;

// Write change to location
	if (range>1.5) { // far enough away parallax is better than naive
		loc.x=scaledX;
		loc.y=range;
		loc.angle+=del_angle;
		loc.valid=L.loc.valid+R.loc.valid;
	}
	
	printf("Parallax: %.1f pix, %.3f markers, %.3f delX, %.3f scaledX, %.1f degoff, %.1f heading\n",
		pix,markers,delX,scaledX,del_angle,loc.angle);
}

/* Refine this marker-derived location, using the Y-offset black spike. 
*/
void refine_location(location_binary &loc,float Y_offset /* <- units: Y distance / true size of marker */, 
	cv::Mat &cameraFrame,const Marker &marker)
{
	std::cout<<"\n\nMarker "<<marker<<"\n\n\n";
	/* Marker corners:
	   [1] [2]
	   [0] [3]
	*/
	cv::Point2f left=marker[0], top=marker[2], right=marker[3];
	cv::Point2f cen=right; // spike is aligned with big marker's right side
	float hfrac=0.7;
	cv::Point2f hdir=hfrac*(right-left); // horizontal range to check for spike
	cv::Point2f vdir=0.12*(top-right); // vertical range to check
	
	cv::Rect cameraRect(cv::Point(),cameraFrame.size());
// Search over the search space for the spike
	int nrow=(int)abs(vdir.y);
	int ncol=(int)abs(hdir.x);
	enum {nrow_check=3};
	int best_col[nrow_check]={0,0,0}; // best column number, indexed by row
	for (int row=0;row<nrow_check;row++) { // scanlines (only check bottom few)
		int darkest_x=-1;
		int darkest_g=200;
		for (int col=0;col<ncol;col++) {
			cv::Point2f p=cen + hdir*(col*(1.0/(ncol-1.0))-0.5) + vdir*(-nrow + 1 + row)*(1.0/(nrow-1.0));
			cv::Point pint((int)p.x,(int)p.y);
			if (cameraRect.contains(pint)) {
				cv::Vec3b bgr=cameraFrame.at<cv::Vec3b>(pint.y,pint.x);
				if (darkest_g>bgr[1]) {
					darkest_g=bgr[1];
					darkest_x=col;
				}
				static cv::Vec3b mark(0,255,255); 
				cameraFrame.at<cv::Vec3b>(pint.y,pint.x)=cv::Vec3b(bgr[0],255,bgr[2]); // green searchspace
				//printf("(%d,%d) ",pint.x,pint.y);
			}
		}
		if (darkest_x>0) {
			printf("row %3d: min %3d at %d (%.2f)\n",row,darkest_g,darkest_x,darkest_x*1.0/(ncol-1)-0.5);
			best_col[row]=darkest_x;
		}
	}
	if (fabs(best_col[1]-best_col[0])>1) return; // we're not reading a consistent value--skip this.
	float avg_col=(best_col[0]+best_col[1])*0.5; // column shift (relative to hdir)
	float ang_col=(avg_col*(1.0/(ncol-1))-0.5)*hfrac/Y_offset; // tangent of angle (approx == angle in radians)
	float radius=sqrt(loc.x*loc.x+loc.y*loc.y); // distance from origin of markers (meters)
	float X=-radius*ang_col; // true X coordinate, in meters
	printf("X: %.2f m.  R: %.2f m. Angle: %.2f radians.  Col: %.1f pixels\n",X,radius, ang_col,avg_col);
	
	// Update location to match new info
	loc.y=radius; // HACK: should take into account X value too
	loc.x=X;
	// FIXME: update loc.angle too (rotate from old x to new X?)
	
	
}




int main(int argc,char **argv)
{
	try {
	string TheInputVideo;
	string TheIntrinsicFile;
	int camNo=1;
	float TheMarkerSize=-1;
	int ThePyrDownLevel=0;
	MarkerDetector MDetector;
	VideoCapture vidcap;
	vector<Marker> TheMarkers;
	CameraParameters cam_param;
	float minSize=0.02; // fraction of frame, minimum size of rectangle
	pair<double,double> AvrgTime(0,0) ;//determines the average time required for detection
	int skipCount=1; // only process frames ==0 mod this
	int skipPhase=0;

	int wid=640, ht=480;
	for (int argi=1; argi<argc; argi++) {
		if (0==strcmp(argv[argi],"-gui")) showGUI=true;
		else if (0==strcmp(argv[argi],"-cam")) camNo=atoi(argv[++argi]);
		else if (0==strcmp(argv[argi],"-refine")) useRefine=true;
		else if (0==strcmp(argv[argi],"-sz")) sscanf(argv[++argi],"%dx%d",&wid,&ht);
		else if (0==strcmp(argv[argi],"-skip")) sscanf(argv[++argi],"%d",&skipCount);
		else if (0==strcmp(argv[argi],"-min")) sscanf(argv[++argi],"%f",&minSize);
		else printf("Unrecognized argument %s\n",argv[argi]);
	}

	//read from camera
	vidcap.open(camNo);
	
	vidcap.set(CV_CAP_PROP_FRAME_WIDTH, wid);
	vidcap.set(CV_CAP_PROP_FRAME_HEIGHT, ht);

	//check video is open
	if (!vidcap.isOpened()) {
		cerr<<"Could not open video"<<endl;
		return -1;
	}

	TheIntrinsicFile="camera.yml";

	//read first image to get the dimensions
	vidcap>>TheInputImage;

	//read camera parameters if passed
	if (TheIntrinsicFile!="") {
		cam_param.readFromXMLFile(TheIntrinsicFile);
		cam_param.resize(TheInputImage.size());
	}
	//Configure other parameters
	if (ThePyrDownLevel>0)
		MDetector.pyrDown(ThePyrDownLevel);
//	MDetector.setCornerRefinementMethod(MarkerDetector::SUBPIX); // more accurate
	MDetector.setCornerRefinementMethod(MarkerDetector::LINES); // more reliable?
	MDetector.setMinMaxSize(minSize,1.0); // for distant/small markers (smaller values == smaller markers, but slower too)
	// MDetector.setCornerMethod(SUBPIX); // bounces around more than "LINES"

	if (showGUI) {
		//Create gui
		cv::namedWindow("in",1);
	}

#ifdef __unix__
	signal(SIGINT,quit_signal_handler); // listen for ctrl-C
#endif
	unsigned int framecount=0;
	uint32_t vidcap_count=0;

	//capture until press ESC or until the end of the video
	while (vidcap.grab()) {
		if (!vidcap.retrieve( TheInputImage) || !vidcap.isOpened()) {
			std::cout<<"ERROR!  Camera "<<camNo<<" no longer connected!\n";
			std::cerr<<"ERROR!  Camera "<<camNo<<" no longer connected!\n";
			exit(1);
		}
		if (quit_signal) exit(0);

		// Skip frames (do no processing)
		skipPhase=(skipPhase+1)%skipCount;
		if (skipPhase!=0) continue;

		double tick = (double)getTickCount();//for checking the speed
		//Detection of markers in the image passed
		MDetector.detect(TheInputImage,TheMarkers,cam_param,1.0);
		
		//check the speed by calculating the mean speed of all iterations
		AvrgTime.first+=((double)getTickCount()-tick)/getTickFrequency();
		AvrgTime.second++;
		cout<<"Time detection="<<1000*AvrgTime.first/AvrgTime.second<<" milliseconds"<<endl;

		// Locations extracted from different markers:
		enum {n_locs=8};
		location_binary locs[n_locs];
		
		parallax[0].loc.valid=0; parallax[1].loc.valid=0;
		
		for (unsigned int i=0; i<TheMarkers.size(); i++) {
			Marker &marker=TheMarkers[i];
			const marker_info_t &mi=get_marker_info(marker.id);
			extract_location(locs[mi.slot],marker);
			if (mi.flags&marker_info_t::FLAG_REFINE)
				refine_location(locs[mi.slot],0.45/0.84,TheInputImage,marker);
		}
		
		// Extract lowest-slot location
		location_binary bin; // invalid by default
		for (int i=0;i<n_locs;i++) {
			if (locs[i].valid) { 
				bin=locs[i];
				break;
			}
		}
		
		// Check for parallax info
		if (parallax[0].loc.valid && parallax[1].loc.valid)
		{
			refine_parallax(bin);
		}

		// Dump to disk		
		static uint32_t bin_count=0;
		bin.count=bin_count++;
		bin.vidcap_count=vidcap_count;
		FILE *fbin=fopen("marker.bin","rb+"); // "r" mode avoids file truncation
		if (fbin==NULL) { // file doesn't exist (yet)
			fbin=fopen("marker.bin","w"); // create the file
			if (fbin==NULL) { // 
				printf("Error creating marker.bin output file (disk full?  permissions?)");
				exit(1);
			}
		} 
		fwrite(&bin,sizeof(bin),1,fbin); // atomic(?) file write
		fclose(fbin);

		bool vidcap=false;
		if ((framecount++%32) == 0) vidcap=true;
		if (showGUI || vidcap) {
			//print marker info and draw the markers in image
			TheInputImage.copyTo(TheInputImageCopy);
			for (unsigned int i=0; i<TheMarkers.size(); i++) {
				Marker &marker=TheMarkers[i];
				// cout<<TheMarkers[i]<<endl;
				marker.draw(TheInputImageCopy,Scalar(0,0,255),1);

				//draw a 3d cube on each marker if there is 3d info
				if (  cam_param.isValid()) {
					CvDrawingUtils::draw3dCube(TheInputImageCopy,marker,cam_param);
					CvDrawingUtils::draw3dAxis(TheInputImageCopy,marker,cam_param);
					//draw_marker_gui_2D(TheInputImageCopy,Scalar(255,255,0),marker);
				}

			}

			if (true) {
				//print other rectangles that contains invalid markers
				for (unsigned int i=0; i<MDetector.getCandidates().size(); i++) {
					aruco::Marker m( MDetector.getCandidates()[i],999);
					m.draw(TheInputImageCopy,cv::Scalar(255,0,0));
				}
			}

			if (vidcap) { // write to disk
				std::vector<int> params;
				params.push_back(CV_IMWRITE_JPEG_QUALITY);
				params.push_back(30); // <- low quality, save disk space and network bandwidth
				cv::imwrite("vidcap_next.jpg",TheInputImageCopy,params); // dump JPEG
				int ignore;
				ignore=system("mv -f vidcap_next.jpg vidcap.jpg"); // atomic(?) file replace
				ignore=system("cp vidcap.jpg vidcaps/`date '+%F__%H_%M_%S__%N'`.jpg"); // telemetry log
				vidcap_count++;
			}
		}
		if (showGUI) {
			//show input with augmented information and  the thresholded image
			cv::imshow("in",TheInputImageCopy);
			// cv::imshow("thres",MDetector.getThresholdedImage());

			char key=cv::waitKey(1);//wait for key to be pressed
			if (key=='q' || key=='x' || key==0x13) exit(0);
		} /* end showGUI */
	} /* end frame loop */

	} catch (std::exception &ex) {
		cout<<"Vision/ArUco exception: "<<ex.what()<<endl;
	}

}

