/**
  An extremely tiny and ill-featured camera class.
  Idea is you:
  
     call oglCameraLookAt(...) inside your display function, after gluPerspective.
     call oglCameraInit() inside your main function
  
  This file doesn't depend on anything else except glut and vec3.
*/
#include "GL/glut.h" /* windows, mice, etc.  http://freeglut.sourceforge.net */
#include "osl/vec4.h" /* 3D vector class */
#define VEC3_TO_XYZ(v) (v).x,(v).y,(v).z /* convert a vec3 to three float arguments */

#if USE_OGL_JOYSTICK /* enable joystick control (more code, but fun!) */
#include "ogl/event.h"
#include "ogl/event.cpp"
#endif


/* 3D orientation */
/* Orthonormal coordinate frame */
class ortho_frame {
public:
	/* Unit vectors pointing along axes of our frame.
		X cross Y is Z. (right-handed coordinate system)
	*/
	vec3 x,y,z;
	ortho_frame() :x(1,0,0), y(0,0,1), z(0,-1,0) {} /* point down Y axis */
	
	/* Reorient this coordinate frame by this far along in the X and Y axes.  
	   "dx" is the distance along the z axis to push the x axis; 
	   "dy" the distance along the z axis to push the y axis.
	*/
	void nudge(double dx,double dy) {
		x+=dx*z;
		y+=dy*z;
		orthonormalize();
	}
	
	/* Reconstruct an orthonormal frame from X and Y axes.
	  Y is primary, X is secondary, Z is tertiary.
	*/
	void orthonormalize(void) {
		y=normalize(y);
		z=normalize(cross(x,y));
		x=normalize(cross(y,z));
	}
};
ortho_frame camera_orient; /* camera orientation */
vec3 camera(0,0,0); /* position of camera (set up in main) */
bool key_down[256]; /* if true, the corresponding key is down */
int key_down_count=0; /* number of keys currently pressed */
bool key_first_down=false; /* redisplay just requested by keyboard function */
int mouse_down_count=0; /* number of buttons currently pressed */
bool correct_head_tilt=true; /* force camera X axis to remain level */
bool correct_head_tilt_sphere=false; /* stay level with planet (at origin) */

