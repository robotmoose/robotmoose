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
	this.last_serial=null;

	if(!this.is_connectable)
		this.is_connectable=function(){return true;};

	var _this=this;

	this.connected=false;
	this.selected_value=null;

	this.checkmark=new checkmark_t(this.el);

	this.dropdown=new dropdown_t(this.checkmark.getElement(),function()
	{
		return _this.connected;
	});
	this.dropdown.set_width("100%");

	this.button=document.createElement("input");
	this.el.appendChild(this.button);
	this.button.type="button";
	this.button.value="Connect";
	this.button.style.width="50%"
	this.button.onclick=function(){_this.button_m();};

	this.interval=setInterval(function(){_this.update_list_m();},250);
}

serial_selector_t.prototype.connect=function()
{
	if(this.dropdown.size()>0&&!this.connected)
	{
		this.dropdown.disable();
		this.button.value="Disconnect";
		this.selected_value=this.dropdown.selected();
		this.last_serial=this.selected_value;
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
		this.dropdown.enable();
		this.button.value="Connect";
		var old_selected_value=this.selected_value;
		this.selected_value=null;
		this.connected=false;

		if(this.on_disconnect)
			this.on_disconnect(old_selected_value);
	}
}

serial_selector_t.prototype.load=function(robot)
{
	if(robot.serial)
		this.last_serial=robot.serial;
}


serial_selector_t.prototype.build_list_m=function(ports)
{
	var real_list=[];
	var old=this.dropdown.selected();
	var found=false;

	for(let ii=0;ii<ports.length;++ii)
	{
		var name=ports[ii].path;
		if(/.*Bluetooth.*/.test(name))
			continue;
		real_list.push(name);
		if(name==old)
			found=true;
	}

	this.dropdown.build(real_list,this.last_serial);

	if(this.connected&&old&&!found)
		this.disconnect();

	this.button.disabled=(this.dropdown.size()<=0||!this.is_connectable());
	this.checkmark.check(this.dropdown.selected()&&this.connected);
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
