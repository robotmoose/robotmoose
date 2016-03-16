/**
  List serial ports on the machine, and show
  a drop-down menu for the user to select one.


  on_connect(port) - Callback called when a connection is requested, port is the name of the port TO connect to.
  on_disonnect(port) - Callback called when a disconnect is requested, port is the name of the port that WAS connected to.
  is_connectable() - Function called when checking if the connect button is clickable, should return true if it should be and false otherwise.
*/

function serial_selector_t(div,on_connect,on_disconnect,is_connectable)
{
	if(!div)
		return null;

	this.div=div;
	this.el=new_div(this.div);

	this.on_connect=on_connect;
	this.on_disconnect=on_disconnect;
	this.is_connectable=is_connectable;

	if(!this.is_connectable)
		this.is_connectable=function(){return true;};

	var _this=this;

	this.connected=false;
	this.selected_value=null;

	this.select=document.createElement("select");
	this.el.appendChild(this.select);
	this.select.style.width="240px";

	this.button=document.createElement("input");
	this.el.appendChild(this.button);
	this.button.type="button";
	this.button.value="Connect";
	this.button.onclick=function(){_this.button_m();};

	this.interval=setInterval(function(){_this.update_list_m();},250);
}

serial_selector_t.prototype.connect=function()
{
	if(this.select.options.length>0&&!this.connected)
	{
		this.select.disabled=true;
		this.button.value="Disconnect";
		this.selected_value=this.select.options[this.select.selectedIndex].value;
		this.connected=true;

		if(this.on_connect)
			this.on_connect(this.selected_value);
	}
}

serial_selector_t.prototype.destroy=function()
{
	this.disconnect();
	this.div.removeChild(this.div);
	clearInterval(this.interval);
}

serial_selector_t.prototype.disconnect=function()
{
	if(this.connected)
	{
		this.select.disabled=false;
		this.button.value="Connect";
		var old_selected_value=this.selected_value;
		this.selected_value=null;
		this.connected=false;

		if(this.on_disconnect)
			this.on_disconnect(old_selected_value);
	}
}




serial_selector_t.prototype.build_list_m=function(ports)
{
	var old="";
	var found=false;

	if(this.select.selectedIndex>=0&&this.select.options.length>this.select.selectedIndex)
		old=this.select.options[this.select.selectedIndex].value;

	this.select.options.length=0;

	//Uncomment to see Sim as a serial port option
	/*{
		var sim_option=document.createElement("option");
		sim_option.text=sim_option.value="Sim";
		this.select.appendChild(sim_option);
	}*/

	for(var ii=0;ii<ports.length;++ii)
	{
		var name=ports[ii].path;

		// Skip bluetooth devices (on Mac)
		if ( /.*Bluetooth.*/.test( name ) )
			continue;

		var option=document.createElement("option");
		option.text=option.value=name;
		this.select.appendChild(option);

		if(name==old)
			option.selected=found=true;
	}

	if(!found)
		this.select.selectedIndex=0;

	if(this.connected&&this.selected_value&&!found)
		this.disconnect();

	this.select.disabled=(this.connected||this.select.options.length<=0);
	this.button.disabled=(this.select.options.length<=0||!this.is_connectable());
}

serial_selector_t.prototype.button_m=function()
{
	if(this.connected)
		this.disconnect();
	else
		this.connect();
}

serial_selector_t.prototype.update_list_m=function()
{
	var _this=this;
	chrome.serial.getDevices(function(ports){_this.build_list_m(ports);});
}
