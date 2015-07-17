function doorways_t(div)
{
	if(!div)
		return null;

	var myself=this;
	this.div=div;
	this.element=document.createElement("div");

	this.dragging=
	{
		on:false,
		doorway:null,
		mouse:
		{
			x:null,
			y:null
		},
		offset:
		{
			x:null,
			y:null
		}
	};

	this.doorways=[];
	this.menu=document.createElement("ul");

	this.div.appendChild(this.element);

	this.menu.className="nav nav-tabs";
	this.element.appendChild(this.menu);

	this.create_menu_button
	(
		"glyphicon glyphicon-eye-close",
		function(event){myself.hide_all();},
		"Click here to hide all windows."
	);

	document.addEventListener("mousemove",function(event){return myself.onmousemove(event);});
	document.addEventListener("mouseup",function(event){return myself.onmouseup(event);});
	window.addEventListener("blur",function(event){myself.onblur(event);});
	window.addEventListener("resize",function(event){myself.onresize(event);});
}

doorways_t.prototype.save=function()
{
	var data=[];

	for(var key in this.doorways)
	{
		var obj=
		{
			title:this.doorways[key].title,
			x:this.doorways[key].pos.x,
			y:this.doorways[key].pos.y,
			z:parseInt(this.doorways[key].panel.style.zIndex),
			width:parseInt(this.doorways[key].body.offsetWidth),
			height:parseInt(this.doorways[key].body.offsetHeight),
			active:this.doorways[key].active,
			minimized:this.doorways[key].minimized
		};

		data.push(obj);
	}

	data.sort(function(lhs,rhs){return lhs.z-rhs.z;});

	return data;
}

doorways_t.prototype.load=function(data)
{
	this.remove_all();

	if(!data)
		return;

	data.sort(function(lhs,rhs){return lhs.z-rhs.z;});

	for(var key in data)
	{
		var doorway=this.create(data[key].title,{x:data[key].x,y:data[key].y});

		if(data[key].width||data[key].height)
			this.resize(doorway,{width:data[key].width,height:data[key].height});

		if(data[key].active)
			this.activate(doorway);
		else
			this.deactivate(doorway);

		if(data[key].minimized)
			this.minimize(doorway);
	}
}

doorways_t.prototype.create=function(title,pos)
{
	var myself=this;

	var doorway=
	{
		active:true,
		minimized:false,
		title:title,
		tab:
		{
			li:document.createElement("li"),
			a:document.createElement("a")
		},
		panel:document.createElement("div"),
		bar:document.createElement("div"),
		heading:document.createElement("h3"),
		minimize:document.createElement("span"),
		body:document.createElement("div"),
		content:document.createElement("div"),
		resizer:null,
		pos:
		{
			x:0,
			y:0
		},
		z:0
	};

	doorway.tab.li.setAttribute("role","presentation");
	this.menu.appendChild(doorway.tab.li);

	doorway.tab.a.doorways_t=doorway;
	doorway.tab.a.innerHTML=title;
	doorway.tab.a.href="javascript:void(0);";
	doorway.tab.a.onclick=function(){myself.activate(this.doorways_t);};
	doorway.tab.li.appendChild(doorway.tab.a);

	doorway.panel.className="panel panel-primary";
	doorway.panel.style.position="absolute";
	doorway.panel.style.margin=0;
	this.element.appendChild(doorway.panel);

	doorway.bar.className="panel-heading";
	doorway.bar.style.cursor="move";
	doorway.bar.doorways_t=doorway;
	doorway.bar.onmousedown=function(event)
	{
		return myself.onmousedown(event,doorway);
	};
	doorway.panel.appendChild(doorway.bar);

	doorway.minimize.className="glyphicon glyphicon-minus-sign";
	doorway.minimize.style.cursor="pointer";
	doorway.minimize.style.float="right";
	doorway.minimize.doorways_t=doorway;
	doorway.minimize.onclick=function(event)
	{
		myself.minimize(this.doorways_t);
	};
	doorway.bar.appendChild(doorway.minimize);

	doorway.heading.className="panel-title";
	doorway.heading.innerHTML=title;
	doorway.bar.appendChild(doorway.heading);

	doorway.body.className="panel-body";
	doorway.body.doorways_t=doorway;
	doorway.body.onclick=doorway.body.onmousedown=function(){myself.activate(this.doorways_t);};
	doorway.body.style.overflow="auto";
	doorway.panel.appendChild(doorway.body);

	doorway.content.style.width="100%";
	doorway.content.style.height="100%";
	doorway.content.doorways_t=doorway;
	doorway.body.appendChild(doorway.content);

	doorway.resizer=new resizer_t(doorway.body,{min_size:{width:200,height:100}});

	this.activate(doorway);
	this.move(doorway,pos);

	this.doorways.push(doorway);

	return doorway;
}

doorways_t.prototype.get_by_title=function(title)
{
	for(var key in this.doorways)
		if(this.doorways[key].title==title)
			return this.doorways[key];

	return null;
}

doorways_t.prototype.move=function(doorway,pos)
{
	if(!doorway)
		return;

	if(pos)
	{
		doorway.pos.x=pos.x;
		doorway.pos.y=pos.y;
	}

	this.constrain(doorway);

	doorway.panel.style.left=doorway.pos.x;
	doorway.panel.style.top=doorway.pos.y;
}

doorways_t.prototype.resize=function(doorway,size)
{
	if(!doorway)
		return;

	if(size)
		doorway.resizer.resize(size);

	this.move(doorway);
}

