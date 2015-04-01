/**
  iRobot Roomba / Create 2 Interface
  
  Mike Moss, 2015-02
*/
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
#define ROOMBA_ID_SENSOR_BUTTONS             18
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

#  define roomba_delay delay

#else

#  include <iostream>
#  define roomba_delay msl::delay_ms

#endif

static bool checksum(const uint8_t header,const uint8_t size,const void* data,const uint8_t cs)
{
	uint16_t sum=header+size+cs;

	for(size_t ii=0;ii<size;++ii)
		sum+=((uint8_t*)data)[ii];

	return (sum&0xff)==0;
}

roomba_t::roomba_t(roomba_serial_t& serial):serial_m(&serial),leds_m(0),
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

	while(serial_m->available()>0 && serial_m->
	#ifdef __AVR
		readBytes((char *)&data,1)==1
	#else
		read(&data,1)==1
	#endif
		)
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
		const uint8_t sensor_count=21;
		const static uint8_t sensors[sensor_count]=
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
			ROOMBA_ID_SENSOR_LIGHT_R,
			ROOMBA_ID_SENSOR_BUTTONS
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
	std::cout<<"m\t"<<(int)sensor_packet_m.mode<<std::endl;
	std::cout<<"bd\t"<<(int)sensor_packet_m.bumper<<std::endl;
	std::cout<<"cs\t"<<(int)sensor_packet_m.battery.state<<std::endl;
	std::cout<<"bv\t"<<(int)sensor_packet_m.battery.voltage<<std::endl;
	std::cout<<"bt\t"<<(int)sensor_packet_m.battery.temperature<<std::endl;
	std::cout<<"bch\t"<<(int)sensor_packet_m.battery.charge<<std::endl;
	std::cout<<"bca\t"<<(int)sensor_packet_m.battery.capacity<<std::endl;
	std::cout<<"enl\t"<<(int)sensor_packet_m.encoder.L<<std::endl;
	std::cout<<"enr\t"<<(int)sensor_packet_m.encoder.L<<std::endl;
	std::cout<<"cl\t"<<(int)sensor_packet_m.floor[0]<<std::endl;
	std::cout<<"cfl\t"<<(int)sensor_packet_m.floor[1]<<std::endl;
	std::cout<<"cfr\t"<<(int)sensor_packet_m.floor[2]<<std::endl;
	std::cout<<"cr\t"<<(int)sensor_packet_m.floor[3]<<std::endl;
	std::cout<<"ll\t"<<(int)sensor_packet_m.light[0]<<std::endl;
	std::cout<<"lfl\t"<<(int)sensor_packet_m.light[1]<<std::endl;
	std::cout<<"lcl\t"<<(int)sensor_packet_m.light[2]<<std::endl;
	std::cout<<"lcr\t"<<(int)sensor_packet_m.light[3]<<std::endl;
	std::cout<<"lfr\t"<<(int)sensor_packet_m.light[4]<<std::endl;
	std::cout<<"lr\t"<<(int)sensor_packet_m.light[5]<<std::endl;
	std::cout<<"ls\t"<<(int)sensor_packet_m.light_field<<std::endl;
	std::cout<<"bs\t"<<(int)sensor_packet_m.buttons<<std::endl;
	std::cout<<std::endl;
	#endif
}

// Read a 16-bit big-endian value from this buffer
uint16_t read_big_16(uint8_t *src)
{
	return (src[0]<<8)|src[1];
}

bool roomba_t::parse_sensor_packet_m()
{
	size_t index=0;

	sensor_t temp_packet;
	while(index<serial_size_m)
	{
		uint8_t sensor=serial_buffer_m[index++];
		if(sensor==ROOMBA_ID_SENSOR_BUMPER_DROP)
		{
			temp_packet.bumper=*(uint8_t*)(serial_buffer_m+index);
			index+=sizeof(uint8_t);
		}
		else if(sensor==ROOMBA_ID_SENSOR_CHARGE_STATE)
		{
			temp_packet.battery.state=*(uint8_t*)(serial_buffer_m+index);
			index+=sizeof(uint8_t);
		}
		else if(sensor==ROOMBA_ID_SENSOR_BATT_VOLTAGE)
		{
			temp_packet.battery.voltage=read_big_16(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(sensor==ROOMBA_ID_SENSOR_BATT_TEMP)
		{
			temp_packet.battery.temperature=*(int8_t*)(serial_buffer_m+index);
			index+=sizeof(int8_t);
		}
		else if(sensor==ROOMBA_ID_SENSOR_BATT_CHARGE)
		{
			temp_packet.battery.charge=read_big_16(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(sensor==ROOMBA_ID_SENSOR_BATT_CAPACITY)
		{
			temp_packet.battery.capacity=read_big_16(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(sensor==ROOMBA_ID_SENSOR_ENCODER_L)
		{
			temp_packet.encoder.L=read_big_16(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(sensor==ROOMBA_ID_SENSOR_ENCODER_R)
		{
			temp_packet.encoder.R=read_big_16(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(sensor==ROOMBA_ID_SENSOR_CLIFF_L)
		{
			temp_packet.floor[0]=read_big_16(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(sensor==ROOMBA_ID_SENSOR_CLIFF_FL)
		{
			temp_packet.floor[1]=read_big_16(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(sensor==ROOMBA_ID_SENSOR_CLIFF_FR)
		{
			temp_packet.floor[2]=read_big_16(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(sensor==ROOMBA_ID_SENSOR_CLIFF_R)
		{
			temp_packet.floor[3]=read_big_16(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(sensor==ROOMBA_ID_SENSOR_MODE)
		{
			temp_packet.mode=*(uint8_t*)(serial_buffer_m+index);
			index+=sizeof(uint8_t);
		}
		else if(sensor==ROOMBA_ID_SENSOR_LIGHT_FIELD)
		{
			temp_packet.light_field=*(uint8_t*)(serial_buffer_m+index);
			index+=sizeof(uint8_t);
		}
		else if(sensor==ROOMBA_ID_SENSOR_LIGHT_L)
		{
			temp_packet.light[0]=read_big_16(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(sensor==ROOMBA_ID_SENSOR_LIGHT_FL)
		{
			temp_packet.light[1]=read_big_16(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(sensor==ROOMBA_ID_SENSOR_LIGHT_CL)
		{
			temp_packet.light[2]=read_big_16(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(sensor==ROOMBA_ID_SENSOR_LIGHT_CR)
		{
			temp_packet.light[3]=read_big_16(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(sensor==ROOMBA_ID_SENSOR_LIGHT_FR)
		{
			temp_packet.light[4]=read_big_16(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if(sensor==ROOMBA_ID_SENSOR_LIGHT_R)
		{
			temp_packet.light[5]=read_big_16(serial_buffer_m+index);
			index+=sizeof(uint16_t);
		}
		else if (sensor==ROOMBA_ID_SENSOR_BUTTONS)
		{
			temp_packet.buttons=*(uint8_t*)(serial_buffer_m+index);
			index+=sizeof(uint8_t);
		}
		else
		{
			return false;
		}
	}

	sensor_packet_m=temp_packet;
	return true;
}
