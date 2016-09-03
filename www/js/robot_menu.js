function robot_menu_t(div)
{
	if(!div)
		return null;

	this.div=div;
	this.el=document.createElement("div");
	this.div.appendChild(this.el);

	this.status_area=document.createElement("div");
	this.el.appendChild(this.status_area);
	this.status_area.className="robot menu status_area";

	this.buttons=[];
}

robot_menu_t.prototype.destroy=function()
{
	if(this.div&&this.el)
		this.div.removeChild(this.el);
	this.div=this.el=null;
}

robot_menu_t.prototype.create=function(title,onclick,tooltip,icon)
{
	var button=document.createElement("button");
	this.el.appendChild(button);
	button.className="robot menu button";
	if (icon)
		button.innerHTML = `<span class="glyphicon glyphicon-${icon}"></span>&nbsp;&nbsp;${title}`;
	else button.innerHTML=title;

	button.addEventListener("click",function(event)
	{
		if(onclick)
			onclick(event);
	});
	button.title=tooltip;
	button.style.whiteSpace = 'nowrap';
	this.buttons.push(button);
}

robot_menu_t.prototype.remove=function(doorway)
{
	var new_buttons=[];
	for(let key in this.buttons)
		if(this.buttons[key].doorway!=doorway)
			new_buttons[key]=this.buttons[key];
	this.buttons=new_buttons;
}

robot_menu_t.prototype.get_status_area=function()
{
	return this.status_area;
}

robot_menu_t.prototype.get_menu_bar=function()
{
	return this.el;
}
