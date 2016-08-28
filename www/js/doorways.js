function doorway_manager_t(div,menu)
{
	if(!div||!menu)
		return null;

	var _this=this;
	this.div=div;
	this.el=document.createElement("div");
	this.menu=menu;

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

	this.el.style.overflow="none";
	this.div.appendChild(this.el);

	this.mousemove_listener=function(event){return _this.onmousemove(event);};
	document.addEventListener("mousemove",this.mousemove_listener);
	this.mouseup_listener=function(event){return _this.onmouseup(event);};
	document.addEventListener("mouseup",this.mouseup_listener);
	this.blur_listener=function(event){_this.onblur(event);};
	window.addEventListener("blur",this.blur_listener);
	this.resizer_listener=function(event){_this.onresize(event);};
	window.addEventListener("resize",this.resizer_listener);
}

doorway_manager_t.prototype.destroy=function()
{
	this.remove_all();
	if(this.div&&this.el)
	{
		this.div.removeChild(this.el);
		this.div=this.el=null;
	}
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
	if(this.resizer_listener)
	{
		window.removeEventListener("resize",this.resizer_listener);
		this.resizer_listener=null;
	}
}

doorway_manager_t.prototype.save=function()
{
	var data=[];

	for(let key in this.doorways)
		data.push(this.doorways[key].save());

	data.sort(function(lhs,rhs){return lhs.z-rhs.z;});

	return data;
}

doorway_manager_t.prototype.load=function(data)
{
	if(!data)
		return;
	data.sort(function(lhs,rhs){return lhs.z-rhs.z;});
	for(let key in data)
	{
		var doorway=this.get_by_title(data[key].title);
		if(!doorway)
			doorway=this.create(data[key].title,{x:data[key].x,y:data[key].y});
		doorway.load(data[key]);
	}
}

doorway_manager_t.prototype.create=function(title,pos,tooltip,help_text,icon)
{
	if(!help_text)
		help_text="";
	var doorway=new doorway_t(this,title,help_text);
	this.menu.create(title,function()
	{
		doorway.activate();
	},
	tooltip, icon);
	this.doorways.push(doorway);
	return doorway;
}

doorway_manager_t.prototype.get_by_title=function(title)
{
	for(let key in this.doorways)
		if(this.doorways[key].title==title)
			return this.doorways[key];

	return null;
}

doorway_manager_t.prototype.remove=function(doorway)
{
	var new_doorways=[];
	for(let key in this.doorways)
		if(this.doorways[key]!=doorway)
			new_doorways[key]=doorways;
		else
			this.doorways[key].destroy();
	this.doorways=new_doorways;
}

doorway_manager_t.prototype.remove_all=function()
{
	for(let key in this.doorways)
		this.remove(this.doorways[key]);
	this.doorways.length=0;
}

doorway_manager_t.prototype.hide_all=function()
{
	for(let key in this.doorways)
		this.doorways[key].set_minimized(true);
}

doorway_manager_t.prototype.show_all=function()
{
	for(let key in this.doorways)
	{
		var temp_old_active=this.doorways[key].old_active;
		this.doorways[key].set_minimized(this.doorways[key].old_minimized);
		if(temp_old_active)
			this.doorways[key].activate();
		else
			this.doorways[key].deactivate();
	}
}

doorway_manager_t.prototype.deactivate_all=function()
{
	for(let key in this.doorways)
		this.doorways[key].deactivate();
}

doorway_manager_t.prototype.onmousedown=function(event,doorway)
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
		doorway.activate();
	}

	return false;
}

doorway_manager_t.prototype.onmousemove=function(event)
{
	this.dragging.mouse.x=event.pageX+parseInt(this.div.scrollLeft);
	this.dragging.mouse.y=event.pageY+parseInt(this.div.scrollTop);

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

	if(this.dragging.doorway)
		this.dragging.doorway.move(pos);

	if((event.buttons%2)==0)
		this.onmouseup(event);

	return false;
}

doorway_manager_t.prototype.onmouseup=function(event)
{
	this.dragging.on=false;
	this.dragging.doorway=null;
	return false;
}

doorway_manager_t.prototype.onblur=function(event)
{
	this.onmouseup(event);
}

doorway_manager_t.prototype.onresize=function(event)
{
	for(let key in this.doorways)
		this.doorways[key].move();
}

doorway_manager_t.prototype.offset_left=function()
{
	var offset=parseInt(this.el.offsetLeft);

	if(!offset)
		offset=0;

	return offset;
}

doorway_manager_t.prototype.offset_top=function()
{
	var offset=parseInt(this.el.offsetTop);

	if(!offset)
		offset=0;

	return offset;
}

doorway_manager_t.prototype.offset_width=function()
{
	var width=parseInt(this.el.offsetWidth);

	if(!width)
		width=0;

	return width;
}

doorway_manager_t.prototype.offset_height=function()
{
	var height=parseInt(this.el.offsetHeight);

	if(!height)
		height=0;

	return height;
}

