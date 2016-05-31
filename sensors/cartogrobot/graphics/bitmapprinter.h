// bitmapprinter.h
// Glenn G. Chappell
// 11 Sep 2013
//
// For CS 381 Fall 2013
// Header for class BitmapPrinter
// Simple bitmap text rendering
// There is no associated source file
// Requires GLUT

#ifndef FILE_BITMAPPRINTER_H_INCLUDED
#define FILE_BITMAPPRINTER_H_INCLUDED

// OpenGL/GLUT includes - DO THESE FIRST
#include <cstdlib>       // Do this before GL/GLUT includes
using std::exit;
#ifndef __APPLE__
# include <GL/glut.h>    // Includes OpenGL headers as well
#else
# include <GLUT/glut.h>  // Apple puts glut.h in a different place
#endif

// Other includes
#include <string>        // For std::string


// class BitmapPrinter
// For drawing text using GLUT bitmap text facility. Uses 9x15 font.
// Usage:
//     BitmapPrinter p(-0.9, 0.9, 0.1); // Start text x, y, line height
//     p.print("Hello");                // 1st line, start @ (-0.9, 0.9)
//     p.print("there");                // 2nd line, start @ (-0.9, 0.8)
// Note: You probably want model/view to be the identity (or just
//  translations) when using member function print.
class BitmapPrinter {

// ***** BitmapPrinter: public functions *****
public:

	// Ctor (0, 1, 2, or 3 params)
	// Set cursx, cursy, lineht to the given values.
	BitmapPrinter(double theCursx = 0.,
				  double theCursy = 0.,
				  double theLineht = 0.1)
	{ setup(theCursx, theCursy, theLineht); }

	// Compiler-generated copy ctor, copy =, dctor used

	// setup
	// Set cursx, cursy, lineht to the given values.
	void setup(double theCursx,
			   double theCursy,
			   double theLineht = 0.1)
	{ cursx = theCursx; cursy = theCursy; lineht = theLineht; }

	// print
	// Draw the given string, using glutBitmapCharacter, with GLUT's
	// 9x15 font, at cursx, cursy, and then reduce cursy by lineht
	// (i.e., move to the next line).
	//
	// The model/view transformation should probably be the identity
	// (or just translations) when calling this function.
	void print(const std::string & msg)
	{
		glRasterPos2d(cursx, cursy);
		for (std::string::const_iterator ii = msg.begin();
			 ii != msg.end();
			 ++ii)
		{
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *ii);
		}
		cursy -= lineht;
	}

// ***** BitmapPrinter: data members *****
private:

	double cursx, cursy;  // Raster pos for next text line: x, y
	double lineht;        // How much to reduce cursy each line

};  // End class BitmapPrinter


#endif //#ifndef FILE_BITMAPPRINTER_H_INCLUDED

