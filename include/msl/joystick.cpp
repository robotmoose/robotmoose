//Windows Dependencies:
//		-lhid -lpthread

//Linux Dependencies:
//		-lpthread

#include "joystick.hpp"

#include <chrono>
#include <stdexcept>
#include <thread>

#if(defined(_WIN32)&&!defined(__CYGWIN__))

#include <Hidsdi.h>

#define HID_USAGE_NONE_DESKTOP			0xff
#define HID_USAGE_PAGE_DESKTOP_CONTROLS	0x01
#define HID_USAGE_ALL					0xff
#define HID_USAGE_UNDEFINED				0x00
#define HID_USAGE_POINTER				0x01
#define HID_USAGE_MOUSE					0x02
#define HID_USAGE_RESERVED				0x03
#define HID_USAGE_JOYSTICK				0x04
#define HID_USAGE_GAMEPAD				0x05
#define HID_USAGE_KEYBOARD				0x06
#define HID_USAGE_KEYPAD				0x07
#define HID_USAGE_CONTROLLER			0x08
#define HID_USAGE_TABLET				0x09
#define HID_USAGE_HAT					0x39

static int hid_get_usage(const RAWINPUTDEVICELIST device)
{
	int return_usage=HID_USAGE_NONE_DESKTOP;
	RID_DEVICE_INFO info;
	unsigned int size=sizeof(RID_DEVICE_INFO);
	info.cbSize=size;

	if(GetRawInputDeviceInfo(device.hDevice,RIDI_DEVICEINFO,&info,&size)==sizeof(RID_DEVICE_INFO))
		if(info.hid.usUsagePage==HID_USAGE_PAGE_DESKTOP_CONTROLS)
			return_usage=info.hid.usUsage;

	return return_usage;
}

static std::vector<RAWINPUTDEVICELIST> hid_list(const int usage)
{
	std::vector<RAWINPUTDEVICELIST> devices;

	unsigned int size=0;
	RAWINPUTDEVICELIST* list=nullptr;

	if(GetRawInputDeviceList(nullptr,&size,sizeof(RAWINPUTDEVICELIST))==0)
	{
		list=new RAWINPUTDEVICELIST[size];

		if(GetRawInputDeviceList(list,&size,sizeof(RAWINPUTDEVICELIST))!=size)
			size=0;
	}

	for(unsigned int ii=0;ii<size;++ii)
		if(usage==HID_USAGE_ALL||hid_get_usage(list[ii])==usage)
			devices.push_back(list[ii]);

	delete[](list);

	return devices;
}

static std::string hid_get_name(const RAWINPUTDEVICELIST device)
{
	std::string return_name="";
	unsigned int size=0;

	if(GetRawInputDeviceInfo(device.hDevice,RIDI_DEVICENAME,nullptr,&size)==0)
	{
		char* name_cstr=new char[size];
		size=GetRawInputDeviceInfo(device.hDevice,RIDI_DEVICENAME,name_cstr,&size);
		return_name=std::string(name_cstr,size);

		if(return_name.find("\\??\\")==0)
			return_name[1]='\\';

		delete[](name_cstr);
	}

	return return_name;
}

typedef std::string preparsed_data_t;

static preparsed_data_t get_preparsed_data(const HANDLE& handle)
{
	preparsed_data_t data;
	unsigned int size=0;

	if(GetRawInputDeviceInfo(handle,RIDI_PREPARSEDDATA,nullptr,&size)==0)
	{
		data.resize(size);

		if(GetRawInputDeviceInfo(handle,RIDI_PREPARSEDDATA,(void*)data.c_str(),&size)!=size)
			data.resize(0);
	}

	return data;
}

static HIDP_CAPS get_caps(const HANDLE& handle)
{
	preparsed_data_t preparsed_data=get_preparsed_data(handle);
	HIDP_CAPS caps;
	HidP_GetCaps((PHIDP_PREPARSED_DATA)preparsed_data.data(),&caps);
	return caps;
}

static std::vector<HIDP_VALUE_CAPS> get_axis_caps(const HANDLE& handle)
{
	std::vector<HIDP_VALUE_CAPS> caps;

	try
	{
		preparsed_data_t preparsed_data=get_preparsed_data(handle);
		unsigned long size=get_caps(handle).NumberInputValueCaps;
		HIDP_VALUE_CAPS* ptr=new HIDP_VALUE_CAPS[size];

		if(HidP_GetValueCaps(HidP_Input,ptr,&size,(PHIDP_PREPARSED_DATA)preparsed_data.data())
			!=HIDP_STATUS_SUCCESS)
			return {};

		for(unsigned int ii=0;ii<size;++ii)
		{
			if(ptr[ii].NotRange.Usage==HID_USAGE_HAT)
			{
				ptr[ii].LogicalMin=0;
				ptr[ii].LogicalMax=255;
			}

			caps.push_back(ptr[ii]);
		}

		delete[](ptr);
	}
	catch(...)
	{}

	return caps;
}

