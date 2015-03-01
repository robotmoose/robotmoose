function load_js(js)
{
	var scr=document.createElement("script");
	scr.src=js;
	document.head.appendChild(scr);
};

function load_dependencies()
{
	load_js("js/3d.js");
};

(function(){load_dependencies()})();

function quizbot_t(renderer)
{
	var myself=this;
	myself.models=new Array();

	for(var ii=0;ii<5;++ii)
		myself.models[ii]=new Array();

	myself.models[0][0]="models/base.obj";
	myself.models[0][1]=0;
	myself.models[0][2]=0;
	myself.models[0][3]=0;
	myself.models[0][4]=null;
	myself.models[0][5]=0xff0000;

	myself.models[1][0]="models/base_servo.obj";
	myself.models[1][1]=0;
	myself.models[1][2]=0;
	myself.models[1][3]=0;
	myself.models[1][4]=null;
	myself.models[1][5]=0x00ff00;

	myself.models[2][0]="models/middle.obj";
	myself.models[2][1]=0.38;
	myself.models[2][2]=37;
	myself.models[2][3]=-12.38;
	myself.models[2][4]=null;
	myself.models[2][5]=0xff0000;

	myself.models[3][0]="models/middle_servo.obj";
	myself.models[3][1]=0.38;
	myself.models[3][2]=37;
	myself.models[3][3]=-12.38;
	myself.models[3][4]=null;
	myself.models[3][5]=0x00ff00;

	myself.models[4][0]="models/head.obj";
	myself.models[4][1]=-0.17;
	myself.models[4][2]=68.5;
	myself.models[4][3]=-12.5;
	myself.models[4][4]=null;
	myself.models[4][5]=0xff0000;

	for(var ii=0;ii<5;++ii)
	{
		myself.models[ii][4]=renderer.load_obj(myself.models[ii][0]);
		myself.models[ii][4].rotation.set(3*Math.PI/2,0,0);
		myself.models[ii][4].position.set(myself.models[ii][1],myself.models[ii][2],myself.models[ii][3]);
		var scale=1;
		myself.models[ii][4].scale.set(scale,scale,scale);
		myself.models[ii][4].set_color(myself.models[ii][5]);
	}

	myself.models[4][4].set_parent(myself.models[2][4]);
	myself.models[4][4].rotation.set(0,0,0);
	myself.models[4][4].position.set(myself.models[4][1]-myself.models[2][1],
		myself.models[4][3]-myself.models[2][3],myself.models[4][2]-myself.models[2][2]);

	myself.set_nod=function(value)
	{
		var rad_value=-(value-40)*Math.PI/180;
		myself.models[4][4].rotation.set(rad_value,myself.models[4][4].rotation.y,
			myself.models[4][4].rotation.z);
	};

	myself.set_twist=function(value)
	{
		var rad_value=(value-90)*Math.PI/180;
		myself.models[2][4].rotation.set(myself.models[2][4].rotation.x,
			myself.models[2][4].rotation.y,rad_value);
		myself.models[3][4].rotation.set(myself.models[3][4].rotation.x,
			myself.models[3][4].rotation.y,rad_value);
	};

	myself.set_position=function(x,y,z)
	{
		for(var ii=0;ii<4;++ii)
			myself.models[ii][4].position.set(x+myself.models[ii][1],
				y+myself.models[ii][2],z+myself.models[ii][3]);
	};
};