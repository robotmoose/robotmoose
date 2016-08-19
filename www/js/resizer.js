//onresizing() - callback called when resizing is started.
//onresize() - callback called when resizing is completed - formerly onresized()

function resizer_t(div,containing_div,options)
{
	if(!div)
		return null;
	this.containing_div=containing_div;

	var myself=this;
	this.div=div;
	this.handle=document.createElement("div");

	this.dragging=
	{
		on:false,
		size:{width:16,height:16},
		mouse:{x:0,y:0},
		offset:{x:0,y:0},
		min_size:{width:1,height:1}
	}

	if(options&&options.min_size)
	{
		this.dragging.min_size.width=options.min_size.width;
		this.dragging.min_size.height=options.min_size.height;
	}

	this.handle.style.position="absolute";
	this.handle.style.cursor="se-resize";
	this.handle.style.width=this.dragging.size.width;
	this.handle.style.height=this.dragging.size.height;
	this.handle.style.backgroundColor="red";
	this.handle.style.opacity=0;
	this.handle.style.zIndex=999999;
	this.handle.addEventListener("mousedown",function(event){return myself.onmousedown(event);});
	this.div.appendChild(this.handle);

	this.mousemove_listener=function(event){return myself.onmousemove(event);};
	document.addEventListener("mousemove",this.mousemove_listener);
	this.mouseup_listener=function(event){return myself.onmouseup(event);};
	document.addEventListener("mouseup",this.mouseup_listener);
	this.blur_listener=function(event){myself.onblur(event);};
	window.addEventListener("blur",this.blur_listener);
}

resizer_t.prototype.destroy=function()
{
	if(this.mousemove_listener)
	{
		document.removeEventListener("mousemove",this.mousemove_listener);
		this.mousemove_listener=null;
	}
	if(this.mouseup_listener)
	{
		document.removeEventListener("mouseup",this.mouseup_listener);
		this.mouseup_listener=null;
	}
	if(this.blur_listener)
	{
		window.removeEventListener("blur",this.blur_listener);
		this.blur_listener=null;
	}
	if(this.div&&this.handle)
	{
		this.div.removeChild(this.handle);
		this.div=this.handle=null;
	}
}

resizer_t.prototype.resize=function(size)
{
	if(!size.width||size.width<this.dragging.min_size.width)
		size.width=this.dragging.min_size.width;
	if(!size.height||size.height<this.dragging.min_size.height)
		size.height=this.dragging.min_size.height;

	this.div.style.width=size.width;
	this.div.style.height=size.height;
	this.handle.style.left=parseInt(this.div.offsetLeft)+
		parseInt(this.div.offsetWidth)-this.dragging.size.width;
	this.handle.style.top=parseInt(this.div.offsetTop)+
		parseInt(this.div.offsetHeight)-this.dragging.size.height;
}








resizer_t.prototype.onmousedown=function(event)
{
	if(!this.dragging.on)
	{
		this.dragging.on=true;

		var offset_left=-parseInt(this.handle.offsetLeft)+
			parseInt(this.containing_div.scrollLeft);
		var offset_top=-parseInt(this.handle.offsetTop)+
			parseInt(this.containing_div.scrollTop);

		if(!offset_left)
			offset_left=0;
		if(!offset_top)
			offset_top=0;

		offset_left+=this.dragging.mouse.x;
		offset_top+=this.dragging.mouse.y;

		this.dragging.offset.x=offset_left;
		this.dragging.offset.y=offset_top;

		if(this.onresizing)
			this.onresizing();
	}

	return false;
}

resizer_t.prototype.onmousemove=function(event)
{
	this.dragging.mouse.x=event.pageX;
	this.dragging.mouse.y=event.pageY;

	if(document.all)
	{
		this.dragging.mouse.x=window.event.clientX;
		this.dragging.mouse.y=window.event.clientY;
	}

	var div_ol=parseInt(this.div.offsetLeft);
	var div_ot=parseInt(this.div.offsetTop);
	var div_ow=parseInt(this.div.offsetWidth);
	var div_oh=parseInt(this.div.offsetHeight);

	var handle_ol=parseInt(this.div.offsetLeft);
	var handle_ot=parseInt(this.div.offsetTop);

	if(this.dragging.on)
	{
		this.handle.style.left=this.dragging.mouse.x-this.dragging.offset.x;
		this.handle.style.top=this.dragging.mouse.y-this.dragging.offset.y;
	}
	else
	{
		this.handle.style.left=div_ol+div_ow-this.dragging.size.width;
		this.handle.style.top=div_ot+div_oh-this.dragging.size.height;
	}

	if(handle_ol<div_ol)
		this.handle.style.left=handle_ol;
	if(handle_ot<div_ot)
		this.handle.style.top=handle_ot;

	if(this.dragging.on)
	{
		this.resize
		({
			width:parseInt(this.handle.offsetLeft)+
				this.dragging.size.width-div_ol+
					parseInt(this.containing_div.scrollLeft),
			height:parseInt(this.handle.offsetTop)+
				this.dragging.size.height-div_ot+
					parseInt(this.containing_div.scrollTop)
		});
		if(this.onresize)
			this.onresize();
	}

	return false;
}

resizer_t.prototype.onmouseup=function(event)
{
	this.dragging.on=false;

	return false;
}

resizer_t.prototype.onblur=function(event)
{
	this.onmouseup(event);
}
