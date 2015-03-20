#include "roomba.h"

#define ROOMBA_ID_START                     128
#define ROOMBA_ID_STOP                      173
#define ROOMBA_ID_RESET                     7
#define ROOMBA_ID_PASSIVE_MODE              133
#define ROOMBA_ID_SAFE_MODE                 131
#define ROOMBA_ID_FULL_MODE                 132
#define ROOMBA_ID_DRIVE_DIRECT              145
#define ROOMBA_ID_LEDS                      139
#define ROOMBA_ID_7_SEGMENT                 164
#define ROOMBA_ID_PLAY_SONG                 141
#define ROOMBA_ID_STREAM_SENSORS_START      148
#define ROOMBA_ID_STREAM_SENSORS_STOP       150

#define ROOMBA_ID_SENSOR_BUMPER_DROP          7
#define ROOMBA_ID_SENSOR_CHARGE_STATE        21
#define ROOMBA_ID_SENSOR_BATT_VOLTAGE        22
#define ROOMBA_ID_SENSOR_BATT_TEMP           24
#define ROOMBA_ID_SENSOR_BATT_CHARGE         25
#define ROOMBA_ID_SENSOR_BATT_CAPACITY       26
#define ROOMBA_ID_SENSOR_ENCODER_L           43
#define ROOMBA_ID_SENSOR_ENCODER_R           44
#define ROOMBA_ID_SENSOR_CLIFF_L             28
#define ROOMBA_ID_SENSOR_CLIFF_FL            29
#define ROOMBA_ID_SENSOR_CLIFF_FR            30
#define ROOMBA_ID_SENSOR_CLIFF_R             31
#define ROOMBA_ID_SENSOR_MODE                35
#define ROOMBA_ID_SENSOR_LIGHT_FIELD         45
#define ROOMBA_ID_SENSOR_LIGHT_L             46
#define ROOMBA_ID_SENSOR_LIGHT_FL            47
#define ROOMBA_ID_SENSOR_LIGHT_CL            48
#define ROOMBA_ID_SENSOR_LIGHT_CR            49
#define ROOMBA_ID_SENSOR_LIGHT_FR            50
#define ROOMBA_ID_SENSOR_LIGHT_R             51

#define ROOMBA_SYNC_TIME                     100
#define ROOMBA_RESET_TIME                    5000

#define ROOMBA_LED_CHECK                      8
#define ROOMBA_LED_DOCK                       4
#define ROOMBA_LED_SPOT                       2
#define ROOMBA_LED_DEBRIS                     1

#define ROOMBA_PACKET_HEADER                 19

#if(defined(__AVR))

	void (*roomba_delay)(unsigned long)=delay;

	roomba_serial_t::roomba_serial_t(const Stream& serial):serial_m((Stream*)&serial)
	{}

	int roomba_serial_t::available()
	{
		return serial_m->available();
	}

	size_t roomba_serial_t::write(const void* buffer,const size_t size)
	{
		return serial_m->write((const uint8_t*)buffer,size);
	}

	size_t roomba_serial_t::read(void* buffer,const size_t size)
	{
		return serial_m->readBytes((char*)buffer,size);
	}

#else

	#include <iostream>

	void (*roomba_delay)(const std::int64_t)=msl::delay_ms;

#endif

static bool checksum(const uint8_t header,const uint8_t size,const void* data,const uint8_t cs)
{
	uint16_t sum=header+size+cs;

	for(size_t ii=0;ii<size;++ii)
		sum+=((uint8_t*)data)[ii];

	return (sum&0xff)==0;
}

roomba_t::roomba_t(const roomba_serial_t& serial):serial_m((roomba_serial_t*)&serial),leds_m(0),
	led_clean_color_m(0),led_clean_brightness_m(0),serial_size_m(0),serial_buffer_m(NULL),
	serial_pointer_m(0),serial_state_m(HEADER)
{
  memset(&sensor_packet_m,0,sizeof(sensor_packet_m));
}

void roomba_t::start()
{
	uint8_t id=ROOMBA_ID_START;
	serial_m->write(&id,1);
	roomba_delay(ROOMBA_SYNC_TIME);
}

