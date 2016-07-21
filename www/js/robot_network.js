function robot_network_t()
{
	var _this=this;
	this.robot=null;
	this.pilot={};
	this.intervals=
	[
		{
			int:null,
			func:function()
			{
				robot_get(_this.robot,"active_experiment",function(data)
				{
					_this.active_experiment=data;
				});
				robot_get(_this.robot,"config",function(data)
				{
					_this.config=data;
				});
				robot_get(_this.robot,"frontendStatus",function(data)
				{
					_this.frontendStatus=data;
				});
				robot_get(_this.robot,"sensors",function(data)
				{
					_this.sensors=data;
				});
				robot_set(_this.robot,"pilot",_this.pilot);
				superstar.flush();
			},
			ms:300
		},
		{
			int:null,
			func:function()
			{
				robot_sub(_this.robot,"experiments",function(data)
				{
					_this.experiments=data;

				});
				robot_set(_this.robot,"chat",function(data)
				{
					_this.chat=data;
				});
			},
			ms:1000
		}
	];
	for(var key in this.intervals)
	{
		this.intervals[key].int=
			setInterval(this.intervals[key].func,this.intervals[key].ms);
		this.intervals[key].func();
	}
}

robot_network_t.prototype.destroy=function()
{
	for(var key in this.intervals)
		try
		{
			if(this.intervals[key].int)
				clearInterval(this.intervals[key].int);
			this.intervals[key].int=null;
		}
		catch(error)
		{}
}

robot_network_t.prototype.set_robot=function(robot)
{
	this.robot=robot;
	var _this=this;
}

function robot_get(robot,path,onsuccess,onfail)
{
	if(valid_robot(robot))
	{
		var starpath="/robots/"+robot.year+"/"+robot.school+"/"+robot.name+"/"+path;
		superstar.get(starpath,onsuccess,onfail);
	}
}

function robot_set(robot,path,value,onsuccess,onfail)
{
	if(valid_robot(robot))
	{
		var starpath="/robots/"+robot.year+"/"+robot.school+"/"+robot.name+"/"+path;
		superstar.set(starpath,value,robot.auth,onsuccess,onfail);
	}
}

function robot_sub(robot,path,onsuccess,onfail)
{
	if(valid_robot(robot))
	{
		var starpath="/robots/"+robot.year+"/"+robot.school+"/"+robot.name+"/"+path;
		superstar.sub(starpath,onsuccess,onfail);
	}
}

function robot_push(robot,path,value,length,onsuccess,onfail)
{
	if(valid_robot(robot))
	{
		var starpath="/robots/"+robot.year+"/"+robot.school+"/"+robot.name+"/"+path;
		superstar.push(starpath,value,length,robot.auth,onsuccess,onfail);
	}
}