/**
  Simple code to compile and link GLSL code in an OpenGL program.
  
  Dr. Orion Lawlor, olawlor@acm.org, 2009-01-04 (Public Domain)
  This version includes enhancements by Dr. Glenn Chappell.
*/
#include <GL/glew.h> /*<- for gl...ARB extentions.  You *must* call glewInit after glutCreateWindow! */
#include <iostream>
#include <stdio.h>
#include <cstdlib> /* for "exit" */
#include <fstream>
#include <string>

#include "glsl.h"

// errorExit
// Output given string to cout, followed by newline, then wait for
//  keypress, and do exit(1).
void errorExit(const std::string & msg)
{
    std::cout << msg << std::endl << std::endl;
    std::cout << "Press ENTER to quit ";
    while (std::cin.get() != '\n') ;
    std::exit(1);
}

// checkShaderOp
// Print message and exit if GLSL build error.
void checkShaderOp(GLhandleARB obj,
                   GLenum errType,
                   const std::string & where) 
{
    GLint compiled;
    glGetObjectParameterivARB(obj, errType, &compiled);
    if (!compiled)
    {
        enum { LOGSIZE = 10000 };
        GLcharARB errorLog[LOGSIZE];
        // Our buffer (errorLog) must be fixed-length, because we are using
        //  OpenGL's "C" API. However, OpenGL knows how big the buffer is,
        //  and there is no possibility of overflow.
        GLsizei len = 0;
        glGetInfoLogARB(obj, GLsizei(LOGSIZE), &len, errorLog);
        errorExit("ERROR - Could not build GLSL shader: " + where
                + "\n\nERROR LOG:\n" + errorLog);
    }
}

// Create a vertex or fragment shader from this code.
GLhandleARB makeShaderObject(int target,const char *code)
{
	GLhandleARB h=glCreateShaderObjectARB(target);
	glShaderSourceARB(h,1,&code,NULL);
	glCompileShaderARB(h);
	checkShaderOp(h,GL_OBJECT_COMPILE_STATUS_ARB,code);
	return h;
}
// Create a complete shader object from these chunks of GLSL shader code.
//  You still need to glUseProgramObjectARB(return value);
//  THIS IS THE FUNCTION YOU PROBABLY *DO* WANT TO CALL!!!!  RIGHT HERE!!!!
GLhandleARB makeProgramObject(const char *vertex,const char *fragment)
{
	if (glUseProgramObjectARB==0) 
	{ /* glew never set up, or OpenGL is too old.. */
		std::cout<<"Error!  OpenGL hardware or software too old--no GLSL!\n";
		exit(1);
	}
	GLhandleARB p=glCreateProgramObjectARB();
	GLhandleARB vo=makeShaderObject(GL_VERTEX_SHADER_ARB,vertex);
	GLhandleARB fo=makeShaderObject(GL_FRAGMENT_SHADER_ARB,fragment);
	glAttachObjectARB(p,vo);
	glAttachObjectARB(p,fo);
	glLinkProgramARB(p);
	checkShaderOp(p,GL_OBJECT_LINK_STATUS_ARB,"link");
	glDeleteObjectARB(vo); glDeleteObjectARB(fo); 
	return p;
}
// Read an entire file into a C++ string.
std::string readFileIntoString(const char *fName) {
	char c; std::string ret;
	std::ifstream f(fName);
	if (!f) {ret="Cannot open file ";ret+=fName; return ret;}
	while (f.read(&c,1)) ret+=c;
        return ret;
}
// Read a file and everything it #includes (recursively!) into this string
std::string processIncludes(const char *fName) {
	std::string ret;
	std::ifstream f(fName);
	if (!f) {ret="Cannot open file ";ret+=fName; return ret;}
	while (f) {
		std::string line;
		std::getline(f,line);
		if (line.find("#include")==0) 
		{ // include line
			unsigned int firstquote=line.find('"');
			unsigned int secondquote=line.find('"',firstquote+1);
			if (secondquote==std::string::npos) return "Can't parse include "+line;
			std::string filename=line.substr(firstquote+1,secondquote-firstquote-1);
			std::cout<<"Including file '"<<filename<<"'\n";
			ret+=processIncludes(filename.c_str());
		}
		else
			ret+=line+"\n"; // non-include line
	}
        return ret;
}

// Create a complete shader object from these GLSL files.
GLhandleARB makeProgramObjectFromFiles(const char *vFile,
	const char *fFile)
{
	return makeProgramObject(
		processIncludes(vFile).c_str(),
		processIncludes(fFile).c_str()
	);
}
