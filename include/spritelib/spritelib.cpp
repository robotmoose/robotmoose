/**
  This is the main implementation file for spritelib.
  
  Typically, you just #include this in your main source file.
  If you're using more than one source file, you'll need to 
  add it to your project.
  
  Dr. Orion Lawlor, lawlor@alaska.edu, 2011-04-05 (Public Domain)
*/
#include "spritelib.h"

/** SOIL, used to read texture images in spritelib::read_tex **/
#include "../soil/SOIL.h" /* Simple OpenGL Image Library, www.lonesock.net/soil.html (plus Dr. Lawlor's modifications) */
#include "../soil/SOIL.c" /* just slap in implementation files here, for easier linking */
#include "../soil/stb_image_aug.c"

#include <iostream>
#include <cmath>
using std::cos; using std::sin;


/* Return a 2D location, equal to (a,b) rotated by this angle around (x,y) */
const float *rotate_2D(float angle,float x,float y,float a,float b)
{
	float d2r=(float)(3.14159265358979/180.0);
	float c=cos(angle*d2r), s=sin(angle*d2r);
	static float ret[2]; /* <- MULTI-threaded HORROR!  (Of course, so's OpenGL) */
	ret[0]=x+a*c+b*s;
	ret[1]=y-a*s+b*c;
	return ret;
}

/* Draw a square sprite with this texture, 
   centered at pixel (x,y), 
   of size w by h pixels,
   rotated counterclockwise by angle "angle" (in degrees)
*/
void spritelib::draw(spritelib_tex tex, 
	float x,float y, float w,float h,
	float angle,bool glow) const
{
	if (glow) glBlendFunc(GL_ONE,GL_ONE); /* just add everything together */
	else glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); /* erase previous stuff */
	glBindTexture(GL_TEXTURE_2D,tex);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLE_FAN);
	w*=0.5f, h*=0.5f; // half width and height
	glTexCoord2f(0.0f,1.0f); glVertex2fv(rotate_2D(angle,x,y,-w,-h));
	glTexCoord2f(1.0f,1.0f); glVertex2fv(rotate_2D(angle,x,y,+w,-h));
	glTexCoord2f(1.0f,0.0f); glVertex2fv(rotate_2D(angle,x,y,+w,+h));
	glTexCoord2f(0.0f,0.0f); glVertex2fv(rotate_2D(angle,x,y,-w,+h));
	glEnd();
}

spritelib_tex spritelib::read_tex(const char *filename) const
{
	return SOIL_load_OGL_texture
        (
                filename,
                SOIL_LOAD_AUTO,
                SOIL_CREATE_NEW_ID,
                SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y,
                GL_RGBA8
        );
}


/* Draw this string, at this X,Y, in this RGBA color */
void spritelib::text(const std::string &s,float x,float y,const float *rgba_color) const
{
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glColor4fv(rgba_color);
	glRasterPos2f(x,y);
	for (unsigned int i=0;i<s.size();i++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,s[i]);
	glColor4f(1,1,1,1);
}


/* Return the GUI's current spritelib object */
spritelib &spritelib_get(bool check_modifiers) {
	static spritelib lib;
	if (check_modifiers) {
		int mod=glutGetModifiers();
		lib.shift=(mod&GLUT_ACTIVE_SHIFT)!=0;
		lib.ctrl =(mod&GLUT_ACTIVE_CTRL)!=0;
		lib.alt  =(mod&GLUT_ACTIVE_ALT)!=0;
	}
	return lib;
}

/* GLUT display function */
void spritelib_display(void) 
{
	spritelib &lib=spritelib_get(false);
	glDisable(GL_DEPTH_TEST);
	// Dunno if alpha is actually a good idea or not...
	glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA); // premultiplied alpha
	glEnable(GL_BLEND);
	
	glClearColor(lib.background[0],lib.background[1],
		lib.background[2],lib.background[3]); // background color
	glClear(GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity(); // flush any ancient matrices
	
	lib.screenw=(float)glutGet(GLUT_WINDOW_WIDTH);
	lib.screenh=(float)glutGet(GLUT_WINDOW_HEIGHT);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); // flush any ancient matrices
	glTranslatef(-1.0f,+1.0f,0.0f);
	glScalef(2.0f/lib.screenw,-2.0f/lib.screenh,0.0f);
	static float oldtime=0.0;
	float newtime=glutGet(GLUT_ELAPSED_TIME)*0.001f;
	lib.dt=newtime-oldtime; if (lib.dt>1.0) lib.dt=1.0;
	lib.time=newtime;
	oldtime=newtime;
	
	glBindTexture(GL_TEXTURE_2D,0); /* turn off texture */
	glColor3f(1.0,1.0,1.0); /* white */
	
	/* Call user's drawing function */
	spritelib_draw_screen(lib);
	
	/* Clear all the oneshots */
	for (int key=0;key<256;key++) lib.key_typed[key]=false;
	lib.mouseleft_clicked=false;
	lib.mouseright_clicked=false;
	
	glutPostRedisplay(); // continual animation
	glutSwapBuffers();
}



void spritelib_mouse(int button,int state,int x,int y) 
{ /* mouse being pressed or released--save position for motion */
	spritelib &lib=spritelib_get();
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
	glutPostRedisplay(); // start redisplays immediately
	
}
void spritelib_motion(int x, int y) { 
	spritelib &lib=spritelib_get(false);
	lib.mousex=x; lib.mousey=y;
	glutPostRedisplay(); // start redisplays immediately
}
void spritelib_keyboard(unsigned char key,int /*x*/,int /*y*/) {
	spritelib &lib=spritelib_get();
	if (!lib.key[key]) lib.key_typed[key]=true; /* first time! */
	lib.key[key]=true;
	glutPostRedisplay();
}
void spritelib_keyboardUp(unsigned char key,int /*x*/,int /*y*/) {
	spritelib &lib=spritelib_get();
	lib.key[key]=false;
	glutPostRedisplay();
}

/** You must call this function once, from your main function.
  Spritelib will open a window with the given title and size,
  and then handle events to that window until you exit. 
  This function never returns. */
void spritelib_run(const char *window_title,int screenw,int screenh)
{
	int argc=1; char *argv[2]; argv[0]=(char *)"main"; argv[1]=0; // fix old GLUT!
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGBA + GLUT_DEPTH + GLUT_DOUBLE);
	glutInitWindowSize(screenw,screenh);
	glutCreateWindow(window_title);
	
	glutDisplayFunc(spritelib_display);
	glutMouseFunc(spritelib_mouse);
	glutMotionFunc(spritelib_motion); 
	glutPassiveMotionFunc(spritelib_motion); 
	glutKeyboardFunc(spritelib_keyboard);
	glutKeyboardUpFunc(spritelib_keyboardUp);
	
	glutMainLoop();
}
