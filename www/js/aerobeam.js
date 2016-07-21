function aerobeam_t(renderer)
{
	var myself=this;

	myself.rad_throttle=0;
	myself.throttle=40;
	myself.throttle_target=40;

	myself.arm=90;
	myself.arm_target=90;

	myself.models=new Array();

	for(var ii=0;ii<5;++ii)
		myself.models[ii]=new Array();

	myself.models[0][0]=url_path+"models/aerobeam/base.obj";
	myself.models[0][1]=0;
	myself.models[0][2]=0;
	myself.models[0][3]=0;
	myself.models[0][4]=null;
	myself.models[0][5]=0xcc0000;

	myself.models[1][0]=url_path+"models/aerobeam/arm.obj";
	myself.models[1][1]=0;
	myself.models[1][2]=0;
	myself.models[1][3]=0;
	myself.models[1][4]=null;
	myself.models[1][5]=0xaaaaaa;

	myself.models[2][0]=url_path+"models/aerobeam/prop.obj";
	myself.models[2][1]=0;
	myself.models[2][2]=25.9;
	myself.models[2][3]=225.91;
	myself.models[2][4]=null;
	myself.models[2][5]=0xcc0000;

	for(var ii=0;ii<3;++ii)
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

	myself.models[2][4].set_parent(myself.models[1][4]);
	myself.models[2][4].rotation.set(0,0,0);
	myself.models[2][4].position.set(myself.models[2][1]-myself.models[1][1],
		myself.models[2][3]-myself.models[1][3],myself.models[2][2]-myself.models[1][2]);

	myself.set_throttle=function(value)
	{
		myself.throttle=value;
	};

	myself.set_throttle_target=function(value)
	{
		myself.throttle_target=value;
	};

	myself.set_arm=function(value)
	{
		myself.arm=value;
	};

	myself.set_arm_target=function(value)
	{
		myself.arm_target=value;
	};

	myself.set_position=function(x,y,z)
	{
		for(var ii=0;ii<2;++ii)
			myself.models[ii][4].position.set(x+myself.models[ii][1],
				y+myself.models[ii][2],z+myself.models[ii][3]);
	};

	myself.loop=function(dt)
	{
		if(dt)
		{
			myself.rad_throttle+=myself.throttle*dt;
			myself.models[2][4].rotation.set(myself.models[2][4].rotation.x,myself.models[2][4].rotation.y,
				myself.rad_throttle);

			var rad_arm=Math.PI+(myself.arm+90)*Math.PI/180;
			myself.models[1][4].rotation.set(rad_arm,
				myself.models[1][4].rotation.y,myself.models[1][4].rotation.z);

			var speed=200; // degrees/second slew rate

			if(Math.floor(myself.throttle)<myself.throttle_target)
				myself.throttle+=speed*dt;
			if(Math.floor(myself.throttle)>myself.throttle_target)
				myself.throttle-=speed*dt;

			if(myself.throttle<0)
				myself.throttle=0;
			if(myself.throttle>100)
				myself.throttle=100;

			while(myself.rad_throttle>=2*Math.PI)
				myself.rad_throttle-=2*Math.PI;
			while(myself.rad_throttle<0)
				myself.rad_throttle+=2*Math.PI;

			if(Math.floor(myself.arm)<myself.arm_target)
				myself.arm+=speed*dt;
			if(Math.floor(myself.arm)>myself.arm_target)
				myself.arm-=speed*dt;

			if(myself.arm<-60)
				myself.arm=-60;
			if(myself.arm>60)
				myself.arm=60;
		}
	};
};
