/**
  OpenGL physics-based scene library.  
  This is the main library implementation file.
  
  License: anyone may use, modify and distribute this code for any purpose.
  Dr. Orion Sky Lawlor, olawlor@acm.org, 2010-12-16 (Public Domain)
*/
#include "world.h" /* "physics/world.h" */
#include "config.h" /* "physics/config.h" */
#include <iostream>
#include <algorithm> /* for std::find */
#include <cstring> /* for std::memset */
#include "ogl/minicam.h" /* simple camera class */

/************* physics::object ************/
void physics::object::simulate(physics::library &lib) {
	/* no physics to do yet; user will hopefully add some */
}
physics::object::~object() {}


/************** physics::scene *************/
/* The entire scene: a list of physics objects. */


void physics::scene::add(physics::object *newobj)
{
	objects.push_back(newobj);
}
void physics::scene::remove(physics::object *oldobj)
{
	objects.erase(std::find(objects.begin(),objects.end(),oldobj));
}

// To draw the scene, just draw each object.
void physics::scene::draw(physics::library &lib) 
{
	for (unsigned int i=0;i<objects.size();i++) {
		objects[i]->draw(lib);
	}
}
		
// This loops over all the objects, and runs their physics.
void physics::scene::simulate(physics::library &lib)
{
	for (unsigned int i=0;i<objects.size();i++) {
		physics::stepsize &t=objects[i]->timestep;
		while (t.last_t+t.dt<lib.time) 
		{ /* This object needs to take another simulated timestep */
			t.last_t+=t.dt;
			objects[i]->simulate(lib);
		}
	}
}
		
physics::scene::~scene() { // deletes all the objects
	for (unsigned int i=0;i<objects.size();i++) {
		delete objects[i];
	}
}

/************* library and UI support **************
  Most of this is straight out of my 2D "spritelib" library.
*/
physics::library &physics_get(bool create_if_needed=false) {
	static physics::library *l=0;
	if (create_if_needed && l==0) {
		l=new physics::library;
		std::memset(l,0,sizeof(physics::library)); /* zero *everything* in lib! */
		l->world=0;
		l->fov=70.0;
		l->cameraVelocity=2.0;
		l->background[0]=l->background[1]=l->background[2]=l->background[3]=
			0.3;
	}
	return *l;
}


