var kb_left=37;
var kb_up=38;
var kb_right=39;
var kb_down=40;

/**
 Create an input handler, which calls this event function
 when keys are pressed or released on this DOM object.
*/
function input_t(event, DOM)
{
	this.keys_down=[]; // currently down
	this.keys_pressed=[]; // has been pressed
	this.keys_released=[]; // currently released
	this.user_event=function(){event();};

	for(let ii=0;ii<255;++ii)
	{
		this.keys_down[ii]=false;
		this.keys_pressed[ii]=false;
		this.keys_released[ii]=false;
	}

	var _this=this;
	if(!DOM)
		DOM=window;
	DOM.addEventListener("keydown",function(evt){_this.keydown(evt);},true);
	DOM.addEventListener("keyup",function(evt){_this.keyup(evt);},true);

	window.addEventListener("blur",function()
	{
		for(let ii=0;ii<255;++ii)
			_this.keys_down[ii]=false;
		_this.clear();
		_this.user_event();
	});
}

input_t.prototype.keydown=function(evt)
{
	if(!this.keys_down[evt.keyCode])
		this.keys_pressed[evt.keyCode]=true;

	this.keys_down[evt.keyCode]=true;
	this.user_event();
};

input_t.prototype.keyup=function(evt)
{
	this.keys_released[evt.keyCode]=true;
	this.keys_down[evt.keyCode]=false;
	this.user_event();
};


input_t.prototype.clear=function()
{
	for(let ii=0;ii<255;++ii)
	{
		this.keys_pressed[ii]=false;
		this.keys_released[ii]=false;
	}
};

