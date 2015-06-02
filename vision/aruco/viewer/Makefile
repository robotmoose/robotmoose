OPTS=-g -O3
ARUCO=/usr/local
CFLAGS=$(OPTS) -I$(ARUCO)/include/aruco -I../../../include
LOPENCV=-lopencv_core -lopencv_calib3d -lopencv_highgui -lopencv_features2d -lopencv_imgproc -lopencv_video -lpthread -lm
LFLAGS=$(OPTS) -L$(ARUCO)/lib -laruco -Wl,-rpath,$(ARUCO)/lib $(LOPENCV)


all: camera

camera: main.cpp
	g++ $< -o $@ $(CFLAGS) $(LFLAGS)

clean:
	- rm camera

