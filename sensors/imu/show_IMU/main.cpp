/**
  Physics simulator demo program.
  
  Dr. Orion Sky Lawlor, olawlor@acm.org, 2011-01-20 (Public Domain)
*/
#include "physics/world.h"
#include "ogl/glsl.h"
#include "soil/SOIL.h"

/* LAME! just include library bodies here, for easy linking */
#include "physics/world.cpp" 
#include "physics/config.cpp"
#include "ogl/glsl.cpp"
#include "soil/SOIL.c"
#include "soil/stb_image_aug.c"

const float far_bumper=108.0; // units to far red bumper

#include "osl/serial.h"
#include "osl/serial.cpp"
#include <sstream>

class robot_IMU : public physics::object {
public:
	std::string serial_line;
	vec3 accel_raw,accel_avg,accel; // last accelerometer reading, mean, scaled
	vec3 accel_del; // long-term average acceleration
	
	vec3 gyro_raw,gyro_raw_avg,gyro; // last gyro reading
	vec3 compass_raw;
	
	vec3 origin; // center of IMU
	vec3 vel; // velocity of IMU, m/sec
	ortho_frame frame; // local coordinate axes (orientation of IMU) in world space

	robot_IMU() :physics::object(0.001), origin(0.0,-1.0,0.5), vel(0.0)
	{
		accel_raw=accel_avg=accel=vec3(0.0);
		accel_del=vec3(0.0,0.0,-9.8);
		gyro_raw=gyro_raw_avg=gyro=vec3(0.0);
		
		frame.x=vec3(1,0,0);
		frame.y=vec3(0,1,0);
		frame.z=vec3(0,0,1);
		Serial.begin(57600); // ASCII reports arrive here
		serial_line="";
	}
	
	// Project this local vector into global coordinates
	vec3 world_from_local(const vec3 &v) {
		return v.x*frame.x+v.y*frame.y+v.z*frame.z;
	}
	// Project this global vector into local coordinates
	vec3 local_from_world(const vec3 &v) {
		return vec3(dot(v,frame.x),dot(v,frame.y),dot(v,frame.z));
	}

	void simulate(physics::library &lib) {
		float dt=timestep.dt;
	
	// Update 3D acceleration
		accel=world_from_local((accel_raw)*(9.8/265.0)); // units: meter/sec^2
		accel_avg=accel_avg+2.0*dt*(accel-accel_avg); // fast accumulate
		accel_del=accel_del+0.1*dt*(accel_avg-accel_del); // slow accumulate
	
	// Update position (lurchy and drunken!)
		vel+=dt*-accel; // integrate velocity from acceleration
		vel.z=0.0;
		vel*=(1.0-0.5*timestep.dt); // scale back, to avoid drifting
		origin+=dt*vel; // integrate position from velocity
	
		origin.x*=0.999; // scale back to reduce drifting
		origin.y=(origin.y+1.0)*0.999-1.0;
		
	// Update gyro
		vec3 gyro_del=gyro_raw-gyro_raw_avg; // rotation rate estimate
		if (length(gyro_del)<200) { // not spinning fast
			gyro_raw_avg=gyro_raw_avg+dt*gyro_del; // slow accumulate
		}
		/* Units:
			from the datasheet, "full scale is 2000 degrees/sec"
		*/
		gyro=gyro_del*((2000.0/(1<<18))/(57.6)); // units: radians/sec
		
		static int count=0;
		if ((++count)%100==0) {
			std::cout<<"	A	"<<accel.x<<"	"<<accel.y<<"	"<<accel.z;
			std::cout<<"	G	"<<gyro.x<<"	"<<gyro.y<<"	"<<gyro.z;
			std::cout<<std::endl;
		}
	
	
	// Rotate coordinate frame by read-in rotation rate
		vec3 rate=gyro;
		if ((length(accel)-9.8)<0.25) 
		{ // mostly pure gravity (not much body motion):
		// this is our chance to fix gyro drift!
			double fixrate=0.2; // radians/sec of fix, per m/s^2 of error
			vec3 world_accel=accel;
			world_accel.z+=9.8; // gravity should face down: if not, fix it!
			
			//  ASSUMES local z axis is basically up (else need full rotation calculation, possibly a cross product or something)
			rate.x+=fixrate*(dot(world_accel,frame.y)); 
			rate.y+=fixrate*(dot(world_accel,frame.x));
			
			// Seems something like this should work, but it doesn't...
			//rate+=local_from_world(cross(world_accel,vec3(0,0,fixrate)));
		}
		rate*=dt;
		
		frame.x-=rate.z*frame.y;
		frame.y+=rate.z*frame.x;
		frame.nudge(-rate.y,-rate.x);
	
	
	// Update serial data
		while (Serial.available()) {
			char c=Serial.read();
			if (c=='\n') { // end of line
				process_serial(serial_line);
				serial_line="";
			}
			else serial_line+=c;
		}
	}
	
