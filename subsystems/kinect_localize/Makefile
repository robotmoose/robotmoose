CC=gcc
CPP=g++
SRC=src
INC=include
SUPERSTAR_INC=$(INC)/superstar
INCLUDE_DIRS=-I$(INC) -I$(INC)/skeltrack -I/usr/include/glib-2.0 -I /usr/lib/x86_64-linux-gnu/glib-2.0/include
CFLAGS=-Wno-sign-compare $(INCLUDE_DIRS)
CCFLAGS=-std=c++11 -O -Wno-unused-result $(INCLUDE_DIRS) -I$(SRC)
CLIB=-pthread -lgio-2.0 -lgobject-2.0 -lgthread-2.0 -lglib-2.0
LIB=-lfreenect $(CLIB)

JSONCPP_SRC=$(SUPERSTAR_INC)/jsoncpp/json_reader.cpp $(SUPERSTAR_INC)/jsoncpp/json_value.cpp $(SUPERSTAR_INC)/jsoncpp/json_writer.cpp
MONGOOSE_SRC=$(SUPERSTAR_INC)/mongoose/mongoose.c
SKELTRACK_SRC=$(INC)/skeltrack/pqueue.c $(INC)/skeltrack/skeltrack-joint.c $(INC)/skeltrack/skeltrack-skeleton.c $(INC)/skeltrack/skeltrack-smooth.c $(INC)/skeltrack/skeltrack-util.c
UTILITY_SRC=$(SUPERSTAR_INC)/auth.cpp $(SRC)/ini.cpp $(SUPERSTAR_INC)/json_util.cpp src/robot_config.cpp $(SUPERSTAR_INC)/string_util.cpp $(SRC)/string_util.cpp $(SUPERSTAR_INC)/superstar.cpp $(SUPERSTAR_INC)/wget.cpp
KINECT_SRC=$(SRC)/Kinect_DOA.cpp $(SRC)/xcor_td.cpp $(SRC)/ofxKinectExtras.cpp
SKELTRACK_OBJ=pqueue.o skeltrack-joint.o skeltrack-skeleton.o skeltrack-smooth.o skeltrack-util.o

all: skeltrack Sound_Localize_Kinect

skeltrack: $(SKELTRACK_SRC)
	$(CC) $(CFLAGS) $^ $(CLIB) -c

Sound_Localize_Kinect: Sound_Localize_Kinect.cpp $(JSONCPP_SRC) $(KINECT_SRC) $(MONGOOSE_SRC) $(UTILITY_SRC)
	$(CPP) $(CCFLAGS) $^ $(SKELTRACK_OBJ) $(LIB) -o $@

clean:
	-rm -f $(SKELTRACK_OBJ) Sound_Localize_Kinect Sound_Localize_Kinect.exe