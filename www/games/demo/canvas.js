var kb_left=37;
var kb_up=38;
var kb_right=39;
var kb_down=40;

var canvas_t=function(canvas_obj,setup_func,loop_func,draw_func)
{
	this.canvas=canvas_obj;

	if(!this.canvas)
		return null;

	this.ctx=this.canvas.getContext("2d");

	if(!this.ctx)
		return null;

	this.keys_down=[];
	this.keys_pressed=[];
	this.keys_released=[];

	for(var ii=0;ii<255;++ii)
	{
		this.keys_down[ii]=false;
		this.keys_pressed[ii]=false;
		this.keys_released[ii]=false;
	}

	this.old_time=new Date();
	this.user_setup=setup_func;
	this.user_loop=loop_func;
	this.user_draw=draw_func;

	var myself=this;

	this.keydown=function(evt)
	{
		if(!myself.keys_down[evt.keyCode])
			myself.keys_pressed[evt.keyCode]=true;

		myself.keys_down[evt.keyCode]=true;
	};

	this.keyup=function(evt)
	{
		myself.keys_released[evt.keyCode]=true;
		myself.keys_down[evt.keyCode]=false;
	};

	this.draw=function()
	{
		if(myself.canvas&&myself.ctx&&myself.user_draw)
		{
			myself.ctx.globalCompositeOperation="destination-over";
			myself.ctx.clearRect(0,0,myself.canvas.width,myself.canvas.height);
			myself.user_draw();
			myself.ctx.restore();
		}
	};

	this.loop=function(obj)
	{
		if(myself.user_loop&&myself.old_time&&myself.loop&&myself.draw)
		{
			var new_time=new Date();
			myself.user_loop(Math.min((new_time-myself.old_time)/1000.0,0.1));
			myself.old_time=new_time;
			myself.draw();
		}

		for(var ii=0;ii<255;++ii)
		{
			myself.keys_pressed[ii]=false;
			myself.keys_released[ii]=false;
		}

		window.requestAnimationFrame(function(){myself.loop();});
	};

	this.user_setup();
	this.loop(this);
	this.canvas.addEventListener("keydown",this.keydown,true);
	this.canvas.addEventListener("keyup",this.keyup,true);
};