	// Read X,Y,Z
	vec3 read_xyz(std::stringstream &ss) {
		double x,y,z;
		ss>>x>>y>>z;
		return vec3(x,y,z);
	}
	
	// Read line of ASCII text, which originated from the serial port
	void process_serial(const std::string &line) {
		std::stringstream ss(line);
		double time; ss>>time;
		std::string format; ss>>format;
		if (format!="A") std::cerr<<"Unrecognized line format '"<<line<<"'\n";
		else {
			accel_raw=read_xyz(ss);
			accel_raw=vec3(+accel_raw.x,-accel_raw.y,-accel_raw.z); // match gyro coords
			accel_raw+=vec3(10,0,0); // fix bias (sadly, bias is time-dependent)
			
			ss>>format;
			if (format!="G") std::cerr<<"Unrecognized line format at G (gyro)\n";
			else {
				gyro_raw=read_xyz(ss);
				gyro_raw=vec3(-gyro_raw.x,+gyro_raw.y,-gyro_raw.z);
				ss>>format;
				if (format!="C") std::cerr<<"Unrecognized line format at C (compass)\n";
				else compass_raw=read_xyz(ss);
			}
		}
	}

	// Draw local 3D coordinate system onscreen
	void draw(physics::library &lib) {
		glLineWidth(12.5);
		
		glBegin(GL_LINES);
		for (int axis=0;axis<3;axis++) {
			vec3 dir=((vec3 *)&frame)[axis];
			dir+=origin;
			
			vec3 color(0.0); color[axis]=1.0;
			
			glColor3fv(color);
			glVertex3fv(origin);
			glColor3fv(color);
			glVertex3fv(dir);
			
			// Shadow:
			glColor4f(0.0,0.0,0.0,0.4);
			glVertex2fv(origin);
			glVertex2fv(dir);
		}
		glColor3f(1.0,0.0,1.0);
		glVertex3fv(origin);
		vec3 down=accel_avg;
		down*=0.03;
		down+=origin;
		glVertex3fv(down);
		
		// Shadow:
		glColor4f(0.0,0.0,0.0,0.4);
		glVertex2fv(origin);
		glVertex2fv(down);
		
		glEnd();
        }

};


vec3 make2d(vec3 src) {
	src.z=0.0;
	return src;
}


/* Called create a new simulation */
void physics_setup(physics::library &lib) {
	if (!lib.key_toggle['f']) {
		//camera=vec3(0.0,-5.0,1.7);
		camera_orient.x=vec3(1,0,0);
		camera_orient.z=normalize(vec3(0,-0.6,0.5));
	}
	lib.world->add(new robot_IMU());
	lib.background[0]=lib.background[1]=0.6;
	lib.background[2]=1.0; // light blue
}

/* Called every frame to draw everything */
void physics_draw_frame(physics::library &lib) {
	/* use SOIL ground texture */
	static GLuint texture=SOIL_load_OGL_texture(
		"ground.jpg",0,0,SOIL_FLAG_MIPMAPS|SOIL_FLAG_TEXTURE_REPEATS
	);
	glBindTexture(GL_TEXTURE_2D,texture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_ANISOTROPY_EXT,4);
	
	/* Draw opaque white ground plane */
	int groundsize=500;
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glColor3f(1,1,1);
	glPushMatrix();glTranslatef(0,0,-0.01); /* <- shove down, to reduce Z fighting */
	glBegin(GL_QUADS);
	glTexCoord2i(-groundsize,-groundsize); glVertex2i(-groundsize,-groundsize);
	glTexCoord2i(+groundsize,-groundsize); glVertex2i(+groundsize,-groundsize);
	glTexCoord2i(+groundsize,+groundsize); glVertex2i(+groundsize,+groundsize);
	glTexCoord2i(-groundsize,+groundsize); glVertex2i(-groundsize,+groundsize);
	glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);

	/* Point and line smoothing needs normal alpha blending */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
	/* Now draw objects */
	lib.world->draw(lib);
}


