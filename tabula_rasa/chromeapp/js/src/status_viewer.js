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

	// Theoretically speeds up text rendering:
	this.textarea.style.textRendering="optimizeSpeed";
	this.textarea.style.webkitFontSmoothing="none";
	this.textarea.style.resize="none";
	this.textarea.style.display="flex";

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
		this.lines.push(message);
}

status_viewer_t.prototype.rebuild_textarea_m=function()
{
	var old_lines=this.textarea.value;

	if(this.lines.length>this.max_lines)
		this.lines=this.lines.slice(-this.max_lines,this.lines.length);

	var lines=""; // buffer up lines to prevent repeated innerHTML updates

	for(let ii=0;ii<this.lines.length;++ii)
		lines+=this.lines[ii]+"\n";

	if(lines!=old_lines)
	{
		var t=this.textarea;
		while (t.firstChild) t.removeChild(t.firstChild);
		this.textarea.appendChild(document.createTextNode(lines));
		this.textarea.scrollTop=this.textarea.scrollHeight;
	}
}

