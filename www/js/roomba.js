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

	myself.model[0]=url_path+"models/roomba/body.obj";
	myself.model[1]=null;
	myself.model[2]=0xcc0000;

	myself.model[1]=renderer.load_obj(myself.model[0]);
	myself.model[1].rotation.set(3*Math.PI/2,0,0);
	myself.model[1].position.set(0,0,0);
	myself.model[1].scale.set(1,1,1);
	myself.model[1].set_color(myself.model[2]);
	myself.model[1].castShadow=true;
	myself.model[1].receiveShadow=true;

	myself.set_position=function(x,y,z)
	{
		myself.model[1].position.set(x,y,z);
	};

	myself.get_position=function()
	{
		return myself.model[1].position;
	};

	myself.set_rotation=function(x,y,z)
	{
		myself.model[1].rotation.set(x,y,z);
	};

	myself.get_rotation=function()
	{
		return myself.model[1].rotation;
	};

	myself.loop=function(dt)
	{
		if(dt)
		{
			//need an easy way of getting the actual model bounding box...
			var stl_scaler=10;

			var mm=1;

			if(myself.model[1].scale)
				mm=myself.model[1].scale.x*stl_scaler/33.655;

			if(myself.left>500)
				myself.left=500;
			if(myself.left<-500)
				myself.left=-500;
			if(myself.right>500)
				myself.right=500;
			if(myself.right<-500)
				myself.right=-500;

			var speed=myself.left*mm*dt;

			var pos=myself.get_position();
			myself.set_position(pos.x,pos.y,pos.z+speed);
		}
	};

	myself.drive=function(left,right)
	{
		myself.left=left;
		myself.right=right;
	};
};
