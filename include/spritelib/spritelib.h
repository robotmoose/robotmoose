/* A trivally simple 2D sprite library, for
a graphical user interface access.

Orion Sky Lawlor, olawlor@acm.org, 2010-11-11 (Public Domain)
*/
#ifndef __OSL_SPRITELIB_H
#define __OSL_SPRITELIB_H

/* Spritelib is built on top of OpenGL.  So you can call OpenGL, if you want. 
  Unfortunately, you also need GLUT installed; the distro comes with Freeglut.
*/
#if defined(_WIN32)
#include "../GL/glut.h"
#else
#ifdef __APPLE__
# include <OpenGL/gl.h>
# include <GLUT/glut.h>
#else
# include <GL/glut.h> /* OpenGL Utilities Toolkit, for GUI tools */
#endif
#endif
#include <string>

/* This represents a 2D texture image, loaded on the graphics card.
   See spritelib.read_tex for how to get one.
*/
typedef unsigned int spritelib_tex; 

/** This is the main class inside spritelib.  It's packed with handy data,
and has functions to draw stuff on the screen. */
class spritelib {
public:
/* Drawing functions */
	/* Load up a texture image from this filename. 
	   You should probably only call this once, and stash the spritelib_tex
	   in a static variable or class member.
	*/
	spritelib_tex read_tex(const char *filename) const;
	
	/* Draw a square sprite with this texture, 
	   centered at pixel (x,y), 
	   of size w by h pixels,
	   rotated counterclockwise by angle "angle" (in degrees)
	*/
	void draw(spritelib_tex tex, float x,float y, 
		float w,float h,float angle=0.0,bool glow=false) const;
	
	/* Draw this string, at this X,Y, in this RGBA color */
	void text(const std::string &s,float x,float y,const float *rgba_color) const;
	
/* Info about the current window ("screen") */
	float background[4]; /* RGBA background color (read/write) */
	float screenw,screenh; /* size of our window, in pixels (read only) */

/* Time */
	float time; /* time since application started, in seconds */
	float dt; /* time between frames, in seconds (for physics) */

/* Keyboard */
	bool key[256]; /* if true, the corresponding key is currently pressed */
	bool key_typed[256]; /* oneshot: the corresponding key was *just* typed */
	bool shift,ctrl,alt; /* this modifier key is currently pressed (note: only updated when another key or mouse is pressed!) */
	
/* Mouse */
	int mousex,mousey; /* location of the mouse */
	bool mouseleft; /* the left mouse button is currently pressed */
	bool mouseleft_clicked; /* oneshot: the left mouse button was *just* clicked */
	bool mouseright; /* ... ditto for right button... */
	bool mouseright_clicked;
	int mousewheel; /* current position of scroll wheel on mouse */

private:
	spritelib(void) {} /* Don't make your own spritelib objects... */
	friend spritelib &spritelib_get(bool check=true); /*...he will. */
	spritelib(const spritelib &o); /* Don't copy spritelib objects... */
	void operator=(const spritelib &o); /* ...don't assign them either. */
};

/** You must call this function once, from your main function.
  Spritelib will open a window with the given title and size,
  and then handle events to that window until you exit. 
  This function never returns. */
void spritelib_run(const char *window_title,int screenw,int screenh);


/**
  You must write this function.  Spritelib will call it repeatedly
  to draw stuff on the screen, and handle mouse and keyboard events.
*/
void spritelib_draw_screen(spritelib &lib);



#endif
