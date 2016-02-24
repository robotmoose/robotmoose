/**
  This displays a console-style log of events onscreen.
*/

function status_viewer_t(div)
{
	var this_=this;
	if(!div)
		return null;

	this.div=div;
	this.el=new_div(this.div);
	maximize(this.el);

	this.max_lines=50;
	this.lines=[];
	this.textarea=new_textarea(this.el);
	// this.textarea.style.visibility="hidden";
	this.interval=setInterval(function() {
		this_.rebuild_textarea_m();
	}, 500);
}

status_viewer_t.prototype.destroy=function()
{
	this.div.removeChild(this.el);
	clearInterval(this.interval);
}

status_viewer_t.prototype.show=function(message)
{
	if(this.lines.length<=0||message!=this.lines[this.lines.length-1])
	{
		this.lines.push(message);

		while(this.lines.length>this.max_lines)
			this.lines=this.lines.slice(-this.max_lines,this.lines.length);

		this.textarea.scrollTop=this.textarea.scrollHeight;
	}
}



status_viewer_t.prototype.rebuild_textarea_m=function()
{
	var lines=""; // buffer up lines to prevent repeated innerHTML updates

	for(var ii=0;ii<this.lines.length;++ii)
		lines+=this.lines[ii]+"\n";

	this.textarea.innerHTML=lines;
}

