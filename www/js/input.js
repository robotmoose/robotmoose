var kb_left=37;
var kb_up=38;
var kb_right=39;
var kb_down=40;

function input_t(loop)
{
	this.interval=null;
	this.keys_down=[];
	this.keys_pressed=[];
	this.keys_released=[];
	this.user_loop=function(){loop();};
}

input_t.prototype.start=function()
{
	for(var ii=0;ii<255;++ii)
	{
		this.keys_down[ii]=false;
		this.keys_pressed[ii]=false;
		this.keys_released[ii]=false;
	}

	var myself=this;
	window.addEventListener("keydown",function(evt){myself.keydown(evt);},true);
	window.addEventListener("keyup",function(evt){myself.keyup(evt);},true);
	this.interval=setInterval(function(){myself.loop();},20);
};

input_t.prototype.stop=function()
{
	clearInterval(this.interval);
	this.interval=null;
};

input_t.prototype.keydown=function(evt)
{
	if(!this.keys_down[evt.keyCode])
		this.keys_pressed[evt.keyCode]=true;

	this.keys_down[evt.keyCode]=true;
};

input_t.prototype.keyup=function(evt)
{
	this.keys_released[evt.keyCode]=true;
	this.keys_down[evt.keyCode]=false;
};

input_t.prototype.loop=function()
{
	if(this.user_loop)
	{
		this.user_loop();

		for(var ii=0;ii<255;++ii)
		{
			this.keys_pressed[ii]=false;
			this.keys_released[ii]=false;
		}
	}
};