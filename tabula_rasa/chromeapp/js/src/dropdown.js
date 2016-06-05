function dropdown_t(div,enable_callback)
{
	if(!div)
		return null;
	this.div=div;
	this.el=document.createElement("div");
	this.el.style.float="left";
	this.div.appendChild(this.el);
	this.enable_callback=enable_callback;
	if(!this.enable_callback)
		this.enable_callback=function(){return false;};
	var _this=this;
	this.selected_value=null;
	this.select=document.createElement("select");
	this.el.appendChild(this.select);
	this.select.onchange=function(){if(_this.onchange)_this.onchange();_this.select.blur();};
	this.mouse_listener=document.addEventListener("mousedown",
		function(){_this.select.blur();});
	this.key_listener=document.addEventListener("keydown",
		function(evt){if(evt.keyCode==9)
		setTimeout(function(){_this.select.blur();},100);});
	this.disabled=false;
}

dropdown_t.prototype.destroy=function()
{
	this.div.removeChild(this.el);
	clearInterval(this.interval);
}

dropdown_t.prototype.size=function()
{
	return this.select.options.length;
}

dropdown_t.prototype.enable=function()
{
	this.disabled=this.select.disabled=false;
	this.set_background_color("cyan");
}

dropdown_t.prototype.disable=function()
{
	this.disabled=this.select.disabled=true;
	this.set_background_color("maroon");
}

dropdown_t.prototype.selected=function()
{
	if(this.select.selectedIndex>=0&&this.select.options.length>this.select.selectedIndex)
		return this.select.options[this.select.selectedIndex].value;
	return null;
}

dropdown_t.prototype.selected_index=function()
{
	return this.select.selectedIndex;
}

dropdown_t.prototype.build=function(list,on_loaded_value)
{
	if(document.activeElement!=this.select)
	{
		var old="";
		var found=false;
		if(this.select.selectedIndex>=0&&this.select.options.length>this.select.selectedIndex)
			old=this.select.options[this.select.selectedIndex].value;
		if(on_loaded_value)
			old=on_loaded_value;
		this.select.options.length=0;
		for(var ii=0;ii<list.length;++ii)
		{
			var option=document.createElement("option");
			option.text=option.value=list[ii];
			this.select.appendChild(option);
			if(list[ii]==old)
				option.selected=found=true;
		}
		if(!found)
			this.select.selectedIndex=0;
		this.select.disabled=(this.disabled||this.enable_callback()||this.select.options.length<=0);
	}
}

dropdown_t.prototype.set_width=function(w)
{
	this.el.style.width=w;
	this.select.style.width="100%";
}

dropdown_t.prototype.set_background_color=function(color)
{
	this.select.style.backgroundColor=color;
}
