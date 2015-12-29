//state {disconnected=0,opening,firmware_check,configure,connected}
//on_message(messag) - callback triggered when an error or status change is added.
//on_disconnect() - callback triggered when a disconnect is triggered.
//on_read(json) - callback triggered when a packet is read from the arduino.
function connection_t(div,on_message,on_disconnect,on_read)
{
	this.config="";
}

connection_t.prototype.connect=function(port)
{
}

connection_t.prototype.disconnect=function(port)
{
}

connection_t.prototype.connected=function()
{
	return false;
}

connection_t.prototype.reconfigure=function(config)
{
}