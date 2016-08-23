CX=gcc
CXX=g++
SRC=../src
INC=../../../include
SKELTRACK_INC=$(INC)/skeltrack
INC_DIRS=-I$(INC) -I$(INC)/skeltrack -I/usr/include/glib-2.0 -I /usr/lib/x86_64-linux-gnu/glib-2.0/include
C_LIB=-pthread -lgio-2.0 -lgobject-2.0 -lgthread-2.0 -lglib-2.0
C_FLAGS=-Wno-sign-compare $(INC_DIRS)
CPP_LIB=-lfreenect $(C_LIB)
CPP_FLAGS=-std=c++11 -O -Wno-unused-result $(C_FLAGS)

CPP_FLAGS+=-DMG_ENABLE_SSL=1
CPP_LIB+=-lssl -lcrypto

JSONCPP_SRC=$(INC)/jsoncpp/json_reader.cpp $(INC)/jsoncpp/json_value.cpp $(INC)/jsoncpp/json_writer.cpp
MONGOOSE_SRC=$(INC)/mongoose/mongoose.c
SKELTRACK_SRC=$(SKELTRACK_INC)/pqueue.c $(SKELTRACK_INC)/skeltrack-joint.c $(SKELTRACK_INC)/skeltrack-skeleton.c $(SKELTRACK_INC)/skeltrack-smooth.c $(SKELTRACK_INC)/skeltrack-util.c
UTILITY_SRC=$(INC)/auth.cpp $(INC)/ini.cpp $(INC)/json_util.cpp $(INC)/robot_config.cpp $(INC)/string_util.cpp $(INC)/superstar.cpp $(INC)/wget.cpp
KINECT_SRC=$(SRC)/Kinect_DOA.cpp $(SRC)/xcor_td.cpp $(SRC)/ofxKinectExtras.cpp

C_SRC=$(SKELTRACK_SRC) $(MONGOOSE_SRC)
CPP_SRC=$(SRC)/Sound_Localize_Kinect.cpp $(JSONCPP_SRC) $(KINECT_SRC) $(UTILITY_SRC)

all: Sound_Localize_Kinect

Sound_Localize_Kinect: $(C_SRC) $(CPP_SRC)
	$(CX) $(C_FLAGS) -c $(C_SRC) $(C_LIB)
	$(CXX) $(CPP_FLAGS) *.o $(CPP_SRC) $(CPP_LIB) -o $@

clean:
	-rm -f *.o Sound_Localize_Kinect Sound_Localize_Kinect.exe