void physics_display(void) 
{
	physics::library &lib=physics_get();
	float newtime=glutGet(GLUT_ELAPSED_TIME)*0.001f;
	
	/* Artificial scale factor for slow CPUs: fewer simulation calls. */
	static float slow_computer=1.0;
	
	static float oldtime=newtime; /* time of previous frame */
	float velocityScale=1.0;
	
	lib.dt=newtime-oldtime; if (lib.dt>1.0) lib.dt=1.0;
	oldtime=newtime;

	if (lib.key_typed['r']) { /* reset simulation */
		while (lib.world->objects.size()>0)
			lib.world->remove(lib.world->objects[0]);
		lib.time=0;
		physics_setup(lib);
	}
	if (lib.key_toggle[' ']) { /* pause simulation */
		lib.dt=0.0;
	}
	if (lib.key['b']) { /* bullet time! */
		lib.dt*=0.1;
		velocityScale=0.5; /* move camera slower too */
	}
	lib.dt*=slow_computer;
	lib.time+=lib.dt;


	static bool first_time=true;
	if (first_time) {physics_setup(lib); first_time=false;}

	glDisable(GL_DEPTH_TEST);
	// Dunno if alpha is actually a good idea or not...
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); // non-premultiplied alpha
	glEnable(GL_BLEND);
	
	glClearColor(lib.background[0],lib.background[1],
		lib.background[2],lib.background[3]); // background color
	glClear(GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity(); // flush any ancient matrices
	lib.screenw=(float)glutGet(GLUT_WINDOW_WIDTH);
	lib.screenh=(float)glutGet(GLUT_WINDOW_HEIGHT);
	gluPerspective(lib.fov, // fov
		 lib.screenw/lib.screenh,
		 0.1,
	        1000.0); // z clipping planes
	oglCameraLookAt(lib.cameraVelocity*velocityScale);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); // flush any ancient matrices
	
	glBindTexture(GL_TEXTURE_2D,0); /* turn off texture */
	glColor3f(1.0,1.0,1.0); /* white */
	
	/* Run user's simulations */
	lib.world->simulate(lib);
	
	if (glutGet(GLUT_ELAPSED_TIME)*0.001f-newtime>0.5) 
	{ /* ouch--that took half a second: we're falling behind. */
		slow_computer*=0.5;
		std::cout<<"Now running at "<<slow_computer<<" speed (for your slow CPU)\n";
	}
	
	
	/* Run user's draw function */
	physics_draw_frame(lib);

	/* Clear all the oneshots */
	for (int key=0;key<256;key++) lib.key_typed[key]=false;
	lib.mouseleft_clicked=false;
	lib.mouseright_clicked=false;
	
	glutPostRedisplay(); // continual animation
	glutSwapBuffers();
}
void physics_mouse(int button,int state,int x,int y) 
{ /* mouse being pressed or released--save position for motion */
	physics::library &lib=physics_get();
	if (state==GLUT_DOWN) {
		if (button==0) {lib.mouseleft=true; lib.mouseleft_clicked=true;}
		else if (button==2) {lib.mouseright=true; lib.mouseright_clicked=true;}
		else if (button==3) {lib.mousewheel++;}
		else if (button==4) {lib.mousewheel--;}
	} else {
		if (button==0) {lib.mouseleft=false;}
		else if (button==2) {lib.mouseright=false;}
	}
	lib.mousex=x; lib.mousey=y;
	camera_mouse(button,state,x,y);
	glutPostRedisplay(); // start redisplays immediately
	
}
void physics_pmotion(int x, int y) {  /* passive motion (button up) */
	physics::library &lib=physics_get(false);
	lib.mousex=x; lib.mousey=y;
	glutPostRedisplay(); // start redisplays immediately
}
void physics_motion(int x, int y) {  /* motion (button down) */
	camera_motion(x,y);
	physics_pmotion(x,y);
}
void physics_keyboard(unsigned char key,int x,int y) {
	physics::library &lib=physics_get();
	if (!lib.key[key]) {
		lib.key_typed[key]=true; /* first time! */
		lib.key_toggle[key]=!lib.key_toggle[key];
	}
	lib.key[key]=true;
	camera_keyboard(key,x,y);
	glutPostRedisplay();
}
void physics_keyboardUp(unsigned char key,int x,int y) {
	physics::library &lib=physics_get();
	lib.key[key]=false;
	camera_keyboardUp(key,x,y);
	glutPostRedisplay();
}

int main(int argc,char *argv[])
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGBA + GLUT_DEPTH + GLUT_DOUBLE);
	
	physics_cfg::read(); /* read config.ini */
	
	oglCameraInit();
	camera=vec3(0,-3,+1.7);
	
	physics::library &lib=physics_get(true);
	lib.background[0]=lib.background[1]=lib.background[2]=1.0;
	lib.background[3]=0.0;
	lib.screenw=800; lib.screenh=600;
	lib.world=new physics::scene();
	const char *window_title="Physics Simulation";
	
	glutInitWindowSize(lib.screenw,lib.screenh);
	glutCreateWindow(window_title);
	
	glewInit(); /* must be *after* window creation! */
	/* ARB_shader_objects */
	if (glUseProgramObjectARB!=0) {lib.hasShaders=true;}
	/* ARB_vertex_buffer_object */
	if (glBindBufferARB!=0) {lib.hasVBO=true;}
	
	glutDisplayFunc(physics_display);
	glutMouseFunc(physics_mouse);
	glutMotionFunc(physics_motion); 
	glutPassiveMotionFunc(physics_pmotion); 
	glutKeyboardFunc(physics_keyboard);
	glutKeyboardUpFunc(physics_keyboardUp);
	
	glutMainLoop();
}
