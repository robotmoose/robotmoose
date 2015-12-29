function status_viewer_t(div)
{
	if(!div)
		return null;

	this.div=div;
	this.el=document.createElement("div");
	this.div.appendChild(this.el);

	this.max_lines=40;
	this.lines=[];

	this.textarea=document.createElement("textarea");
	this.el.appendChild(this.textarea);
	this.textarea.wrap="off";
	this.textarea.readOnly=true;
	this.textarea.style.resize="none";
	this.textarea.style.width="370px";
	this.textarea.style.height="350px";
}

status_viewer_t.prototype.destroy=function()
{
	this.div.removeChild(this.el);
}

status_viewer_t.prototype.show=function(message)
{
	if(this.lines.length<=0||message!=this.lines[this.lines.length-1])
	{
		this.lines.push(message);

		while(this.lines.length>this.max_lines)
			this.lines=this.lines.slice(-this.max_lines,this.lines.length);

		this.rebuld_textarea_m();
		this.textarea.scrollTop=this.textarea.scrollHeight;
	}
}



status_viewer_t.prototype.rebuld_textarea_m=function()
{
	this.textarea.innerHTML="";

	for(var ii=0;ii<this.lines.length;++ii)
		this.textarea.innerHTML+=this.lines[ii]+"\n";
}