void roomba_t::stop()
{
	uint8_t id=ROOMBA_ID_STOP;
	serial_m->write(&id,1);
	roomba_delay(ROOMBA_SYNC_TIME);
}

void roomba_t::reset()
{
	uint8_t id=ROOMBA_ID_RESET;
	serial_m->write(&id,1);
	roomba_delay(ROOMBA_RESET_TIME);
}

void roomba_t::update()
{
	uint8_t data;

	while(serial_m->available()>0&&serial_m->read(&data,1)==1)
	{
		if(serial_state_m==HEADER&&data==ROOMBA_PACKET_HEADER)
		{
			serial_state_m=SIZE;
		}
		else if(serial_state_m==SIZE)
		{
			serial_size_m=data;
			serial_buffer_m=(uint8_t*)malloc(serial_size_m);
			serial_state_m=DATA;
		}
		else if(serial_state_m==DATA)
		{
			serial_buffer_m[serial_pointer_m++]=data;

			if(serial_pointer_m>=serial_size_m)
				serial_state_m=CHECKSUM;
		}
		else if(serial_state_m==CHECKSUM)
		{
			if(checksum(ROOMBA_PACKET_HEADER,serial_size_m,serial_buffer_m,data))
			{
				parse_sensor_packet_m();
				dump_sensors();
			}

			serial_size_m=0;
			free(serial_buffer_m);
			serial_pointer_m=0;
			serial_state_m=HEADER;
		}
	}
}

void roomba_t::set_mode(const mode_t& mode)
{
	uint8_t id=ROOMBA_ID_PASSIVE_MODE;

	if(mode==SAFE)
		id=ROOMBA_ID_SAFE_MODE;
	else if(mode==FULL)
		id=ROOMBA_ID_FULL_MODE;

	serial_m->write(&id,1);
	roomba_delay(ROOMBA_SYNC_TIME);
}

void roomba_t::drive(const int16_t left,const int16_t right)
{
	uint8_t id=ROOMBA_ID_DRIVE_DIRECT;
	serial_m->write(&id,1);
	serial_m->write((uint8_t*)&right+1,1);
	serial_m->write((uint8_t*)&right,1);
	serial_m->write((uint8_t*)&left+1,1);
	serial_m->write((uint8_t*)&left,1);
}

void roomba_t::set_led_check(const bool on)
{
	if(on)
		leds_m|=ROOMBA_LED_CHECK;
	else
		leds_m&=~ROOMBA_LED_CHECK;
}

void roomba_t::set_led_dock(const bool on)
{
	if(on)
		leds_m|=ROOMBA_LED_DOCK;
	else
		leds_m&=~ROOMBA_LED_DOCK;
}

void roomba_t::set_led_spot(const bool on)
{
	if(on)
		leds_m|=ROOMBA_LED_SPOT;
	else
		leds_m&=~ROOMBA_LED_SPOT;
}

void roomba_t::set_led_debris(const bool on)
{
	if(on)
		leds_m|=ROOMBA_LED_DEBRIS;
	else
		leds_m&=~ROOMBA_LED_DEBRIS;
}

void roomba_t::set_led_clean(const uint8_t color,const uint8_t brightness)
{
	led_clean_color_m=color;
	led_clean_brightness_m=brightness;
}

void roomba_t::led_update()
{
	uint8_t id=ROOMBA_ID_LEDS;
	serial_m->write(&id,1);
	serial_m->write(&leds_m,1);
	serial_m->write(&led_clean_color_m,1);
	serial_m->write(&led_clean_brightness_m,1);
}

void roomba_t::set_7_segment(const uint8_t digits[4])
{
	uint8_t id=ROOMBA_ID_7_SEGMENT;
	serial_m->write(&id,1);

	for(int ii=0;ii<4;++ii)
		serial_m->write(digits,4);
}

void roomba_t::set_7_segment(const char* text)
{
	uint8_t digits[4]={32,32,32,32};

	for(int ii=0;ii<4;++ii)
	{
		if(text[ii]=='\0')
			break;

		digits[ii]=text[ii];
	}

	set_7_segment(digits);
}

void roomba_t::play_song(const uint8_t number)
{
	uint8_t id=ROOMBA_ID_PLAY_SONG;
	serial_m->write(&id,1);
	serial_m->write(&number,1);
}

