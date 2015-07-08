function doorways_t(div)
{
	if(!div)
		return null;

	this.div=div;
	this.element=document.createElement("div");
	this.draggable=true;
	this.dragging=null;
	this.mouse_x=0;
	this.mouse_y=0;
	this.offset_x=0;
	this.offset_y=0;
	this.windows={};
	this.menu=document.createElement("ul");

	this.div.appendChild(this.element);

	this.menu.className="nav nav-pills nav-justified";
	this.element.appendChild(this.menu);

	var myself=this;
	document.onmousemove=function(event){return myself.mouse_move_m(event);};
	document.onmouseup=function(event){return myself.mouse_up_m(event);};
	window.onblur=function(){myself.mouse_up_m(event);myself.draggable=true;};
}

doorways_t.prototype.save=function()
{
	var data=[];

	for(var key in this.windows)
		data.push
		({
			title:key,
			x:parseInt(this.windows[key].window.style.left),
			y:parseInt(this.windows[key].window.style.top)
		});

	return data;
}

doorways_t.prototype.load=function(data)
{
	for(var key in this.windows)
		this.remove_window(key);

	if(data)
		for(key in data)
			this.create_window(data[key].title,data[key].x,data[key].y);
}

doorways_t.prototype.create_window=function(title,x,y)
{
	if(!title)
		return null;

	if(!this.windows[title])
		this.create_window_m(title,x,y);
}

doorways_t.prototype.get_window=function(title)
{
	if(!title)
		return null;

	if(!this.windows[title])
		return null;

	return this.windows[title];
}

doorways_t.prototype.move_window=function(title,x,y)
{
	if(!title)
		return;

	if(!this.windows[title])
		return;

	if(x<this.element.offsetLeft)
			x=this.element.offsetLeft;

	if(y<this.element.offsetTop+this.menu.offsetHeight)
		y=this.element.offsetTop+this.menu.offsetHeight;

	if(x)
		this.windows[title].window.style.left=x;
	if(y)
		this.windows[title].window.style.top=y;
}

doorways_t.prototype.remove_window=function(title)
{
	if(!title)
		return null;

	if(!this.windows[title])
		return;

	this.element.removeChild(this.windows[title]);
	this.windows[title]=null;
}

doorways_t.prototype.minimize=function(title,value)
{
	if(!title)
		return;

	if(!this.windows[title])
		return;

	this.windows[title].window.style.display="";

	if(value)
	{
		this.windows[title].window.style.display="none";
		this.set_menu_item_active_m(title,false);
	}
}

doorways_t.prototype.hide_all=function()
{
	for(var key in this.windows)
		this.minimize(key,true);
}

doorways_t.prototype.set_menu_item_active=function(title,value)
{
	if(!title)
		return;

	if(!this.windows[title])
		return;

	for(var key in this.windows)
		this.set_menu_item_active_m(key,false);

	if(value)
		this.windows[title].window.style.zIndex=this.zindex_top_m();

	this.set_menu_item_active_m(title,value);
	this.minimize(title,!value);
}

doorways_t.prototype.set_menu_item_active_m=function(title,value)
{
	if(!title)
		return;

	if(!this.windows[title])
		return;

	if(value)
	{
		this.windows[title].menu_li.className="active";
		this.windows[title].window.className="panel panel-primary";
	}
	else
	{
		this.windows[title].menu_li.className="";
		this.windows[title].window.className="panel panel-default";
	}
}