static std::vector<HIDP_BUTTON_CAPS> get_button_caps(const HANDLE& handle)
{
	std::vector<HIDP_BUTTON_CAPS> caps;

	try
	{
		preparsed_data_t preparsed_data=get_preparsed_data(handle);
		unsigned long size=get_caps(handle).NumberInputValueCaps;
		HIDP_BUTTON_CAPS* ptr=new HIDP_BUTTON_CAPS[size];

		if(HidP_GetButtonCaps(HidP_Input,ptr,&size,(PHIDP_PREPARSED_DATA)preparsed_data.data())
			!=HIDP_STATUS_SUCCESS)
			return {};

		for(unsigned int ii=0;ii<size;++ii)
			caps.push_back(ptr[ii]);

		delete[](ptr);
	}
	catch(...)
	{}

	return caps;
}

static bool update_axes(const HANDLE& handle,const std::string& report,std::vector<float>& axes)
{
	preparsed_data_t preparsed_data=get_preparsed_data(handle);
	auto caps=get_axis_caps(handle);
	auto axes_temp=axes;
	unsigned int axes_index=0;

	for(unsigned int ii=0;ii<caps.size();++ii)
	{
		unsigned long value=0;

		if(caps[ii].ReportID==0x01&&HidP_GetUsageValue(HidP_Input,caps[ii].UsagePage,0,caps[ii].NotRange.Usage,&value,
			(PHIDP_PREPARSED_DATA)preparsed_data.c_str(),(char*)report.c_str(),
			report.size())!=HIDP_STATUS_SUCCESS)
			return false;

		axes_temp[axes_index]=value*2.0/(float)(caps[ii].LogicalMax-caps[ii].LogicalMin)-1.0;
		++axes_index;

		if(caps[ii].NotRange.Usage==HID_USAGE_HAT)
		{
			axes_temp[axes_index-1]=axes_temp[axes_index]=0.0;

			if(value>=0&&value<=7)
			{
				if(value>4) axes_temp[axes_index-1]=-1.0;
				if(value>0&&value<4) axes_temp[axes_index-1]=1.0;
				if(value<2||value>6) axes_temp[axes_index]=-1.0;
				if(value>2&&value<6) axes_temp[axes_index]=1.0;
			}

			++axes_index;
		}
	}

	axes=axes_temp;
	return true;
}

static bool update_buttons(const HANDLE& handle,const std::string& report,std::vector<bool>& buttons)
{
	preparsed_data_t preparsed_data=get_preparsed_data(handle);
	auto caps=get_button_caps(handle);
	std::vector<bool> buttons_temp=buttons;

	for(auto ii:buttons_temp)
		ii=false;

	for(unsigned int ii=0;ii<caps.size();++ii)
	{
		unsigned long buttons_count=caps[ii].Range.UsageMax-caps[ii].Range.UsageMin+1;
		USAGE usages[buttons_count];

		if(HidP_GetUsages(HidP_Input,caps[ii].UsagePage,0,usages,&buttons_count,(PHIDP_PREPARSED_DATA)preparsed_data.c_str(),(char*)report.c_str(),report.size())==HIDP_STATUS_SUCCESS)
			for(unsigned int bb=0;bb<buttons_count;++bb)
				buttons_temp[usages[bb]-caps[ii].Range.UsageMin]=true;
	}

	buttons=buttons_temp;
	return true;
}

std::vector<msl::js_info_t> msl::joystick_t::list()
{
	static std::vector<msl::js_info_t> list;

	for(auto ii:hid_list(HID_USAGE_JOYSTICK))
		list.push_back({hid_get_name(ii),ii});

	return list;
}

static bool joystick_valid_fd(const msl::js_fd_t& fd)
{
	return (fd.handle!=INVALID_HANDLE_VALUE);
}

static msl::js_fd_t joystick_open(const msl::js_info_t info)
{
	return {CreateFile(info.name.c_str(),GENERIC_READ,0,nullptr,
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr),info.device};
}

static bool joystick_close(const msl::js_fd_t& fd)
{
	return CloseHandle(fd.handle);
}

static void joystick_update(msl::js_fd_t& fd,std::vector<float>& axes,std::vector<bool>& buttons)
{
	if(joystick_valid_fd(fd))
	{
		std::string report;
		report.resize(get_caps(fd.device.hDevice).InputReportByteLength);
		DWORD bytes_read=0;

		if(ReadFile(fd.handle,(char*)report.c_str(),report.size(),&bytes_read,nullptr)!=0)
		{
			if(bytes_read==report.size()&&report[0]==0x01)
			{
				update_axes(fd.device.hDevice,report,axes);
				update_buttons(fd.device.hDevice,report,buttons);
			}
		}
		else
		{
			fd.handle=INVALID_HANDLE_VALUE;
		}
	}
}

