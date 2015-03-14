/**
 Keep track of events like keydowns, joystick buttons, etc.
 
 Orion Sky Lawlor, olawlor@acm.org, 2006/06/21 (Public Domain)
*/
#ifndef __OGL_EVENT_H
#define __OGL_EVENT_H

/**
 Return true if this keyboard key is currently enabled.
 "what" should be a human-readable description of what
 happens when the key gets pressed.
 Pressing the key toggles between enabled and disabled.
*/
bool oglKey(char c,const char *what);

/**
 Return true if this keyboard key is currently down.
 Again, "what" human-readable.
*/
bool oglKeyDown(char c,const char *what);

/**
 Return true if this 1-based joystick/controller button is pressed.
 "what" is a human-readable description of what will happen.
 
 For example, button 1 is usually labelled '1'.
*/
bool oglButton(int butNo,const char *what);

/**
 Return the value of this 1-based joystick axis.
 Axes are normalized to return values from -1 to +1, with
 the rest position at 0.0.
 
 For example, axis 1 is usually left X, axis 2 left Y,
 axis 3 right Y, axis 4 right X (on new kernels).  
 Y is usually positive down by default.
*/
double oglAxis(int axisNo,const char *what);


/** 
  DEPRECATED--use "oglKey" for new code.
  Stores user-controllable keyboard "toggles" for each character.
  E.g., oglToggles['x'] starts at 0, alternates between 1 and 0 at
     each press of the 'x' key.
*/
typedef int toggle_t[256];
extern toggle_t oglToggles;
extern toggle_t oglKeyMap;

/**
  Set this value to 1 to request a repaint call from the idle routine.
  Note that this variable is safe to manipulate from a thread or signal, 
  unlike glutPostRedisplay or oglRepaint.
  
    oglAsyncRepaintRequest==0 means nothing is requested.
    oglAsyncRepaintRequest==+1 means a one-time repaint is requested.
    oglAsyncRepaintRequest==-1 means repeated repaints are requested.
*/
extern volatile int oglAsyncRepaintRequest;

#endif