doorway_manager_t.prototype.update_zindicies=function()
{
	this.doorways.sort(function(lhs,rhs)
	{
		var lhs_value=parseInt(lhs.panel.style.zIndex);
		var rhs_value=parseInt(rhs.panel.style.zIndex);
		return rhs_value-lhs_value;
	});

	for(let ii=0;ii<this.doorways.length;++ii)
		if(this.doorways[ii])
			this.doorways[ii].panel.style.zIndex=this.doorways.length-ii;
}




//onactivate() - callback called when doorway is activated.
//ondeactivate() - callback called when doorway is deactivated.
function doorway_t(manager,title,help_text)
{
	if(!manager)
		return null;
	this.manager=manager;

	this.active=true;
	this.minimized=false;
	this.title=title;
	this.pos={x:0,y:0}
	this.z=0;
	var _this=this;

	this.panel=document.createElement("div");
	this.manager.el.appendChild(this.panel);
	this.panel.className="panel panel-primary";
	this.panel.style.position="absolute";
	this.panel.style.margin=0;

	this.bar=document.createElement("div");
	this.panel.appendChild(this.bar);
	this.bar.className="panel-heading";
	this.bar.style.cursor="move";
	this.bar.addEventListener("mousedown",function(event)
	{
		return _this.manager.onmousedown(event,_this);
	});

	this.minimize=document.createElement("span");
	this.bar.appendChild(this.minimize);
	this.minimize.className="glyphicon glyphicon-minus-sign";
	this.minimize.style.cursor="pointer";
	this.minimize.style.float="right";
	this.minimize.addEventListener("click",function(event)
	{
		_this.set_minimized(true);
	});

	if(help_text)
	{
		this.help_button=document.createElement("span");
		this.bar.appendChild(this.help_button);
		this.help_button.className="glyphicon glyphicon-question-sign";
		this.help_button.style.cursor="pointer";
		this.help_button.style.float="right";
		this.help_button.style.marginRight=5;

		this.help_window=new modal_ok_t(this.manager.el,this.title,help_text);
		this.help_button.addEventListener("click",function(event)
		{
			_this.help_window.show();
		});
	}

	this.heading=document.createElement("h3");
	this.bar.appendChild(this.heading);
	this.heading.className="panel-title";
	this.heading.innerHTML=title;

	this.body=document.createElement("div");
	this.panel.appendChild(this.body);
	this.body.className="panel-body";
	this.body.style.overflow="auto";
	this.body.addEventListener("mousedown",function()
	{
		_this.activate(true);
	});

	this.content=document.createElement("div");
	this.body.appendChild(this.content);
	this.content.style.width="100%";
	this.content.style.height="100%";

	this.resizer=new resizer_t(this.body,this.manager.div,{min_size:{width:200,height:100}});

	this.activate();
	this.move(this.pos);
}

doorway_t.prototype.destroy=function()
{
	if(this.resizer)
	{
		this.resizer.destroy();
		this.resizer=null;
	}
	if(this.manager&&this.panel)
	{
		this.manager.el.removeChild(this.panel);
		this.manager=this.panel=null;
	}
}

doorway_t.prototype.set_minimized=function(value)
{
	this.deactivate();
	this.old_minimized=this.minimized;
	this.minimized=value;

	if(value)
		this.panel.style.visibility="hidden";
	else
		this.panel.style.visibility="visible";
}

doorway_t.prototype.activate=function()
{
	this.manager.deactivate_all();

	this.set_minimized(false);
	this.old_active=this.active;
	this.active=true;
	this.panel.className="panel panel-primary";
	this.panel.style.zIndex=this.manager.doorways.length+1;
	this.manager.update_zindicies();

	if(this.onactivate)
		this.onactivate();
}

doorway_t.prototype.deactivate=function()
{
	this.old_active=this.active;
	this.active=false;
	this.panel.className="panel panel-default";

	if(this.ondeactivate)
		this.ondeactivate();
}

doorway_t.prototype.move=function(pos)
{
	if(pos)
	{
		if(pos.x)
			this.pos.x=pos.x;
		if(pos.y)
			this.pos.y=pos.y;
	}
	this.constrain();
	this.panel.style.left=this.pos.x+"px";;
	this.panel.style.top=this.pos.y+"px";;
}

doorway_t.prototype.resize=function(size)
{
	if(size)
		this.resizer.resize(size);
	this.move();
}

doorway_t.prototype.constrain=function()
{
	if(this.pos.x<this.manager.offset_left())
		this.pos.x=this.manager.offset_left();
	if(this.pos.y<this.manager.offset_top())
		this.pos.y=this.manager.offset_top();
}

doorway_t.prototype.save=function()
{
	var data=
	{
		title:this.title,
		x:this.pos.x,
		y:this.pos.y,
		z:parseInt(this.panel.style.zIndex),
		width:parseInt(this.body.offsetWidth),
		height:parseInt(this.body.offsetHeight),
		active:this.active,
		minimized:this.minimized
	};
	return data;
}

doorway_t.prototype.load=function(data)
{
	this.z=0;
	this.move({x:data.x,y:data.y});
	this.resize({width:data.width,height:data.height});
	this.set_minimized(data.minimized);
	if(data.active)
		this.activate();
	else
		this.deactivate();
}