doorways_t.prototype.remove=function(doorway)
{
	for(var key in this.doorways)
	{
		if(this.doorways[key]===doorway)
		{
			this.element.removeChild(doorway.panel);
			this.menu.removeChild(doorway.tab.li);
			delete this.doorways[key];
			break;
		}
	}
}

doorways_t.prototype.minimize=function(doorway,value)
{
	if(!doorway)
		return;

	if(!value&&value!=false)
		value=true;

	this.deactivate(doorway);
	doorway.minimized=value;

	if(value)
		doorway.panel.style.visibility="hidden";
	else
		doorway.panel.style.visibility="visible";
}

doorways_t.prototype.activate=function(doorway)
{
	this.deactivate_all();

	if(!doorway)
		return;

	this.minimize(doorway,false);
	doorway.active=true;
	doorway.panel.className="panel panel-primary";
	doorway.panel.style.zIndex=this.doorways.length+1;
	doorway.tab.li.className="active";
	this.update_zindicies();
}

doorways_t.prototype.deactivate=function(doorway)
{
	if(!doorway)
		return;

	doorway.active=false;
	doorway.panel.className="panel panel-default";
	doorway.tab.li.className="";
}

doorways_t.prototype.remove_all=function()
{
	for(var key in this.doorways)
		this.remove(this.doorways[key]);

	this.doorways.length=0;
}

doorways_t.prototype.hide_all=function()
{
	for(var key in this.doorways)
		this.minimize(this.doorways[key]);
}

doorways_t.prototype.deactivate_all=function()
{
	for(var key in this.doorways)
		this.deactivate(this.doorways[key]);
}












doorways_t.prototype.onmousedown=function(event,doorway)
{
	if(!this.dragging.on&&!this.dragging.doorway&&doorway)
	{
		this.dragging.on=true;
		this.dragging.doorway=doorway;
		var offset_left=-parseInt(this.dragging.doorway.panel.offsetLeft);
		var offset_top=-parseInt(this.dragging.doorway.panel.offsetTop);

		if(!offset_left)
			offset_left=0;
		if(!offset_top)
			offset_top=0;

		offset_left+=this.dragging.mouse.x;
		offset_top+=this.dragging.mouse.y;

		this.dragging.offset.x=offset_left;
		this.dragging.offset.y=offset_top;
		this.activate(doorway);
	}

	return false;
}

doorways_t.prototype.onmousemove=function(event)
{
	this.dragging.mouse.x=event.pageX;
	this.dragging.mouse.y=event.pageY;

	if(document.all)
	{
		this.dragging.mouse.x=window.event.clientX;
		this.dragging.mouse.y=window.event.clientY;
	}

	var pos=
	{
		x:this.dragging.mouse.x-this.dragging.offset.x,
		y:this.dragging.mouse.y-this.dragging.offset.y
	};

	this.move(this.dragging.doorway,pos);

	if((event.buttons%2)==0)
		this.onmouseup(event);

	return false;
}

doorways_t.prototype.onmouseup=function(event)
{
	this.dragging.on=false;
	this.dragging.doorway=null;
	return false;
}

doorways_t.prototype.onblur=function(event)
{
	this.onmouseup(event);
}

doorways_t.prototype.onresize=function(event)
{
	for(var key in this.doorways)
		this.move(this.doorways[key]);
}

doorways_t.prototype.constrain=function(doorway)
{
	if(!doorway)
		return;

	var view_width=window.innerWidth;
	var view_height=window.innerHeight;

	var width=doorway.panel.offsetWidth;
	var height=doorway.panel.offsetHeight;

	var right=doorway.pos.x+width;
	var bottom=doorway.pos.y+height;

	var x_diff=view_width-right;
	var y_diff=view_height-bottom;

	if(x_diff<0)
		doorway.pos.x+=x_diff;
	if(doorway.pos.x<this.offset_left())
		doorway.pos.x=this.offset_left();

	if(y_diff<0)
		doorway.pos.y+=y_diff;
	if(doorway.pos.y<this.offset_top())
		doorway.pos.y=this.offset_top();
}

doorways_t.prototype.offset_left=function()
{
	var offset=parseInt(this.element.offsetLeft);

	if(!offset)
		offset=0;

	return offset;
}

doorways_t.prototype.offset_top=function()
{
	var offset=parseInt(this.element.offsetTop)+parseInt(this.menu.offsetHeight);

	if(!offset)
		offset=0;

	return offset;
}

doorways_t.prototype.update_zindicies=function()
{
	this.doorways.sort(function(lhs,rhs)
	{
		var lhs_value=parseInt(lhs.panel.style.zIndex);
		var rhs_value=parseInt(rhs.panel.style.zIndex);
		return rhs_value-lhs_value;
	});

	for(var ii=0;ii<this.doorways.length;++ii)
		if(this.doorways[ii])
			this.doorways[ii].panel.style.zIndex=this.doorways.length-ii;
}

doorways_t.prototype.create_menu_button=function(glyph,onclick,tooltip)
{
	var button=document.createElement("button");
	var span=document.createElement("span");

	button.className="btn btn-default btn-lg";
	button.style.marginTop=1;
	button.style.float="right";
	button.setAttribute("aria-label","Left Align");
	button.onclick=onclick;
	this.menu.appendChild(button);

	span.className=glyph;
	span.style.color="#337ab7";
	span.setAttribute("aria-hidden","true");
	button.appendChild(span);
}