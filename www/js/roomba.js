var url_path="/"; // http://robotmoose.com/";

function load_js(js)
{
	var scr=document.createElement("script");
	scr.src=js;
	document.head.appendChild(scr);
};

function load_dependencies()
{
	load_js(url_path+"js/3d.js");
};

(function(){load_dependencies()})();

function roomba_t(renderer)
{
	var myself=this;

	myself.model=new Array();

	myself.left=0;
	myself.right=0;

	var model_path=url_path+"models/roomba/body.obj";
	myself.model=null;
	var model_color=0xcc0000;

	myself.model=renderer.load_obj(model_path);
	myself.model.rotation.set(0,0,0);
	myself.model.position.set(0,0,0);
	myself.model.scale.set(1,1,1);
	myself.model.set_color(model_color);
	myself.model.castShadow=true;
	myself.model.receiveShadow=true;

	myself.set_position=function(x,y,z)
	{
		myself.model.position.set(x,y,z);
	};

	myself.get_position=function()
	{
		return myself.model.position;
	};

	myself.set_rotation=function(x,y,z)
	{
		myself.model.rotation.set(x,y,z);
	};

	myself.get_rotation=function()
	{
		return myself.model.rotation;
	};

	myself.loop=function(dt)
	{
		dt = dt || 0.001; // weird startup, null dt?
		var mm=1; // from mm to model coordinates

		// Limit motor power
		if(myself.left>500)
			myself.left=500;
		if(myself.left<-500)
			myself.left=-500;
		if(myself.right>500)
			myself.right=500;
		if(myself.right<-500)
			myself.right=-500;
		
		var speed=mm*dt;
		var speedL=speed*myself.left;
		var speedR=speed*myself.right;

		var pos=myself.get_position();
		myself.set_position(pos.x+speedR,pos.y+speedL,pos.z); // HACK!
	
	};

	myself.drive=function(left,right)
	{
		myself.left=left;
		myself.right=right;
	};
};