void roomba_t::set_receive_sensors(const bool on)
{
	if(on)
	{
		uint8_t id=ROOMBA_ID_STREAM_SENSORS_START;
		const uint8_t sensor_count=20;
		uint8_t sensors[sensor_count]=
		{
			ROOMBA_ID_SENSOR_BUMPER_DROP,
			ROOMBA_ID_SENSOR_CHARGE_STATE,
			ROOMBA_ID_SENSOR_BATT_VOLTAGE,
			ROOMBA_ID_SENSOR_BATT_TEMP,
			ROOMBA_ID_SENSOR_BATT_CHARGE,
			ROOMBA_ID_SENSOR_BATT_CAPACITY,
			ROOMBA_ID_SENSOR_ENCODER_L,
			ROOMBA_ID_SENSOR_ENCODER_R,
			ROOMBA_ID_SENSOR_CLIFF_L,
			ROOMBA_ID_SENSOR_CLIFF_FL,
			ROOMBA_ID_SENSOR_CLIFF_FR,
			ROOMBA_ID_SENSOR_CLIFF_R,
			ROOMBA_ID_SENSOR_MODE,
			ROOMBA_ID_SENSOR_LIGHT_FIELD,
			ROOMBA_ID_SENSOR_LIGHT_L,
			ROOMBA_ID_SENSOR_LIGHT_FL,
			ROOMBA_ID_SENSOR_LIGHT_CL,
			ROOMBA_ID_SENSOR_LIGHT_CR,
			ROOMBA_ID_SENSOR_LIGHT_FR,
			ROOMBA_ID_SENSOR_LIGHT_R
		};

		serial_m->write(&id,1);
		serial_m->write(&sensor_count,1);
		serial_m->write(sensors,sensor_count);
	}
	else
	{
		uint8_t id=ROOMBA_ID_STREAM_SENSORS_STOP;
		serial_m->write(&id,1);
	}
}

roomba_t::sensor_t roomba_t::get_sensors() const
{
	return sensor_packet_m;
}

void roomba_t::dump_sensors() const
{
	#if(!defined(__AVR))
	std::cout<<"bd\t"<<(int)sensor_packet_m.bumper_drop<<std::endl;
	std::cout<<"cs\t"<<(int)sensor_packet_m.charge_state<<std::endl;
	std::cout<<"bv\t"<<(int)sensor_packet_m.batt_voltage<<std::endl;
	std::cout<<"bt\t"<<(int)sensor_packet_m.batt_temp<<std::endl;
	std::cout<<"bch\t"<<(int)sensor_packet_m.batt_charge<<std::endl;
	std::cout<<"bca\t"<<(int)sensor_packet_m.batt_capacity<<std::endl;
	std::cout<<"enl\t"<<(int)sensor_packet_m.encoder_l<<std::endl;
	std::cout<<"enr\t"<<(int)sensor_packet_m.encoder_r<<std::endl;
	std::cout<<"cl\t"<<(int)sensor_packet_m.cliff_l<<std::endl;
	std::cout<<"cfl\t"<<(int)sensor_packet_m.cliff_fl<<std::endl;
	std::cout<<"cfr\t"<<(int)sensor_packet_m.cliff_fr<<std::endl;
	std::cout<<"cr\t"<<(int)sensor_packet_m.cliff_r<<std::endl;
	std::cout<<"m\t"<<(int)sensor_packet_m.mode<<std::endl;
	std::cout<<"ls\t"<<(int)sensor_packet_m.light_field<<std::endl;
	std::cout<<"ll\t"<<(int)sensor_packet_m.light_l<<std::endl;
	std::cout<<"lfl\t"<<(int)sensor_packet_m.light_fl<<std::endl;
	std::cout<<"lcl\t"<<(int)sensor_packet_m.light_cl<<std::endl;
	std::cout<<"lcr\t"<<(int)sensor_packet_m.light_cr<<std::endl;
	std::cout<<"lfr\t"<<(int)sensor_packet_m.light_fr<<std::endl;
	std::cout<<"lr\t"<<(int)sensor_packet_m.light_r<<std::endl;
	std::cout<<std::endl;
	#endif
}

