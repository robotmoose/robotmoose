function robot_menu_t(div)
{
	if(!div)
		return null;

	this.div=div;
	this.element=document.createElement("div");
	this.menu={};

	this.menu.nav=document.createElement("nav");
	this.menu.div=document.createElement("div");
	this.menu.ul=document.createElement("ul");
	this.menu.status={};
	this.menu.status.li=document.createElement("li");
	this.menu.status.a=document.createElement("a");

	this.buttons=[];

	this.div.appendChild(this.element);

	this.menu.nav.className="navbar navbar-default";
	this.menu.nav.style.marginBottom=-1;
	this.element.appendChild(this.menu.nav);

	this.menu.div.className="container-fluid";
	this.menu.nav.appendChild(this.menu.div);

	this.menu.ul.className="nav navbar-nav";
	this.menu.ul.style.width="100%";
	this.menu.div.appendChild(this.menu.ul);

	this.menu.status.li.style.float="right";
	this.menu.ul.appendChild(this.menu.status.li);

	this.menu.status.a.innerHTML="";
	this.menu.status.a.href="javascript:void(0);";
	this.menu.status.a.style.pointerEvents="none";

	this.menu.status.li.appendChild(this.menu.status.a);

	setInterval(function(){$('.hiddenclass').show();},100);
}

robot_menu_t.prototype.create_button=function(name,onclick,glyph,drops,tooltip)
{
	this.buttons[name]={};
	this.buttons[name].li=document.createElement("li");
	if (tooltip) this.buttons[name].li.title=tooltip;
	this.buttons[name].a=document.createElement("a");
	this.buttons[name].glyph=document.createElement("span");
	this.buttons[name].text=document.createTextNode(" "+name+" ");

	if(!glyph||glyph.length==0)
		this.buttons[name].text=document.createTextNode(name+" ");

	this.buttons[name].span=document.createElement("span");
	this.buttons[name].ul=document.createElement("ul");
	this.buttons[name].drops={};

	if(drops)
		this.buttons[name].li.className="dropdown";
	this.menu.ul.appendChild(this.buttons[name].li);

	this.buttons[name].a.href="javascript:void(0);";
	this.buttons[name].a.onclick=onclick;
	if(drops)
	{
		this.buttons[name].a.className="dropdown-toggle";
		this.buttons[name].a.setAttribute("data-toggle","dropdown");
		this.buttons[name].a.setAttribute("role","button");
		this.buttons[name].a.setAttribute("aria-haspopup","true");
		this.buttons[name].a.setAttribute("aria-expanded","false");
	}
	this.buttons[name].li.appendChild(this.buttons[name].a);

	this.buttons[name].glyph.className=glyph;
	this.buttons[name].a.appendChild(this.buttons[name].glyph);

	this.buttons[name].a.appendChild(this.buttons[name].text);

	if(drops)
		this.buttons[name].span.className="caret";
	this.buttons[name].a.appendChild(this.buttons[name].span);

	this.buttons[name].li.appendChild(this.buttons[name].ul);

	if(drops)
	{
		this.buttons[name].ul.className="dropdown-menu";

		for(var key in drops)
		{
			this.buttons[name].drops[key]={};
			this.buttons[name].drops[key].li=document.createElement("li");
			this.buttons[name].drops[key].a=document.createElement("a");
			this.buttons[name].drops[key].span=document.createElement("span");
			this.buttons[name].drops[key].text=document.createTextNode(" "+key);
			this.buttons[name].drops[key].onclick=drops[key].onclick;

			this.buttons[name].ul.appendChild(this.buttons[name].drops[key].li);

			this.buttons[name].drops[key].a.href="javascript:void(0);";
			this.buttons[name].drops[key].a.robot_menu_t=this.buttons[name].drops[key];
			this.buttons[name].drops[key].a.click_event=drops[key].onclick;
			this.buttons[name].drops[key].a.onclick=function()
			{
				if(this.robot_menu_t.li.className!="disabled")
					this.robot_menu_t.onclick();
			}
			this.buttons[name].drops[key].li.appendChild(this.buttons[name].drops[key].a);

			this.buttons[name].drops[key].span.className=drops[key].glyph;
			this.buttons[name].drops[key].a.appendChild(this.buttons[name].drops[key].span);

			this.buttons[name].drops[key].a.appendChild(this.buttons[name].drops[key].text);

			this.buttons[name].drops[key].disable=function()
			{
				this.li.className="disabled";
			}

			this.buttons[name].drops[key].enable=function()
			{
				this.li.className="none";
			}
		}
	}
}

robot_menu_t.prototype.get_status_area=function()
{
	return this.menu.status.a;
}

robot_menu_t.prototype.get_menu_bar=function()
{
	return this.menu.ul;
}
