# Standalone makefile for C++ OpenGL program

ifeq ("$(shell uname -s)","Darwin")
 # For MacOS X, use this set of libraries:
 SYSLIBS= -framework GLUT -framework OpenGL -lobjc 
else
 # All other UNIX-derived platforms basically just need glut
 # For Linux,    sudo apt-get freeglut3-dev
 SYSLIBS= -L/usr/local/lib -L/usr/X11R6/lib \
	-lglut -lGLU -lGL -lpthread -lm

 # Some older Linux machines need way more libraries:
 #SYSLIBS= -L/usr/local/lib -L/usr/X11R6/lib \
 #	-lglut -lGLU -lGL  -lXxf86vm  -lXmu -lX11 -lXext -lXi -ldl -lpthread -lm

endif

# Compiler and flags
CCC=g++
CC=gcc
OPTS=-O -ffast-math
CFLAGS=-I. -I../show_IMU $(OPTS)

# Program pieces
DEST=main
OBJS=main.o \
	../show_IMU/ogl/glew.o

all: $(DEST)

# Build main from object files
$(DEST): $(OBJS)
	$(CCC) $(CFLAGS) $(OBJS) $(SYSLIBS) -o $(DEST)

clean:
	-rm $(OBJS) $(DEST)

# Trick gmake into compiling .cpp into .o
o=o
OUTFLAG=-o
%.$o: %.cpp 
	$(CCC) $(CFLAGS) -c $< $(OUTFLAG)$@

%.$o: %.C
	$(CCC) $(CFLAGS) -c $< $(OUTFLAG)$@

%.$o: %.c
	$(CC) $(CFLAGS) -c $< $(OUTFLAG)$@

# Trick other makes into compiling .cpp's into .o's.
.SUFFIXES: .cpp .C .c

.cpp.$o:
	$(CCC) $(CFLAGS) -c $< $(OUTFLAG)$@

.C.$o:
	$(CCC) $(CFLAGS) -c $< $(OUTFLAG)$@

.c.$o:
	$(CC) $(CFLAGS) -c $< $(OUTFLAG)$@