bool roomba_t::parse_sensor_packet_m()
{
	size_t index=0;

	sensor_t temp_packet;

	while(index<serial_size_m)
	{
		if(serial_buffer_m[index]==ROOMBA_ID_SENSOR_BUMPER_DROP)
		{
			++index;
			temp_packet.bumper_drop=*(uint8_t*)(serial_buffer_m+index);
			index+=sizeof(uint8_t);
		}
		else if(serial_buffer_m[index]==ROOMBA_ID_SENSOR_CHARGE_STATE)
		{
			++index;
			temp_packet.charge_state=*(uint8_t*)(serial_buffer_m+index);
			index+=sizeof(uint8_t);
		}
		else if(serial_buffer_m[index]==ROOMBA_ID_SENSOR_BATT_VOLTAGE)
		{
			++index;
			temp_packet.batt_voltage=*(uint16_t*)(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(serial_buffer_m[index]==ROOMBA_ID_SENSOR_BATT_TEMP)
		{
			++index;
			temp_packet.batt_temp=*(int8_t*)(serial_buffer_m+index);
			index+=sizeof(int8_t);
		}
		else if(serial_buffer_m[index]==ROOMBA_ID_SENSOR_BATT_CHARGE)
		{
			++index;
			temp_packet.batt_charge=*(uint16_t*)(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(serial_buffer_m[index]==ROOMBA_ID_SENSOR_BATT_CAPACITY)
		{
			++index;
			temp_packet.batt_capacity=*(uint16_t*)(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(serial_buffer_m[index]==ROOMBA_ID_SENSOR_ENCODER_L)
		{
			++index;
			temp_packet.encoder_l=*(uint16_t*)(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(serial_buffer_m[index]==ROOMBA_ID_SENSOR_ENCODER_R)
		{
			++index;
			temp_packet.encoder_r=*(uint16_t*)(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(serial_buffer_m[index]==ROOMBA_ID_SENSOR_CLIFF_L)
		{
			++index;
			temp_packet.cliff_l=*(uint16_t*)(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(serial_buffer_m[index]==ROOMBA_ID_SENSOR_CLIFF_FL)
		{
			++index;
			temp_packet.cliff_fl=*(uint16_t*)(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(serial_buffer_m[index]==ROOMBA_ID_SENSOR_CLIFF_FR)
		{
			++index;
			temp_packet.cliff_fr=*(uint16_t*)(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(serial_buffer_m[index]==ROOMBA_ID_SENSOR_CLIFF_R)
		{
			++index;
			temp_packet.cliff_r=*(uint16_t*)(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(serial_buffer_m[index]==ROOMBA_ID_SENSOR_MODE)
		{
			++index;
			temp_packet.mode=*(uint8_t*)(serial_buffer_m+index);
			index+=sizeof(uint8_t);
		}
		else if(serial_buffer_m[index]==ROOMBA_ID_SENSOR_LIGHT_FIELD)
		{
			++index;
			temp_packet.light_field=*(uint8_t*)(serial_buffer_m+index);
			index+=sizeof(uint8_t);
		}
		else if(serial_buffer_m[index]==ROOMBA_ID_SENSOR_LIGHT_L)
		{
			++index;
			temp_packet.light_l=*(uint16_t*)(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(serial_buffer_m[index]==ROOMBA_ID_SENSOR_LIGHT_FL)
		{
			++index;
			temp_packet.light_fl=*(uint16_t*)(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(serial_buffer_m[index]==ROOMBA_ID_SENSOR_LIGHT_CL)
		{
			++index;
			temp_packet.light_cl=*(uint16_t*)(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(serial_buffer_m[index]==ROOMBA_ID_SENSOR_LIGHT_CR)
		{
			++index;
			temp_packet.light_cr=*(uint16_t*)(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(serial_buffer_m[index]==ROOMBA_ID_SENSOR_LIGHT_FR)
		{
			++index;
			temp_packet.light_fr=*(uint16_t*)(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(serial_buffer_m[index]==ROOMBA_ID_SENSOR_LIGHT_R)
		{
			++index;
			temp_packet.light_r=*(uint16_t*)(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else
		{
			return false;
		}
	}

	sensor_packet_m=temp_packet;
	return true;
}
