#include "serial.hpp"

serial_list_t serial_t::list()
{
	serial_list_t ports;

	sp_port** list_ptr=NULL;

	if(sp_list_ports(&list_ptr)==SP_OK)
	{
		sp_port** list_iter=list_ptr;

		while(*list_iter!=NULL)
		{
			ports.push_back(serial_t(*list_iter).name());
			++list_iter;
		}

		sp_free_port_list(list_ptr);
	}

	return ports;
}

serial_t::serial_t(const std::string& name):port_m(NULL),good_m(false)
{
	if(sp_get_port_by_name(name.c_str(),&port_m)!=SP_OK)
		port_m=NULL;
}

serial_t::serial_t(const serial_t& copy):port_m(NULL),good_m(false)
{
	*this=copy;
}

serial_t& serial_t::operator=(const serial_t& copy)
{
	if(&copy!=this)
	{
		free();

		if(copy.port_m!=NULL)
		{
			sp_copy_port(copy.port_m,&port_m);
			good_m=copy.good_m;
		}
	}

	return *this;
}

serial_t::~serial_t()
{
	free();
}

std::string serial_t::name() const
{
	if(port_m!=NULL)
		return sp_get_port_name(port_m);
	else
		return "";
}

bool serial_t::good() const
{
	return good_m;
}

bool serial_t::open(const size_t baud)
{
	if(port_m!=NULL)
	{
		good_m=sp_open(port_m,SP_MODE_READ_WRITE)==SP_OK&&sp_set_baudrate(port_m,baud)==SP_OK;
		return good();
	}

	return false;
}

void serial_t::close()
{
	if(port_m!=NULL)
	{
		sp_close(port_m);
		good_m=false;
	}
}

size_t serial_t::available()
{
	sp_return bytes=sp_input_waiting(port_m);

	if(bytes>=0)
		return bytes;

	good_m=false;
	return -1;
}

size_t serial_t::read(void* buffer,const size_t count)
{
	if(port_m!=NULL)
	{
		sp_return bytes=sp_nonblocking_read(port_m,buffer,count);

		if(bytes>=0)
			return bytes;
	}

	good_m=false;
	return -1;
}

size_t serial_t::write(const void* buffer,const size_t count)
{
	if(port_m!=NULL)
	{
		sp_return bytes=sp_nonblocking_write(port_m,buffer,count);

		if(bytes>=0)
			return bytes;
	}

	good_m=false;
	return -1;
}

serial_t::serial_t(const sp_port* port):port_m(NULL),good_m(false)
{
	if(port!=NULL)
		sp_copy_port(port,&port_m);
}

void serial_t::free()
{
	if(port_m!=NULL)
		sp_free_port(port_m);

	port_m=NULL;
}