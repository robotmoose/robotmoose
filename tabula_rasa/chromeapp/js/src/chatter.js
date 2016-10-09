function chatter_t(div,maxlines,handle)
{
	if(!div)
		return null;

	this.div=div;
	this.robot=null;
	this.maxlines=maxlines;
	this.handle=handle;
	var _this=this;

	if(!this.maxlines)
		this.maxlines=10;

	this.div.style.overflow="hidden";

	this.el=document.createElement("div");
	this.div.appendChild(this.el);
	this.el.style.height="100%";
	this.last_chat_hash=null;

	this.table=new_table(this.el,2,1);
	maximize(this.table);
	this.table.rows[1].style.height="0%";

	this.history=document.createElement("textarea");
	this.history.className="form-control";
	this.history.readOnly=true;
	this.history.spellcheck=false;
	this.history.style.resize="none";
	this.history.style.overflow="scroll";
	maximize(this.history);
	this.table.rows[0].cells[0].appendChild(this.history);

	this.el.appendChild(document.createElement("br"));

	this.input=document.createElement("input");
	this.table.rows[1].cells[0].appendChild(this.input);
	this.input.type="text";
	this.input.className="form-control";
	this.input.style.width="100%";
	this.input.spellcheck=false;
	this.input.onkeydown=function(event)
	{
		if(event.keyCode==13)
		{
			_this.chat(this.value);
			this.value="";
		}
	};

	this.prev_data="";
	this.comet=null;
}

chatter_t.prototype.destroy=function()
{
	if(this.comet)
	{
		this.comet.destroy();
		this.comet=null;
	}
	try
	{
		this.div.removeChild(this.el);
	}
	catch(error)
	{}
	this.div=this.robot=null;
}

chatter_t.prototype.chat=function(msg)
{
	if(valid_robot(this.robot))
	{
		var _this=this;
		var obj=
			{
				handle:this.handle,
				message:msg,
				time:(new Date()).getTime()
			};
		superstar_append(this.robot,"chat",obj,_this.maxlines);
	}
}

chatter_t.prototype.set_robot=function(robot)
{
	if(JSON.stringify(robot)!=JSON.stringify(this.robot))
	{
		this.robot=robot;
		var _this=this;

		if(this.comet)
		{
			this.comet.destroy();
			this.comet=null;
		}

		var service=function(data)
		{
			var chat="";
			for(let key in data)
			{
				try
				{
					var obj=data[key];
					if(obj.handle&&obj.message&&obj.time)
					{
						var time=moment(obj.time).fromNow();
						chat+=obj.handle+" ("+time+"):\t"+obj.message+"\n";
					}
				}
				catch(error)
				{}
			}

			_this.history.value=chat;
			_this.history.scrollTop=_this.history.scrollHeight;
			request();
		};

		var request=function()
		{
			_this.comet=superstar.get_next(robot_to_starpath(_this.robot)+"chat",_this.last_chat_hash,function(data)
			{
				_this.last_chat_hash=data.hash;
				service(data.value);
			},
			function()
			{
				setTimeout(function(){request();},1000);
			});
		};

		request();
	}
}