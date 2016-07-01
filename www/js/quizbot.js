function quizbot_t(renderer)
{
	var myself=this;

	myself.nod=40;
	myself.nod_target=40;

	myself.twist=90;
	myself.twist_target=90;

	myself.models=new Array();

	for(var ii=0;ii<5;++ii)
		myself.models[ii]=new Array();

	myself.models[0][0]="/models/quizbot/base.obj";
	myself.models[0][1]=0;
	myself.models[0][2]=0;
	myself.models[0][3]=0;
	myself.models[0][4]=null;
	myself.models[0][5]=0xcc0000;

	myself.models[1][0]="/models/quizbot/base_servo.obj";
	myself.models[1][1]=0;
	myself.models[1][2]=0;
	myself.models[1][3]=0;
	myself.models[1][4]=null;
	myself.models[1][5]=0x00cc00;

	myself.models[2][0]="/models/quizbot/middle.obj";
	myself.models[2][1]=0.38;
	myself.models[2][2]=37;
	myself.models[2][3]=-12.38;
	myself.models[2][4]=null;
	myself.models[2][5]=0xcc0000;

	myself.models[3][0]="/models/quizbot/middle_servo.obj";
	myself.models[3][1]=0.38;
	myself.models[3][2]=37;
	myself.models[3][3]=-12.38;
	myself.models[3][4]=null;
	myself.models[3][5]=0x00cc00;

	myself.models[4][0]="/models/quizbot/head.obj";
	myself.models[4][1]=-0.17;
	myself.models[4][2]=68.5;
	myself.models[4][3]=-12.5;
	myself.models[4][4]=null;
	myself.models[4][5]=0xcc0000;

	for(var ii=0;ii<5;++ii)
	{
		myself.models[ii][4]=renderer.load_obj(myself.models[ii][0]);
		myself.models[ii][4].rotation.set(3*Math.PI/2,0,0);
		myself.models[ii][4].position.set(myself.models[ii][1],myself.models[ii][2],myself.models[ii][3]);
		var scale=1;
		myself.models[ii][4].scale.set(scale,scale,scale);
		myself.models[ii][4].set_color(myself.models[ii][5]);
		myself.models[ii][4].castShadow=true;
		myself.models[ii][4].receiveShadow=true;
	}

	myself.models[4][4].set_parent(myself.models[2][4]);
	myself.models[4][4].rotation.set(0,0,0);
	myself.models[4][4].position.set(myself.models[4][1]-myself.models[2][1],
		myself.models[4][3]-myself.models[2][3],myself.models[4][2]-myself.models[2][2]);

	myself.set_nod=function(value)
	{
		myself.nod=value;
	};

	myself.set_nod_target=function(value)
	{
		myself.nod_target=value;
	};

	myself.set_twist=function(value)
	{
		myself.twist=value;
	};

	myself.set_twist_target=function(value)
	{
		myself.twist_target=value;
	};

	myself.set_position=function(x,y,z)
	{
		for(var ii=0;ii<4;++ii)
			myself.models[ii][4].position.set(x+myself.models[ii][1],
				y+myself.models[ii][2],z+myself.models[ii][3]);
	};

	myself.loop=function(dt)
	{
		if(dt)
		{
			var rad_nod=-(myself.nod-40)*Math.PI/180;
			myself.models[4][4].rotation.set(rad_nod,myself.models[4][4].rotation.y,
				myself.models[4][4].rotation.z);

			var rad_twist=-(myself.twist-90)*Math.PI/180;
			myself.models[2][4].rotation.set(myself.models[2][4].rotation.x,
				myself.models[2][4].rotation.y,rad_twist);
			myself.models[3][4].rotation.set(myself.models[3][4].rotation.x,
				myself.models[3][4].rotation.y,rad_twist);

			var speed=200; // degrees/second slew rate

			if(Math.floor(myself.nod)<myself.nod_target)
				myself.nod+=speed*dt;
			if(Math.floor(myself.nod)>myself.nod_target)
				myself.nod-=speed*dt;

			if(myself.nod<0)
				myself.nod=0;
			if(myself.nod>180)
				myself.nod=180;

			if(Math.floor(myself.twist)<myself.twist_target)
				myself.twist+=speed*dt;
			if(Math.floor(myself.twist)>myself.twist_target)
				myself.twist-=speed*dt;

			if(myself.twist<0)
				myself.twist=0;
			if(myself.twist>180)
				myself.twist=180;
		}
	};
};
