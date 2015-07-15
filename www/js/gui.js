/**
  This is an OLD version of the window managing;
  the modern version is "doorways.js".  (2015-07)
*/

var gui_t_grid_size=64;

function gui_t(div,widgets)
{
	if(!div)
		return null;

	//document.html.padding=0;
	//document.html.margin=0;
	this.div=div;
	this.element=document.createElement("div");
	this.element.className="gridster";
	this.ul=document.createElement("ul");
	this.element.appendChild(this.ul);
	this.div.appendChild(this.element);

	var options=
	{
		widget_base_dimensions:[gui_t_grid_size,gui_t_grid_size],
		min_cols:Math.ceil(2000/gui_t_grid_size),
		min_rows:Math.ceil(3000/gui_t_grid_size),
		widget_margins:[4,4],
		resize:{enabled:true},
		draggable:{handle:"header"},
		avoid_overlapped_widgets:false
	};

	this.gridster=$(".gridster ul").gridster(options).data("gridster");

	this.htmls={};
	this.widget_lookups={};
	this.widget_lookups_reverse={};
	this.load(widgets);
}

gui_t.prototype.load=function(widgets)
{
	if(widgets)
	{
		this.gridster.remove_all_widgets();

		for(var ii=0;ii<widgets.length;++ii)
			this.create_widget(widgets[ii]);
	}
}

gui_t.prototype.save=function()
{
	var serialize=this.gridster.serialize();

	for(var ii=0;ii<serialize.length;++ii)
		serialize[ii].name=this.widget_lookups_reverse[ii];

	return serialize;
}

gui_t.prototype.create_widget=function(widget)
{
	if(this.widget_lookups[widget.name]==null)
	{
		var new_index=Object.keys(this.widget_lookups).length;
		this.widget_lookups[widget.name]=new_index;
		this.widget_lookups_reverse[new_index]=widget.name;

		this.htmls[widget.name]={};

		this.htmls[widget.name].li=this.gridster.add_widget("<li>",
			widget.size_x,widget.size_y,widget.col,widget.row)[0];
		this.htmls[widget.name].li.style.paddingBottom=40;

		this.htmls[widget.name].panel=document.createElement("div");
		this.htmls[widget.name].panel.className="panel panel-primary";
		this.htmls[widget.name].panel.style.margin=0;
		this.htmls[widget.name].panel.style.padding=0;

		this.htmls[widget.name].panel_heading=document.createElement("header");
		this.htmls[widget.name].panel_heading.className="panel-heading primary";
		this.htmls[widget.name].panel_heading.innerHTML=widget.name;

		this.htmls[widget.name].li.appendChild(this.htmls[widget.name].panel);
		this.htmls[widget.name].panel.appendChild(this.htmls[widget.name].panel_heading);

		this.create_panel_body_m(widget.name);
	}
	else
	{
		var old_widget=this.gridster.$widgets.eq(this.widget_lookups[widget.name]);
		this.gridster.mutate_widget_in_gridmap(old_widget,old_widget.coords().grid,widget);
		this.create_panel_body_m(widget.name);
	}
}

gui_t.prototype.create_panel_body_m=function(name)
{
	if(name&&this.htmls[name])
	{
		if(this.htmls[name].panel&&this.htmls[name].panel_body)
			this.htmls[name].panel.removeChild(this.htmls[name].panel_body);

		this.htmls[name].panel_body=document.createElement("div");
		this.htmls[name].panel_body.className="panel-body";
		this.htmls[name].panel_body.style.width="100%";
		this.htmls[name].panel_body.style.height="100%";
		this.htmls[name].panel_body.style.boxSizing="border-box";
		this.htmls[name].panel_body.style.margin=0;
		this.htmls[name].panel_body.style.padding=0;
		this.htmls[name].panel.appendChild(this.htmls[name].panel_body);
	}
}

gui_t.prototype.get_widget=function(name)
{
	if(this.widget_lookups[name]==null)
		return null;

	return this.htmls[name].panel_body;
}
