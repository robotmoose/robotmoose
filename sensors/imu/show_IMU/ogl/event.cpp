/**
 Keep track of events like keydowns, joystick buttons, etc.
 
 Orion Sky Lawlor, olawlor@acm.org, 2006/06/21 (Public Domain)
*/
#include "ogl/event.h"

volatile int oglAsyncRepaintRequest=0;

/* Storage for oglToggles extern.
  This is an array indexed by character number that's 1 when the key is down.
*/
toggle_t oglToggles;
toggle_t oglKeyMap;

enum {max_buttons=32};
enum {max_axes=32};
static volatile bool joy_buttons[max_buttons];
static volatile float joy_axes[max_axes];
static void update_joy(void);

/**
 Return true if this keyboard key is currently pressed.
 "what" should be a human-readable description of what
 happens when the key gets pressed.
*/
bool oglKey(char c,const char *what) {
	int v=(int)(unsigned char)c;
	if (v<0 || v>=256) return 0;
	return oglToggles[v];
}
bool oglKeyDown(char c,const char *what) {
	int v=(int)(unsigned char)c;
	if (v<0 || v>=256) return 0;
	return oglKeyMap[v];
}

/**
 Return true if this joystick/controller button is pressed.
 "what" is a human-readable description of what will happen.
*/
bool oglButton(int n,const char *what) {
	update_joy();
	if (n<0 || n>=max_buttons) return 0;
	return joy_buttons[n];
}

/**
 Return the value of this joystick axis.
 Axes are normalized to return values from -1 to +1, with
 the rest position at 0.0.
*/
double oglAxis(int n,const char *what) {
	update_joy();
	if (n<0 || n>=max_axes) return 0;
	return joy_axes[n];
}

#ifdef __linux__
/******************* Linux Event Interface **************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h> /* for O_RDONLY */
#include <linux/joystick.h>
#include <pthread.h>

/* Joystick event handling thread.  This is only needed
  to avoid polling because O_ASYNC doesn't work for joystick devices. */
extern "C" void *joystick_thread(void *joy_fd_pointer) 
{
	int joy_fd=(int)(long)joy_fd_pointer;
	printf("Joystick thread running, joystick opened as FD %d\n",joy_fd);
	while (1) {
		struct js_event e;
		if (read(joy_fd,&e,sizeof(e))<=0) continue; /* nothing to grab yet */
		//e.type&=~JS_EVENT_INIT; /* special startup messages */
		int n=e.number+1, v=e.value;
		if (n>max_axes) continue; /* Numbered way too high! */
		if (e.type==JS_EVENT_AXIS) {
			switch (n) {
			case 5: n=4; break; /* fixup axis numbers */
			case 6: n=3; break;
			};
			joy_axes[n]=v*(1.0/0x7fFF);
		} else if (e.type==JS_EVENT_BUTTON) {
			joy_buttons[n]=v;
		} else continue; /* some other weird event */
		
		/* Repaint indefinitely by default */
		oglAsyncRepaintRequest=-1;
		if (v==0) { /* end of a button or axis--one last repaint? */
			for (n=0;n<max_buttons;n++)
				if (joy_buttons[n]!=0) goto keepRepeating;
			for (n=0;n<max_axes;n++)
				if (joy_axes[n]!=0) goto keepRepeating;
			/* All buttons are released--just one more repaint. */
			oglAsyncRepaintRequest=+1;
		keepRepeating: ;
		}
		if (0) printf("Grabbed joystick event: type %d, number %d, value %d; async=%d\n",
			e.type,n,v,oglAsyncRepaintRequest);
	}
	return 0;
}


static void open_joy(void) {
	const static char *names[]={
		"/dev/input/js0", /* default joystick name under Linux */
		"/dev/input/js1", /* unplug-and-replug, or two joysticks? */
		NULL
	};
	for (int i=0;names[i]!=0;i++) {
		int fd=open(names[i],O_RDONLY);
		if (fd<=0) {
			//printf("Error opening joystick...\n");
			continue;
		}
		pthread_t pt;
		pthread_create(&pt,NULL,joystick_thread,(void *)(long)fd);
	}
}

static void update_joy(void) {
	static bool joy_initted=false;
	if (!joy_initted) {
		joy_initted=true;
		open_joy();
	}
}

#elif defined(_WIN32)
/******************** Windows Implementation of Joystick ***************/
#include <windows.h>
#include <string>

extern "C" { /*<- the ancient versions of the Windows headers I've got don't even support C++! */
#include <setupapi.h> /* for SetupDiGetClassDevs and so on */
#pragma comment (lib,"setupapi.lib") /* make linker bring in right library */
//#include <hidsdi.h>
};

