function robot_menu_t(div)
{
	if(!div)
		return null;

	this.div=div;
	this.element=document.createElement("div");
	this.menu={};

	this.menu.nav=document.createElement("nav");
	this.menu.div_outer=document.createElement("div");
	this.menu.div_inner=document.createElement("div");
	this.menu.ul=document.createElement("ul");

	this.drops=[];

	this.div.appendChild(this.element);

	this.menu.nav.className="navbar navbar-default";
	this.menu.nav.style.marginBottom=-1;
	this.element.appendChild(this.menu.nav);

	this.menu.div_outer.className="container-fluid";
	this.menu.nav.appendChild(this.menu.div_outer);

	this.menu.div_inner.className="collapse navbar-collapse";
	this.menu.div_outer.appendChild(this.menu.div_inner);

	this.menu.ul.className="nav navbar-nav";
	this.menu.div_inner.appendChild(this.menu.ul);

	this.create_button
	(
		"Robot",
		null,
		"glyphicon glyphicon-cog",
		{
			"Connect":
			{
				onclick:function(){alert("Connect!");},
				glyph:"glyphicon glyphicon-off"
			},
			"Make a Copy":
			{
				onclick:function(){alert("Copy!");},
				glyph:"glyphicon glyphicon-duplicate"
			}
		}
	);
}

robot_menu_t.prototype.create_button=function(name,onclick,glyph,options)
{
	var drop={};
	this.drops[name]={};
	this.drops[name].li=document.createElement("li");
	this.drops[name].a=document.createElement("a");
	this.drops[name].glyph=document.createElement("span");
	this.drops[name].text=document.createTextNode(" "+name+" ");

	if(!glyph||glyph.length==0)
		this.drops[name].text=document.createTextNode(name+" ");

	this.drops[name].span=document.createElement("span");
	this.drops[name].ul=document.createElement("ul");
	this.drops[name].options={};

	this.drops[name].li.className="dropdown";
	this.menu.ul.appendChild(this.drops[name].li);

	this.drops[name].a.href="javascript:void(0);";
	this.drops[name].a.className="dropdown-toggle";
	this.drops[name].a.setAttribute("data-toggle","dropdown");
	this.drops[name].a.setAttribute("role","button");
	this.drops[name].a.setAttribute("aria-haspopup","true");
	this.drops[name].a.setAttribute("aria-expanded","false");
	this.drops[name].li.appendChild(this.drops[name].a);

	this.drops[name].glyph.className=glyph;
	this.drops[name].a.appendChild(this.drops[name].glyph);

	this.drops[name].a.appendChild(this.drops[name].text);

	this.drops[name].span.className="caret";
	this.drops[name].a.appendChild(this.drops[name].span);

	this.drops[name].ul.className="dropdown-menu";
	this.drops[name].li.appendChild(this.drops[name].ul);

	for(var key in options)
	{
		this.drops[name].options[key]={};
		this.drops[name].options[key].li=document.createElement("li");
		this.drops[name].options[key].a=document.createElement("a");
		this.drops[name].options[key].span=document.createElement("span");
		this.drops[name].options[key].text=document.createTextNode(" "+key);

		this.drops[name].ul.appendChild(this.drops[name].options[key].li);

		this.drops[name].options[key].a.href="javascript:void(0);";
		this.drops[name].options[key].a.onclick=options[key].onclick;
		this.drops[name].options[key].li.appendChild(this.drops[name].options[key].a);

		this.drops[name].options[key].span.className=options[key].glyph;
		this.drops[name].options[key].a.appendChild(this.drops[name].options[key].span);

		this.drops[name].options[key].a.appendChild(this.drops[name].options[key].text);
	}
}