function gui_t(div,widgets)
{
	if(!div)
		return null;

	this.div=div;
	this.element=document.createElement("div");
	this.element.className="gridster";
	this.ul=document.createElement("ul");
	this.element.appendChild(this.ul);
	this.div.appendChild(this.element);

	this.gridster=$(".gridster ul").gridster
	({
		widget_base_dimensions:[100,55],
		widget_margins:[5,5],
		resize:{enabled:true}
	}).data("gridster");

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
		this.htmls[widget.name].div=document.createElement("div");
		this.htmls[widget.name].li.appendChild(this.htmls[widget.name].div);
	}
	else
	{
		var old_widget=this.gridster.$widgets.eq(this.widget_lookups[widget.name]);
		this.gridster.mutate_widget_in_gridmap(old_widget,old_widget.coords().grid,widget);
		this.htmls[widget.name].li.removeChild(this.htmls[widget.name].div);
		this.htmls[widget.name].div=document.createElement("div");
		this.htmls[widget.name].li.appendChild(this.htmls[widget.name].div);
	}
}

gui_t.prototype.get_widget=function(name)
{
	if(this.widget_lookups[name]==null)
		return null;

	return this.htmls[name].div
}