/** Return the i'th USB HID device's name (suitable for CreateFile), or 0 if none.

This routine was written after staring at Alan Macek <www.alanmacek.com>'s usb.c/usb.h.
It includes no code written by him, though.
*/
static std::string name_of_USB_HID(int i) {
    std::string ret="";
    /* Nothing interesting or useful is happening here.  Nothing.
       It's just a bunch of STUPID s...tuff required by Windows to get the device name. */
    static const unsigned char s1c[]={
      0xb2,0x55,0x1e,0x4d,0x6f,0xf1,0xcf,0x11,
      0x88,0xcb,0x00,0x11,0x11,0x00,0x00,0x30
    };
    GUID s1;
    memcpy(&s1,s1c,sizeof(s1));
    /* The below annoying hid.lib library can also be used instead:
    HidD_GetHidGuid(&s1); for (int g=0;g<sizeof(GUID);g++)
        printf("0x%02x,",((unsigned char *)&s1)[g]); printf("}\n");
    */
    
    HDEVINFO s2=SetupDiGetClassDevs(&s1,0,0,DIGCF_PRESENT|DIGCF_DEVICEINTERFACE);
    SP_DEVICE_INTERFACE_DATA s3; s3.cbSize = sizeof(s3);
    if (0==SetupDiEnumDeviceInterfaces (s2,0,&s1,i,&s3)) goto abort_s2;
    ULONG s4len; SetupDiGetDeviceInterfaceDetail(s2,&s3,0,0,&s4len,0);
    SP_INTERFACE_DEVICE_DETAIL_DATA *s4=(SP_INTERFACE_DEVICE_DETAIL_DATA *)malloc(s4len);
    s4->cbSize=sizeof(*s4); /* NOT dynamic size; size of fixed-struct part.  Sigh. */
    if (0==SetupDiGetDeviceInterfaceDetail(s2,&s3,s4,s4len,&s4len,0)) goto abort_s4;
    ret=s4->DevicePath; /* finally! */
abort_s4:
    free(s4);
abort_s2:
    SetupDiDestroyDeviceInfoList(s2);
    return ret;
}

/** Return the number of bytes read from this handle. */
static int winread_HID(HANDLE h,void *dest,int tryBytes) {
    DWORD nRead=0;
    ReadFile(h, dest, tryBytes, &nRead, 0);
    return nRead;
}

/**
  Reads data from one USB HID device.
*/
DWORD WINAPI device_read_thread_HID(LPVOID device_name_v)
{
	char *device_name=(char *)device_name_v;
	HANDLE h = CreateFile(device_name, GENERIC_READ, 0,0, OPEN_EXISTING, 0,0);
	if (h==INVALID_HANDLE_VALUE) {
		//printf("Device '%s' can't be opened: error %d\n",device_name,GetLastError());
	} else {
		printf("Successfully opened USB device '%s' for joystick input\n",device_name); fflush(stdout);
		while (1) {
			enum {len=16};
			unsigned char buf[len];
			DWORD nRead=winread_HID(h,&buf[0],len);
			
			if (0) { /* Print the raw incoming data */
				printf("Returned struct of size %d:\n	    ",nRead);
				for (int i=0;i<nRead;i++)    printf("%02x ",buf[i]);
				printf("\n");
			}
			
			int buttons=0;
			if ( (nRead==7 || nRead==14) && (buf[0]==0x01) ) 
			{ /* Saitek P2500 joystick-- first four bytes are axes */
				joy_axes[1]=buf[1]*(1.0/128)-1.0;
				joy_axes[2]=buf[2]*(1.0/128)-1.0;
				joy_axes[3]=buf[4]*(1.0/128)-1.0; /* flipped? */
				joy_axes[4]=buf[3]*(1.0/128)-1.0;
				buttons=buf[5];
				/* FIXME: D-pad is buf[6], and possibly some high buttons (start, select, hat switch) */
			}
			
			/* Copy button bitmask out to array */
			for (unsigned int b=0;b<32;b++)
				joy_buttons[1+b]=!!(buttons&(1<<b));
			
			/* Keep on repainting, as long as we keep getting data */
			oglAsyncRepaintRequest=-1;
		}
		CloseHandle(h);
	}
	free(device_name);
	return 0;
}

static void update_joy(void) {
	static int initted=0;
	if (initted) return;
	else initted=1;
	
	/* Create separate threads to handle each USB HID device.
	   It's safer to do this than poll on each one, because
	   some HID devices only send reports when a button is pressed. */
	std::string r;
	int i=0;
	while (""!=(r=name_of_USB_HID(i++))) {
		HANDLE  thr;
		DWORD	threadID;
		thr = CreateThread(NULL, 0, device_read_thread_HID, 
			(LPVOID)strdup(r.c_str()), 0, &threadID);
	}
}
#else
/******************** Default Implementation of Joystick ***************/
static void update_joy(void) {
	/* do nothing.  All buttons and axes at 0.0 */
}
#endif
