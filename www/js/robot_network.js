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
				if(valid_robot(_this.robot))
				{
					superstar_set_and_get_multiple
					(
						_this.robot,
						"pilot",
						_this.pilot,
						["active_experiment","config","frontendStatus"],
						function(json)
						{
							_this.active_experiment=json[0];
							_this.config=json[1];
							_this.frontendStatus=json[2];
						}
					);
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

					superstar_generic(_this.robot,"chat","?get",function(str)
					{
						_this.chat=str;
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
	superstar_getnext(this.robot,"sensors",function(json)
	{
		_this.sensors=json;
	});
}
