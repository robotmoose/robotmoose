function doorways_t(div)
{
	if(!div)
		return null;

	var myself=this;
	this.div=div;
	this.element=document.createElement("div");
	this.draggable=true;
	this.dragging=null;
	this.mouse_x=0;
	this.mouse_y=0;
	this.offset_x=0;
	this.offset_y=0;
	this.windows={};
	this.menu={};
	this.menu.bar=document.createElement("ul");

	this.menu.show_desktop=this.create_menu_button_m
	(
		"glyphicon glyphicon-eye-close",
		function(event){myself.hide_all_windows();},
		"Click here to hide all windows."
	);

	this.div.appendChild(this.element);

	this.menu.bar.className="nav nav-tabs";
	this.element.appendChild(this.menu.bar);

	document.onmousemove=function(event){return myself.mouse_move_m(event);};
	document.onmouseup=function(event){return myself.mouse_up_m(event);};
	window.onblur=function(event){myself.mouse_up_m(event);myself.draggable=true;};
	window.onresize=function(event){myself.refresh_windows_m();};
}

doorways_t.prototype.save=function()
{
	var data=[];


	for(var key in this.windows)
	{
		var obj={};
		obj.title=key;
		obj.x=parseInt(this.windows[key].window.style.left)-this.element.offsetLeft;
		obj.y=parseInt(this.windows[key].window.style.top)-this.element.offsetTop-this.menu.bar.offsetHeight;
		obj.active=this.windows[key].active;
		obj.minimized=this.windows[key].minimized;

		if(!obj.x)
			obj.x=0;

		if(!obj.y)
			obj.y=0;

		data.push(obj);
	}

	return data;
}

doorways_t.prototype.load=function(data)
{
	if(!data)
		return;

	for(key in data)
	{
		var obj=data[key];

		if(obj.title!=null&&obj.x!=null&&obj.y!=null&&obj.active!=null&&obj.minimized!=null)
		{
			if(!this.windows[data[key].title])
				this.create_window_m(obj.title,obj.x,obj.y,obj.active,obj.minimized);
			else
			{
				var myself=this;

				var force_move=function()
				{
					var x=parseInt(myself.windows[obj.title].window.style.left)-
						parseInt(myself.element.offsetLeft);
					var y=parseInt(myself.windows[obj.title].window.style.top)-
						parseInt(myself.element.offsetTop)-parseInt(myself.menu.bar.offsetHeight);

					if(!x)
						x=0;
					if(!y)
						y=0;

					if(x!=obj.x||y!=obj.y)
						setTimeout(force_move,100);

					myself.move_window(obj.title,obj.x,obj.y);
				};
			}

			force_move();
		}
	}

	for(key in data)
	{
		var obj=data[key];

		if(obj.title!=null&&obj.x!=null&&obj.y!=null&&obj.active!=null&&obj.minimized!=null)
		{
			this.set_menu_item_active(obj.title,obj.active);
			this.minimize_m(obj.title,obj.minimized);
		}
	}

	this.refresh_windows_m();

	//Hack to get refresh windows to be called when the windows are finally done loading...
	var myself=this;
	setTimeout(function(){myself.refresh_windows_m();},100);
	setTimeout(function(){myself.refresh_windows_m();},500);
}

doorways_t.prototype.create_window=function(title,x,y,active,minimized)
{
	if(!title)
		return null;

	if(!this.windows[title])
	{
		this.create_window_m(title,x,y,active,minimized);
		this.refresh_windows_m();
	}
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

	var view_width=window.innerWidth;
	var view_height=window.innerHeight;
	var local_window=this.windows[title].window;

	var old_x=x;
	var old_y=y;

	if(x||x==0)
	{
		x+=this.element.offsetLeft;
		var width=local_window.offsetWidth;
		var right=x+width;
		var x_diff=view_width-right;

		if(x_diff<0)
			x+=x_diff;
		if(x<this.element.offsetLeft)
			x=this.element.offsetLeft;

		local_window.style.left=x;
	}

	if(y||y==0)
	{
		y+=this.element.offsetTop+this.menu.bar.offsetHeight;
		var height=local_window.offsetHeight;
		var bottom=y+height;
		var y_diff=view_height-bottom;

		if(y_diff<0)
			y+=y_diff;
		if(y<this.element.offsetTop+this.menu.bar.offsetHeight)
			y=this.element.offsetTop+this.menu.bar.offsetHeight;

		local_window.style.top=y;
	}
}

