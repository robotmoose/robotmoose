//onresizing() - callback called when resizing is started.

function resizer_t(div,options)
{
	if(!div)
		return null;

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
	this.handle.style.backgroundColor="#ff000000";
	this.handle.onmousedown=function(event){return myself.onmousedown(event);}
	this.div.appendChild(this.handle);

	document.addEventListener("mousemove",function(event){return myself.onmousemove(event);});
	document.addEventListener("mouseup",function(event){return myself.onmouseup(event);});
	window.addEventListener("blur",function(event){myself.onblur(event);});
}

resizer_t.prototype.resize=function(size)
{
	if(!size.width||size.width<this.dragging.min_size.width)
		size.width=this.dragging.min_size.width;
	if(!size.height||size.height<this.dragging.min_size.height)
		size.height=this.dragging.min_size.height;

	this.div.style.width=size.width;
	this.div.style.height=size.height;
	this.handle.style.left=parseInt(this.div.offsetLeft)+parseInt(this.div.offsetWidth)-this.dragging.size.width;
	this.handle.style.top=parseInt(this.div.offsetTop)+parseInt(this.div.offsetHeight)-this.dragging.size.height;
}








resizer_t.prototype.onmousedown=function(event)
{
	if(!this.dragging.on)
	{
		this.dragging.on=true;

		var offset_left=-parseInt(this.handle.offsetLeft);
		var offset_top=-parseInt(this.handle.offsetTop);

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

	if(this.dragging.on)
	{
		this.handle.style.left=this.dragging.mouse.x-this.dragging.offset.x;
		this.handle.style.top=this.dragging.mouse.y-this.dragging.offset.y;
	}
	else
	{
		this.handle.style.left=parseInt(this.div.offsetLeft)+parseInt(this.div.offsetWidth)-this.dragging.size.width;
		this.handle.style.top=parseInt(this.div.offsetTop)+parseInt(this.div.offsetHeight)-this.dragging.size.height;
	}

	if(parseInt(this.handle.offsetLeft)<parseInt(this.div.offsetLeft))
		this.handle.style.left=parseInt(this.div.offsetLeft);
	if(parseInt(this.handle.offsetTop)<parseInt(this.div.offsetTop))
		this.handle.style.top=parseInt(this.div.offsetTop);

	if(this.dragging.on)
		this.resize
		({
			width:parseInt(this.handle.offsetLeft)+this.dragging.size.width-parseInt(this.div.offsetLeft),
			height:parseInt(this.handle.offsetTop)+this.dragging.size.height-parseInt(this.div.offsetTop)
		});

	return false;
}

resizer_t.prototype.onmouseup=function(event)
{
	this.dragging.on=false;

	if(this.onresized)
		this.onresized();

	return false;
}

resizer_t.prototype.onblur=function(event)
{
	this.onmouseup(event);
}