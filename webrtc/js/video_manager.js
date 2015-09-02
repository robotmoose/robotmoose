function video_manager_t(div)
{
	if(!div)
		return null;

	this.div=div;

	this.remote_div=document.createElement("div");
	this.remote_div.style.margin="0px";
	this.remote_div.style.padding="0px";
	this.remote_div.style.width="100%";
	this.remote_div.style.height="100%";
	this.remote_div.style.overflow="hidden";
	this.remote_div.style.position="absolute";
	this.remote_div.style.top=0;
	this.remote_div.style.pointerEvents="none";
	this.remote_div.style.verticalAlign="middle";
	this.remote_div.style.backgroundColor="red";
	this.div.appendChild(this.remote_div);

	this.switching_div=document.createElement("div");
	this.switching_div.style.margin="0px";
	this.switching_div.style.padding="0px";
	this.switching_div.style.width="100%";
	this.switching_div.style.height="100%";
	this.switching_div.style.overflow="hidden";
	this.switching_div.style.position="absolute";
	this.switching_div.style.top=0;
	this.div.appendChild(this.switching_div);

	this.index=0;
	this.objs=[];
}

video_manager_t.prototype.create=function(video,peer)
{
	this.remote_div.style.backgroundColor="white";
	video.style.position="absolute";
	video.style.top="0px";
	video.style.height="0px";
	video.style.padding="0px";
	video.style.margin="0px";
	video.style.left="0px";
	video.style.width="100%";
	video.style.visibility="hidden";

	var myself=this;

	var obj={};
	obj.p=peer;
	obj.v=video;
	obj.p.handleMessage=function(message){if(myself.onreceive)myself.onreceive(message);}

	this.objs.push(obj);

	if(this.objs.length==1)
		this.show_m(0);
}

video_manager_t.prototype.remove=function(video,peer)
{
	var new_objs=[];

	for(var key in this.objs)
		if(this.objs[key].p===peer)
			this.objs[key]=undefined;
		else
			new_objs.push(this.objs[key]);

	this.objs=new_objs;

	if(this.objs.length==0)
		this.remote_div.style.backgroundColor="red";

	while(this.index>=this.objs.length&&this.index>0)
		--this.index;

	this.show_m(this.index);
}

video_manager_t.prototype.left=function()
{
	if(this.index-1>=0)
		--this.index;
	else if(this.objs.length>0)
		this.index=this.objs.length-1;

	this.hide_all_m();
	this.show_m(this.index);
}

video_manager_t.prototype.right=function()
{
	if(this.index+1<this.objs.length)
		++this.index;
	else if(this.objs.length>0)
		this.index=0;

	this.hide_all_m();
	this.show_m(this.index);
}

















video_manager_t.prototype.hide_m=function(index)
{
	this.objs[index].v.style.height="0px";
	this.objs[index].v.style.visibility="hidden";
}

video_manager_t.prototype.show_m=function(index)
{
	index;

	if(index<this.objs.length&&this.index>=0)
	{
		this.objs[index].v.style.height="100%";
		this.objs[index].v.style.visibility="visible";
	}
}

video_manager_t.prototype.hide_all_m=function()
{
	for(var ii=0;ii<this.objs.length;++ii)
		this.hide_m(ii);
}
