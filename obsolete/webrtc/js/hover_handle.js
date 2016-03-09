function hover_handle_t(div,side)
{
	if(!div)
		return null;

	this.div=div;
	var myself=this;

	this.el=document.createElement("div");
	this.el.style.top=0;
	this.el.style.float=side;
	this.el.style.margin=this.el.style.padding="0px";
	this.el.style.width="100px";
	this.el.style.height="100%";
	this.el.style.backgroundColor="black";
	this.el.style.opacity=0.1;
	this.el.style.cursor="pointer";
	this.el.onclick=function(){if(myself.onclick)myself.onclick();}
	this.el.onmouseenter=function(){myself.el.style.opacity=0.5;}
	this.el.onmouseleave=function(){myself.el.style.opacity=0.1;}
	this.div.appendChild(this.el);
}

hover_handle_t.prototype.remove=function()
{
	this.div.removeChild(this.el);
}