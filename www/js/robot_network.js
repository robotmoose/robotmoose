function robot_network_t()
{
	var _this=this;
	this.robot=null;
	this.pilot={};
	this.old_pilot={};
	this.intervals=
	[
		{
			int:null,
			func:function()
			{
				if(valid_robot(_this.robot))
				{
					if(JSON.stringify(_this.pilot)!=JSON.stringify(_this.old_pilot))
					{
						superstar_set(_this.robot,"pilot",_this.pilot);
						_this.old_pilot=JSON.parse(JSON.stringify(_this.pilot));
					}
					superstar_get(_this.robot,"active_experiment",function(data)
					{
						_this.active_experiment=data;
					});
					superstar_get(_this.robot,"config",function(data)
					{
						_this.config=data;
					});
					superstar_get(_this.robot,"frontendStatus",function(data)
					{
						_this.frontendStatus=data;
					});
				}
			},
			ms:300
		},
		{
			int:null,
			func:function()
			{
				if(valid_robot(_this.robot))
				{
					superstar_sub(_this.robot,"experiments",function(json)
					{
						_this.experiments=json;

					});

					superstar_get(_this.robot,"chat",function(data)
					{
						_this.chat=data;
					});
				}
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

	robot_set_superstar(robot.superstar);
	superstar_get(_this.robot,"sensors",function(data)
	{
		_this.sensors=data;
	});

	var func=function()
	{
		superstar.get_next(robot_to_starpath(_this.robot)+"sensors",function(json)
		{
			_this.sensors=json;
			func();
		},
		function()
		{
			func();
		});
	}
	func();
}
