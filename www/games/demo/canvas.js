var kb_left=37;
var kb_up=38;
var kb_right=39;
var kb_down=40;

function canvas_t(canvas_obj,setup_func,loop_func,draw_func)
{
	var myself=this;

	myself.canvas=canvas_obj;
	myself.old_time=new Date();
	myself.user_setup=setup_func;
	myself.user_loop=loop_func;
	myself.user_draw=draw_func;
	myself.ctx=null;
	myself.keys_down=[];
	myself.keys_pressed=[];
	myself.keys_released=[];

	myself.setup=function()
	{
		if(!myself.canvas)
			return false;

		myself.ctx=myself.canvas.getContext("2d");

		if(!myself.ctx)
			return false;

		for(var ii=0;ii<255;++ii)
		{
			myself.keys_down[ii]=false;
			myself.keys_pressed[ii]=false;
			myself.keys_released[ii]=false;
		}

		myself.user_setup();
		myself.loop();

		window.addEventListener("keydown",myself.keydown,true);
		window.addEventListener("keyup",myself.keyup,true);

		return true;
	};

	myself.keydown=function(evt)
	{
		if(!myself.keys_down[evt.keyCode])
			myself.keys_pressed[evt.keyCode]=true;

		myself.keys_down[evt.keyCode]=true;
	};

	myself.keyup=function(evt)
	{
		myself.keys_released[evt.keyCode]=true;
		myself.keys_down[evt.keyCode]=false;
	};

	myself.draw=function()
	{
		if(myself.canvas&&myself.ctx&&myself.user_draw)
		{
			myself.ctx.globalCompositeOperation="destination-over";
			myself.ctx.clearRect(0,0,myself.canvas.width,myself.canvas.height);
			myself.user_draw();
			myself.ctx.restore();
		}
	};

	myself.loop=function()
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
};