doorways_t.prototype.remove_window=function(title)
{
	if(!title)
		return;

	if(!this.windows[title])
		return;

	this.element.removeChild(this.windows[title].window);
	this.menu.bar.removeChild(this.windows[title].menu.li);
	delete this.windows[title];
}

doorways_t.prototype.minimize=function(title,value)
{
	this.minimize_m(title,value);
	this.refresh_windows_m();
}

doorways_t.prototype.hide_all_windows=function()
{
	for(var key in this.windows)
		this.minimize(key,true);
}

doorways_t.prototype.deactivate_all_windows=function()
{
	for(var key in this.windows)
		this.windows[key].active=false;

	this.refresh_windows_m();
}

doorways_t.prototype.set_menu_item_active=function(title,value)
{
	if(!title)
		return;

	if(!this.windows[title])
		return;

	this.windows[title].active=false;

	if(value)
	{
		for(var key in this.windows)
			this.windows[key].active=false;

		this.windows[title].active=true;
		this.windows[title].minimized=false;
	}

	this.refresh_windows_m();
}

doorways_t.prototype.create_menu_button_m=function(glyph,onclick,tooltip)
{
	var myself=this;
	var button=document.createElement("button");
	var span=document.createElement("span");

	button.className="btn btn-default btn-lg";
	button.style.marginTop=1;
	button.style.float="right";
	button.setAttribute("aria-label","Left Align");
	button.onclick=onclick;
	this.menu.bar.appendChild(button);

	span.className=glyph;
	span.style.color="#337ab7";
	span.setAttribute("aria-hidden","true");
	button.appendChild(span);
}

doorways_t.prototype.refresh_windows_m=function()
{
	for(key in this.windows)
	{
		var local_window=this.windows[key];

		if(local_window)
		{
			if(local_window.active)
			{
				local_window.window.style.zIndex=this.zindex_top_m();
				local_window.menu.li.className="active";
				local_window.window.className="panel panel-primary";
				local_window.minimized=false;
			}
			else
			{
				local_window.menu.li.className="";
				local_window.window.className="panel panel-default";
			}

			if(local_window.minimized)
				local_window.window.style.display="none";
			else
				local_window.window.style.display="";

			var x=local_window.window.offsetLeft-this.element.offsetLeft;
			var y=local_window.window.offsetTop-this.element.offsetTop-this.menu.bar.offsetHeight;

			if(!local_window.minimized)
				this.move_window(key,x,y);
		}
	}
}