doorways_t.prototype.create_window_m=function(title,x,y)
{
	if(!this.windows[title])
	{
		var myself=this;

		this.windows[title]={};
		this.windows[title].title=title;
		this.windows[title].menu_li=document.createElement("li");
		this.windows[title].menu_a=document.createElement("a");
		this.windows[title].window=document.createElement("div");
		this.windows[title].title_bar=document.createElement("div");
		this.windows[title].title_text=document.createElement("h3");
		this.windows[title].minimize=document.createElement("span");
		this.windows[title].body=document.createElement("div");
		this.windows[title].body_content=document.createElement("div");

		this.element.appendChild(this.windows[title].window);

		this.menu.appendChild(this.windows[title].menu_li);

		this.windows[title].menu_li.role="presentation";
		this.windows[title].menu_li.appendChild(this.windows[title].menu_a);

		this.windows[title].menu_a.doorways_t=this.windows[title];
		this.windows[title].menu_a.href="javascript:void(0);";
		this.windows[title].menu_a.onclick=function()
		{
			if(this.doorways_t)
				myself.set_menu_item_active(this.doorways_t.title,true);
		};
		this.windows[title].menu_a.ondblclick=function()
		{
			if(this.doorways_t)
				myself.set_menu_item_active(this.doorways_t.title,false);
		};
		this.windows[title].menu_a.innerHTML=title;

		this.windows[title].window.className="panel panel-primary";
		this.windows[title].window.style.position="absolute";
		this.windows[title].window.style.zIndex=-1;
		this.windows[title].window.style.zIndex=this.zindex_top_m();
		this.move_window(title,x,y);
		this.windows[title].window.appendChild(this.windows[title].title_bar);
		this.windows[title].window.appendChild(this.windows[title].body);

		this.windows[title].title_bar.className="panel-heading";
		this.windows[title].title_bar.style.cursor="move";
		this.windows[title].title_bar.doorways_t=this.windows[title];
		this.windows[title].title_bar.onmousedown=function(event)
		{
			return myself.mouse_down_m(event,this);
		};
		this.windows[title].title_bar.appendChild(this.windows[title].title_text);

		this.windows[title].title_text.className="panel-title";
		this.windows[title].title_text.innerHTML=title;

		this.windows[title].minimize.className="glyphicon glyphicon-minus";
		this.windows[title].minimize.style.cursor="pointer";
		this.windows[title].minimize.style.float="right";
		this.windows[title].minimize.style.marginLeft=16;
		this.windows[title].minimize.doorways_t=this.windows[title];
		this.windows[title].minimize.onclick=function(event)
		{
			if(!myself.draggable)
			{
				myself.minimize_m(this);
				myself.draggable=true;
			}
		};
		this.windows[title].minimize.onmousedown=function(event)
		{
			myself.mouse_up_m(event);
			myself.draggable=false;
		};
		this.windows[title].minimize.onmouseleave=function(event)
		{
			if(!myself.draggable)
			{
				myself.mouse_up_m(event);
				myself.draggable=true;
			}
		};
		this.windows[title].title_text.appendChild(this.windows[title].minimize);

		this.windows[title].body.className="panel-body";
		this.windows[title].body.appendChild(this.windows[title].body_content);
	}

	return this.windows[title];
}

doorways_t.prototype.minimize_m=function(element)
{
	if(element.doorways_t)
		this.minimize(element.doorways_t.title,true);
}

doorways_t.prototype.mouse_down_m=function(event,element)
{
	if(this.draggable&&!this.dragging&&element.doorways_t)
	{
		this.dragging=element.doorways_t.window;
		this.dragging.style.zIndex=this.zindex_top_m();
		this.offset_x=this.mouse_x-this.dragging.offsetLeft;
		this.offset_y=this.mouse_y-this.dragging.offsetTop;
		this.set_menu_item_active(element.doorways_t.title,true);
	}

	return false;
}

doorways_t.prototype.mouse_move_m=function(event)
{
	this.mouse_x=event.pageX;
	this.mouse_y=event.pageY;

	if(document.all)
	{
		this.mouse_x=window.event.clientX;
		this.mouse_y=window.event.clientY;
	}

	if(this.dragging)
	{
		var x=this.mouse_x-this.offset_x;
		var y=this.mouse_y-this.offset_y;

		if(x<this.element.offsetLeft)
			x=this.element.offsetLeft;

		if(y<this.element.offsetTop+this.menu.offsetHeight)
			y=this.element.offsetTop+this.menu.offsetHeight;

		this.dragging.style.left=x;
		this.dragging.style.top=y;
	}

	return false;
}

doorways_t.prototype.mouse_up_m=function(event)
{
	this.dragging=null;
	return false;
}

doorways_t.prototype.update_zindicies_m=function()
{
	var array=[];

	for(var key in this.windows)
		array.push(this.windows[key]);

	array.sort(function(lhs,rhs)
	{
		var lhs_value=lhs.window.style.zIndex;
		var rhs_value=rhs.window.style.zIndex;

		if(lhs_value<0)
			return -1;

		if(rhs_value<0)
			return 1;

		return lhs_value-rhs_value;
	});

	for(var ii=0;ii<array.length;++ii)
		this.windows[array[ii].title].window.style.zIndex=ii+1;
}

doorways_t.prototype.zindex_top_m=function()
{
	this.update_zindicies_m();
	return Object.keys(this.windows).length+1;
}