static size_t joystick_axis_count(const msl::js_fd_t& fd)
{
	auto caps=get_axis_caps(fd.device.hDevice);
	size_t count=0;

	for(auto ii:caps)
		if(ii.ReportID==0x01)
		{
			if(ii.NotRange.Usage==HID_USAGE_HAT)
				count+=2;
			else
				count+=ii.Range.UsageMax-ii.Range.UsageMin+1;
		}

	return count;
}

static size_t joystick_button_count(const msl::js_fd_t& fd)
{
	auto caps=get_button_caps(fd.device.hDevice);
	size_t count=0;

	for(auto ii:caps)
		if(ii.ReportID==0x01)
			count+=ii.Range.UsageMax-ii.Range.UsageMin+1;

	return count;
}

#else

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/joystick.h>
#include <unistd.h>

#define INVALID_HANDLE_VALUE (-1)

std::vector<msl::js_info_t> msl::joystick_t::list()
{
	std::vector<msl::js_info_t> list;
	std::vector<std::string> files;

	DIR* dp=opendir("/dev/input");

	while(dp!=nullptr)
	{
		dirent* np=readdir(dp);

		if(np==nullptr)
		{
			closedir(dp);
			break;
		}

		std::string node_name(np->d_name);

		if(node_name!="."&&node_name!=".."&&np->d_type==DT_CHR)
			files.push_back(node_name);
	}

	for(auto ii:files)
		if(ii.find("js")==0)
			list.push_back({"/dev/input/"+ii});

	return list;
}

static bool joystick_valid_fd(const msl::js_fd_t& fd)
{
	return (fd.handle!=INVALID_HANDLE_VALUE);
}

static msl::js_fd_t joystick_open(const msl::js_info_t info)
{
	return {open(info.name.c_str(),O_RDONLY)};
}

static bool joystick_close(const msl::js_fd_t& fd)
{
	return close(fd.handle);
}

static void joystick_update(msl::js_fd_t& fd,std::vector<float>& axes,std::vector<bool>& buttons)
{
	if(joystick_valid_fd(fd))
	{
		js_event js;

		if(read(fd.handle,&js,sizeof(js_event))==sizeof(js_event))
		{
			unsigned char event=js.type&(~JS_EVENT_INIT);

			if(event==JS_EVENT_AXIS)
				axes[js.number]=js.value/32767.0;
			if(event==JS_EVENT_BUTTON)
				buttons[js.number]=js.value;
		}
		else
		{
			fd.handle=INVALID_HANDLE_VALUE;
		}
	}
}

static size_t joystick_axis_count(const msl::js_fd_t& fd)
{
	unsigned char count=0;
	ioctl(fd.handle,JSIOCGAXES,&count);

	return (size_t)count;
}

static size_t joystick_button_count(const msl::js_fd_t& fd)
{
	unsigned char count=0;
	ioctl(fd.handle,JSIOCGBUTTONS,&count);

	return (size_t)count;
}

#endif

msl::joystick_t::joystick_t(const msl::js_info_t info):info_m(info),axes_m({}),buttons_m({})
{}

msl::joystick_t::~joystick_t()
{
	close();
}

void msl::joystick_t::open()
{
	fd_m=joystick_open(info_m);

	if(good())
	{
		lock_m.lock();
		axes_m.resize(joystick_axis_count(fd_m));
		buttons_m.resize(joystick_button_count(fd_m));
		std::thread test(std::bind(&msl::joystick_t::update_m,this));
		test.detach();
		lock_m.unlock();
	}
}

void msl::joystick_t::close()
{
	lock_m.lock();
	joystick_close(fd_m);
	fd_m.handle=INVALID_HANDLE_VALUE;
	lock_m.unlock();
}

bool msl::joystick_t::good()
{
	lock_m.lock();
	auto value=joystick_valid_fd(fd_m);
	lock_m.unlock();
	return value;
}

msl::js_info_t msl::joystick_t::info() const
{
	return info_m;
}

float msl::joystick_t::axis(const size_t index)
{
	if(index>=axis_count())
		throw std::out_of_range("joystick_t::axis");

	lock_m.lock();
	auto value=axes_m[index];
	lock_m.unlock();
	return value;
}

bool msl::joystick_t::button(const size_t index)
{
	if(index>=button_count())
		throw std::out_of_range("joystick_t::button");

	lock_m.lock();
	auto value=buttons_m[index];
	lock_m.unlock();
	return value;
}

size_t msl::joystick_t::axis_count()
{
	lock_m.lock();
	auto count=axes_m.size();
	lock_m.unlock();
	return count;
}

size_t msl::joystick_t::button_count()
{
	lock_m.lock();
	auto count=buttons_m.size();
	lock_m.unlock();
	return count;
}

void msl::joystick_t::update_m()
{
	while(good())
	{
		lock_m.lock();
		auto fd_copy=fd_m;
		auto axes_copy=axes_m;
		auto buttons_copy=buttons_m;
		lock_m.unlock();

		joystick_update(fd_copy,axes_copy,buttons_copy);

		lock_m.lock();
		fd_m=fd_copy;
		axes_m=axes_copy;
		buttons_m=buttons_copy;
		lock_m.unlock();

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}