doorways_t.prototype.create_window_m=function(title,x,y,active,minimized)
{
	if(!this.windows[title])
	{
		var myself=this;

		this.windows[title]={};
		this.windows[title].title=title;

		if(active)
			this.windows[title].active=true;
		else
			this.windows[title].active=false;

		if(minimized)
			this.windows[title].minimized=true;
		else
			this.windows[title].minimized=false;

		this.windows[title].menu={};
		this.windows[title].menu.li=document.createElement("li");
		this.windows[title].menu.a=document.createElement("a");
		this.windows[title].window=document.createElement("div");
		this.windows[title].title_bar=document.createElement("div");
		this.windows[title].title_text=document.createElement("h3");
		this.windows[title].buttons={};
		this.windows[title].buttons.div=document.createElement("div");
		this.windows[title].buttons.minimize=document.createElement("span");
		this.windows[title].body={};
		this.windows[title].body.div=document.createElement("div");
		this.windows[title].body.content=document.createElement("div");

		this.windows[title].window.style.marginBottom=0;
		this.element.appendChild(this.windows[title].window);

		this.menu.bar.appendChild(this.windows[title].menu.li);

		this.windows[title].menu.li.setAttribute("role","presentation");
		this.windows[title].menu.li.appendChild(this.windows[title].menu.a);

		this.windows[title].menu.a.doorways_t=this.windows[title];
		this.windows[title].menu.a.href="javascript:void(0);";
		this.windows[title].menu.a.onclick=function()
		{
			if(this.doorways_t)
			{
				if(!this.doorways_t.active)
					myself.set_menu_item_active(this.doorways_t.title,true);
				//else
					//myself.minimize(this.doorways_t.title,true);
			}
		};
		this.windows[title].menu.a.innerHTML=title;

		this.windows[title].window.className="panel panel-primary";
		this.windows[title].window.style.position="absolute";
		this.windows[title].window.style.zIndex=-1;
		this.windows[title].window.style.zIndex=this.zindex_top_m();
		this.move_window(title,x,y);
		this.windows[title].window.appendChild(this.windows[title].title_bar);
		this.windows[title].window.appendChild(this.windows[title].body.div);

		this.windows[title].title_bar.className="panel-heading";
		this.windows[title].title_bar.style.cursor="move";
		this.windows[title].title_bar.doorways_t=this.windows[title];
		this.windows[title].title_bar.onmousedown=function(event)
		{
			return myself.mouse_down_m(event,this);
		};
		/*this.windows[title].title_bar.ondblclick=function(event)
		{
			if(this.doorways_t)
				myself.minimize(this.doorways_t.title,true);
		};*/
		this.windows[title].title_bar.appendChild(this.windows[title].title_text);

		this.windows[title].title_text.className="panel-title";
		this.windows[title].title_text.innerHTML=title;

		this.windows[title].buttons.div.style.float="right";
		this.windows[title].buttons.div.style.marginLeft=16;
		this.windows[title].title_text.appendChild(this.windows[title].buttons.div);

		this.windows[title].buttons.minimize.className="glyphicon glyphicon-minus-sign";
		this.windows[title].buttons.minimize.style.cursor="pointer";
		this.windows[title].buttons.minimize.doorways_t=this.windows[title];
		this.windows[title].buttons.minimize.onclick=function(event)
		{
			if(!myself.draggable)
			{
				if(this.doorways_t)
					myself.minimize(this.doorways_t.title,true);
				myself.draggable=true;
			}
		};
		this.windows[title].buttons.minimize.onmousedown=function(event)
		{
			myself.mouse_up_m(event);
			myself.draggable=false;
		};
		this.windows[title].buttons.minimize.onmouseleave=function(event)
		{
			if(!myself.draggable)
			{
				myself.mouse_up_m(event);
				myself.draggable=true;
			}
		};
		this.windows[title].buttons.div.appendChild(this.windows[title].buttons.minimize);

		this.windows[title].body.div.doorways_t=this.windows[title];
		this.windows[title].body.div.className="panel-body";
		this.windows[title].body.div.onmousedown=function(event)
		{
			if(myself.draggable&&!myself.dragging&&this.doorways_t)
				myself.set_menu_item_active(this.doorways_t.title,true);
		};
		this.windows[title].body.div.appendChild(this.windows[title].body.content);
	}

	return this.windows[title];
}

doorways_t.prototype.minimize_m=function(title,value)
{
	if(!title)
		return;

	if(!this.windows[title])
		return;

	if(value)
	{
		this.windows[title].minimized=true;
		this.windows[title].active=false;
	}
	else
	{
		this.windows[title].minimized=false;
	}
}

doorways_t.prototype.mouse_down_m=function(event,element)
{
	if(this.draggable&&!this.dragging&&element.doorways_t)
	{
		this.dragging=element.doorways_t;
		this.dragging.window.style.zIndex=this.zindex_top_m();
		this.offset_x=this.mouse_x-this.dragging.window.offsetLeft;
		this.offset_y=this.mouse_y-this.dragging.window.offsetTop;
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
		var y=this.mouse_y-this.offset_y-this.element.offsetTop-this.menu.bar.offsetHeight;

		this.move_window(this.dragging.title,x,y);
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
		if(!lhs||!lhs.window)
			return 1;

		if(!rhs||!rhs.window)
			return -1;

		var lhs_value=lhs.window.style.zIndex;
		var rhs_value=rhs.window.style.zIndex;

		if(lhs_value<0)
			return -1;

		if(rhs_value<0)
			return 1;

		return lhs_value-rhs_value;
	});

	for(var ii=0;ii<array.length;++ii)
		if(array[ii]&&array[ii].title&&this.windows[array[ii].title])
			this.windows[array[ii].title].window.style.zIndex=ii+1;
}

doorways_t.prototype.zindex_top_m=function()
{
	this.update_zindicies_m();
	return Object.keys(this.windows).length+1;
}