/**
  Camera moves at the given real velocity in world-units per second.
*/
void oglCameraLookAt(float velocity=3.0)
{
	static float display_last_t=0.0;
	glFinish(); /* makes timer below run *way* smoother... */
	float display_cur_t=0.001*glutGet(GLUT_ELAPSED_TIME);
	float display_dt=display_cur_t-display_last_t; /* seconds per frame */
	display_last_t=display_cur_t;
	if (key_first_down) {
		display_dt=0.0; /* avoid jerky startup */
		key_first_down=false;
	} 
	if (key_down_count>0 || mouse_down_count>0) { /* auto animate */
		glutPostRedisplay();
	}
	
	/* Physics and display timesteps: */
	//printf("dt=%.3f ms\n",display_dt);
	float vel=velocity*display_dt; /* meters camera motion per frame */
	if (key_down[(int)'w']) camera-=vel*camera_orient.z;
	if (key_down[(int)'s']) camera+=vel*camera_orient.z;
	if (key_down[(int)'a']) camera-=vel*camera_orient.x;
	if (key_down[(int)'d']) camera+=vel*camera_orient.x;
	if (key_down[(int)'q']) camera+=vel*camera_orient.y;
	if (key_down[(int)'z']) camera-=vel*camera_orient.y;
	
	if (key_down[(int)0x1B]) exit(0); /* escape key */
	
	if (correct_head_tilt) {
		/* Don't allow head tilting! */
		camera_orient.x.z=0.0; 
		camera_orient.y=cross(camera_orient.z,camera_orient.x);
		camera_orient.orthonormalize();
	}
	if (correct_head_tilt_sphere) {
		vec3 upvector=normalize(camera);
		camera_orient.x-=upvector*dot(upvector,camera_orient.x);
		camera_orient.y=cross(camera_orient.z,camera_orient.x);
		//camera_orient.y=camera; /* Y axis always faces away from origin */
		camera_orient.orthonormalize();
	}

#if USE_OGL_JOYSTICK /* add joystick contributions */
	static double zvel=0.0;
	double damping=10.0*display_dt;
	if (damping>1.0) damping=0.9;
	zvel=zvel*(1.0-damping); /* apply damping */
	zvel+=0.1*(oglButton(7,"Z up")-oglButton(8,"Z down")); /* power */
	camera+=vel*oglAxis(1,"X move")*camera_orient.x
		+vel*oglAxis(2,"Y move")*camera_orient.z
		+vel*zvel*camera_orient.y;
	double ovel=1.0*display_dt;
	camera_orient.nudge(
		ovel*oglAxis(4,"X look"),
		-ovel*oglAxis(3,"Y look"));
#endif
	
	/* Slowly blend in new camera orientation (smooth mouse jerks) */
	static ortho_frame slow_orient=camera_orient;
	float lerpfrac=1.0-exp(-10.0*display_dt); /* mouse motion half-life */
	lerpfrac=clamp(lerpfrac,0.01,1.0);
	slow_orient.x=mix(slow_orient.x,camera_orient.x,lerpfrac);
	slow_orient.y=mix(slow_orient.y,camera_orient.y,lerpfrac);
	slow_orient.z=mix(slow_orient.z,camera_orient.z,lerpfrac);
	
	vec3 target=camera-slow_orient.z; /* camera looks down *negative* Z */
	gluLookAt(VEC3_TO_XYZ(camera), 
		VEC3_TO_XYZ(target), 
		VEC3_TO_XYZ(slow_orient.y)
	);
}

int mouse_x=0,mouse_y=0;
void camera_mouse(int button,int state,int x,int y) 
{ /* mouse being pressed or released--save position for motion */
	mouse_x=x; mouse_y=y;
	if (state==GLUT_DOWN) {
		mouse_down_count++;
	} else {
		mouse_down_count--;
		if (mouse_down_count<0) mouse_down_count=0;
	}
	glutPostRedisplay(); // start redisplays immediately
	
}
void camera_motion(int x, int y) { 
	float scale=0.005; /* radians rotation/frame per mouse pixel motion */
	camera_orient.nudge(
		-scale*(x-mouse_x), 
		scale*(y-mouse_y)
	);
//printf("Mouse motion: %d,%d\n",x-mouse_x,y-mouse_y);
	mouse_x=x; mouse_y=y; /* save old mouse positions */
}
void camera_keyboard(unsigned char key,int x,int y) {
	key_down[key]=true;
	key_down_count++;
	if (key_down_count==1) key_first_down=true;
//printf("Key down: %d  0x%x   count=%d\n",key,key,key_down_count);
	glutPostRedisplay();
}
void camera_keyboardUp(unsigned char key,int x,int y) {
	key_down[key]=false;
	key_down_count--;
	if (key_down_count<0) key_down_count=0;
//printf("Key up: count=%d\n",key_down_count);
}

void oglCameraInit(void)
{
	glutMouseFunc(camera_mouse);
	glutMotionFunc(camera_motion); 
	glutKeyboardFunc(camera_keyboard);
	glutKeyboardUpFunc(camera_keyboardUp);
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
}

void oglCameraAxes(void)
{ /* Draw 3D axes on the ground */
	for (int axis=0;axis<3;axis++) {
		vec3 color(0.0);
		color[axis]=1.0;
		glColor3fv(color);
		
		glBegin(GL_LINES);
		glVertex3f(0,0,0);
		vec3 end(0.0); end[axis]=1.0;
		glVertex3fv(end);
		glEnd();
		
		glRasterPos3fv(end);
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,'X'+axis);
	}
}

