function dropdown_t(div,enable_callback,onchange,default_value)
{
	if(!div)
		return null;
	this.div=div;
	this.el=document.createElement("div");
	this.el.style.float="left";
	this.el.style.className="form-group";
	this.div.appendChild(this.el);
	this.enable_callback=enable_callback;
	this.onchange=onchange;
	if(!this.enable_callback)
		this.enable_callback=function(){return false;};
	var _this=this;
	this.value=null;
	this.index=0;
	this.select=document.createElement("select");
	this.select.className="form-control";
	this.el.appendChild(this.select);
	this.select.addEventListener("change",function()
	{
		_this.value=_this.selected_m();
		_this.index=_this.selected_index_m();
		if(_this.onchange)
			_this.onchange(_this.value);
	});
	this.select.addEventListener("select",function()
	{
		window.focus();
	});
	this.disabled=false;
	if(default_value)
	{
		var option=document.createElement("option");
		option.text=option.value=default_value;
		this.select.appendChild(option);
	}
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

dropdown_t.prototype.clear=function()
{
	this.select.selectedIndex=0;
	this.select.options.length=0;
}

dropdown_t.prototype.set_enable=function(value)
{
	if(value)
		this.enable();
	else
		this.disable();
}

dropdown_t.prototype.enable=function()
{
	this.disabled=this.select.disabled=false;
}

dropdown_t.prototype.disable=function()
{
	this.disabled=this.select.disabled=true;
}

dropdown_t.prototype.selected=function()
{
	return this.value;
}

dropdown_t.prototype.selected_index=function()
{
	return this.index;
}

dropdown_t.prototype.build=function(list,on_loaded_value)
{
	if(!list)
		return;
	//console.log("Building dropdown: " + JSON.stringify(list));
	if(document.activeElement!=this.select)
	{
		var old="";
		var found=false;
		if(this.select.selectedIndex>=0&&this.select.options.length>this.select.selectedIndex)
			old=this.select.options[this.select.selectedIndex].value;
		var changed=false;
		if(on_loaded_value)
		{
			if(old)
				changed=true;
			old=on_loaded_value;
		}
		this.select.options.length=0;
		for(let ii=0;ii<list.length;++ii)
		{
			var option=document.createElement("option");
			option.text=option.value=list[ii];
			this.select.appendChild(option);
			if(list[ii]==old)
				option.selected=found=true;
		}
		if(!found)
		{
			changed=false;
			this.select.selectedIndex=0;
			if(this.onchange)
				this.onchange(null);
		}
		var change=(!this.value)||changed;
		this.value=this.selected_m();
		this.index=this.selected_index_m();
		this.select.disabled=(this.disabled||this.enable_callback()||this.select.options.length<=0);
		if(change&&this.onchange)
			this.onchange(this.value);
		return;
	}

	var _this=this;
	setTimeout(function(){_this.build(list,on_loaded_value);},10);
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








dropdown_t.prototype.selected_m=function()
{
	if(this.select.selectedIndex>=0&&this.select.options.length>this.select.selectedIndex)
		return this.select.options[this.select.selectedIndex].value;
	return null;
}

dropdown_t.prototype.selected_index_m=function()
{
	return this.select.selectedIndex;
}