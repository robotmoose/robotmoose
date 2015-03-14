/**
  Simple GLSL shader interface.
  
  Relies on GLEW internally to access OpenGL extensions.
  
  Dr. Orion Sky Lawlor, olawlor@acm.org, 2008-02-14 (public domain)
*/
#ifndef __OSL_GLSL_H
#define __OSL_GLSL_H

#include <string>

// Create a complete shader object from these chunks of GLSL shader code.
//  You still need to glUseProgramObjectARB(return value);
GLhandleARB makeProgramObject(const char *vertex,const char *fragment);

// This macro converts unquoted GLSL code into a quoted C++ "const char *" string:
#define STRINGIFY_GLSL(code) #code

// This macro allows you to hardcode your shader *without* the annoying string quotes!
#define MAKE_PROGRAM_OBJECT(vertexcode,fragmentcode) \
	makeProgramObject(#vertexcode,#fragmentcode)

// Create a complete shader object from these GLSL files.
GLhandleARB makeProgramObjectFromFiles(const char *vFile="vertex.txt",
	const char *fFile="fragment.txt");

// Handy function: read an entire file into a C++ string.
std::string readFileIntoString(const char *fName);

// Set this uniform float to this value
inline void setUniform(GLhandleARB prog,const char *name,float value) {
	glUniform1fARB(glGetUniformLocationARB(prog,name),value);
}
inline void setUniform(GLhandleARB prog,const char *name,double value) {
	setUniform(prog,name,(float)value);
}
// Set this uniform int to this value
inline void setUniform(GLhandleARB prog,const char *name,int value) {
	glUniform1iARB(glGetUniformLocationARB(prog,name),value);
}
#ifdef __OSL_VEC4_H
inline void setUniform(GLhandleARB prog,const char *name,const vec3 &value) {
	glUniform3fvARB(glGetUniformLocationARB(prog,name),1,value);
}
inline void setUniform(GLhandleARB prog,const char *name,const vec4 &value) {
	glUniform4fvARB(glGetUniformLocationARB(prog,name),1,value);
}
#endif

/* These faster versions of the uniform-setting functions take a string name,
  but cache the string's looked-up "UniformLocation" index in a static variable. */
#define glFastUniform(prog,name,fn,args) \
	do { static int __ul=glGetUniformLocationARB(prog,name); \
	     glUniform##fn##ARB args; } while (0)
#define glFastUniform1i(prog,name,val) glFastUniform(prog,name,1i,(__ul,val))
#define glFastUniform1f(prog,name,val) glFastUniform(prog,name,1f,(__ul,val))
#define glFastUniform1fv(prog,name,cnt,val) glFastUniform(prog,name,1fv,(__ul,cnt,val))
#define glFastUniform2fv(prog,name,cnt,val) glFastUniform(prog,name,2fv,(__ul,cnt,val))
#define glFastUniform3fv(prog,name,cnt,val) glFastUniform(prog,name,3fv,(__ul,cnt,val))
#define glFastUniform4fv(prog,name,cnt,val) glFastUniform(prog,name,4fv,(__ul,cnt,val))
#define glFastUniformMatrix3fv(prog,name,cnt,trs,val) glFastUniform(prog,name,Matrix3fv,(__ul,cnt,trs,val))
#define glFastUniformMatrix4fv(prog,name,cnt,trs,val) glFastUniform(prog,name,Matrix4fv,(__ul,cnt,trs,val))


#endif
