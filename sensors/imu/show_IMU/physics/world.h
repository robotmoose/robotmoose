/**
  OpenGL physics-based world library.  This is the main library header.
  
  License: anyone may use, modify and distribute this code for any purpose.
  Dr. Orion Sky Lawlor, olawlor@acm.org, 2010-12-16 (Public Domain)
*/
#ifndef __OSL_PHYSICS_WORLD_H
#define __OSL_PHYSICS_WORLD_H

#include "GL/glew.h" /* GL extension wrangler: http://glew.sourceforge.net/ */
#include "GL/glut.h" /* windows, mice, etc.  http://freeglut.sourceforge.net */
#include "osl/vec4.h" /* 3D vector class */
#include <vector> /* std::vector container */

#ifndef M_PI
#define M_PI 3.14159265358979 /* why doesn't Microsoft already have this? */
#endif


namespace physics {
	class scene; // forward declaration

	/* Everything this library provides is in here: 
	   time handling, keystrokes, mouse location,
	   texture and framebuffer utility functions, 
	   config parameters and config file, etc. 
	*/
	class /* physics:: */ library {
	public:
		physics::scene *world; // All the objects in the world
	
	
		bool hasVBO; /* if true, this machine has Vertex Buffer Object support */
		bool hasShaders; /* if true, can compile and run GLSL OK */
		
		double time; // current simulated time, in seconds (careful: slowmo & pause!)
		float dt; // time between frames, in seconds (for framerate independence)
		
		
		/* Current drawing mode:
			draw_shadows, // preparing shadow map pixels
			draw_background, // sky, terrain, etc
			draw_main, // central objects of the scene
			draw_ontop, // alpha or distortion effects (after main scene)
		
		Current geometry LOD (for framerate feedback)
		
		 */
		
	/* Info about the current window ("screen") */
		float background[4]; /* RGBA background color (read/write) */
		float screenw,screenh; /* size of our window, in pixels (read only) */
		float fov; /* field of view, in degrees */
		float cameraVelocity; /* motion rate of camera, world units per second */

	/* Keyboard */
		bool key[256]; /* if true, the corresponding key is currently pressed */
		bool key_typed[256]; /* oneshot: the corresponding key was *just* typed */
		bool key_toggle[256]; /* toggle: initially off, type the key to turn on */
		bool shift,ctrl,alt; /* this modifier key is currently pressed (note: only updated when another key or mouse is pressed!) */
	
	/* Mouse */
		int mousex,mousey; /* location of the mouse */
		bool mouseleft; /* the left mouse button is currently pressed */
		bool mouseleft_clicked; /* oneshot: the left mouse button was *just* clicked */
		bool mouseright; /* ... ditto for right button... */
		bool mouseright_clicked;
		int mousewheel; /* current position of scroll wheel on mouse */

	
	};
	
	/* This little class keeps track of your timestep size.
	  It's only updated by the library itself, not by you.  */
	class /* physics::*/ stepsize {
	public:
		double dt; // timestep: seconds between simulate calls
		double last_t; // time of the last simulate call
		
		stepsize(double dt_,double last_t_=0.0) :dt(dt_), last_t(last_t_) {}
	};
	
	/* Describes properties of a physics::object. */
	typedef enum {
		flag_paused=1<<10, /* disable simulate calls */
		flag_bounded=1<<11, /* object has a bounding box (for clipping) */
		flag_last
	} flags;
	
	/* One drawable object that is running a physics simulation. 
	   You should probably inherit your classes from this type. */
	class /* physics:: */ object {
	public:
		/* Draw yourself onscreen.  This will be called at each frame.
		  The OpenGL projection and modelview are already set up;
		  you need to enable, disable, or bind any other features you need. */
		virtual void draw(physics::library &lib) =0;
		
		/* Take one physics simulation step. 
		   This function will be called once every timestep. 
		   By default, does nothing. */
		virtual void simulate(physics::library &lib);
		
		/* This set of physics::flags describes our object.
		   Write it once, and the library will read it out. */
		unsigned long flags;
		
		/* This object describes how big your physics timesteps are.
		   This will be updated by the library, not you! */
		physics::stepsize timestep;
		
		// constructor and destructor
		object(float timestep_,unsigned long flags_=0)
			 :flags(flags_), timestep(timestep_) {}
		virtual ~object();
	};
	
	/* The entire scene: a list of physics objects. */
	class /* physics:: */ scene : public physics::object {
	public:
		// These are all the objects in the scene.
		//  You can add objects at any time.
		std::vector<physics::object *> objects;
		
		// Call this function to add an object to our list.
		//   Eventually, we will delete the object, so allocate with new.
		virtual void add(physics::object *newobj);
		
		// Remove this object from our list.  Deleting the object
		//  is then your responsibility.
		virtual void remove(physics::object *oldobj);
	
		// To draw the scene, just draw each object.
		virtual void draw(physics::library &lib);
		
		// This loops over all the objects, and runs their physics.
		virtual void simulate(physics::library &lib);
		
		scene() :physics::object(0.01,0) {}
		virtual ~scene(); // deletes all the objects
	};
};


// YOU define this function, which is called once at startup.
//   typically this just adds some geometry to lib.world.
extern void physics_setup(physics::library &lib);

// YOU define this function, which is called every frame to draw the world.
//   typically this just calls lib.world->draw(lib);
extern void physics_draw_frame(physics::library &lib);


#endif // def(thisHeader)
