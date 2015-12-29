//on_connect(port) - callback called when a connection is requested, port is the name of the port TO connect to.
//on_disonnect(port) - callback called when a disconnect is requested, port is the name of the port that WAS connected to.
function serial_selector_t(div,on_connect,on_disconnect)
{
	if(!div)
		return null;

	this.div=div;
	this.el=document.createElement("div");
	this.div.appendChild(this.el);

	this.on_connect=on_connect;
	this.on_disconnect=on_disconnect;

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
	var old_value=-1;

	if(this.select.options.length>0)
		old_value=this.select.options[this.select.selectedIndex].text;

	this.select.length=0;
	var found=false;

	for(var ii=0;ii<ports.length;++ii)
	{
		var option=document.createElement("option");
		this.select.appendChild(option);
		option.text=ports[ii].path;

		if(ports[ii].path==old_value)
			this.select.selectedIndex=ii;

		if(ports[ii].path==this.selected_value)
			found=true;
	}

	if(this.connected&&this.selected_value&&!found)
		this.disconnect();

	this.select.disabled=(this.connected||this.select.options.length<=0);
	this.button.disabled=(this.select.options.length<=0);

	if(this.select.options.length<=0)
	{
		var option=document.createElement("option");
		this.select.appendChild(option);
		option.text="No serial ports.";
	}
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