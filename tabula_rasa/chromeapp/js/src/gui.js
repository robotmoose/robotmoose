function gui_t(div)
{
	if(!div)
		return null;

	this.main_div=document.createElement("div");
	maximize(this.main_div);
	this.main_div.style.paddingRight="1px";

	var _this=this;

	this.gruveo_div=document.createElement("div");
	maximize(this.gruveo_div);
	this.gruveo_div.style.overflow="hidden";
	this.gruveo=document.getElementById("gruveo");
	this.gruveo_div.appendChild(gruveo);
	this.gruveo.addEventListener("permissionrequest",function(evt)
	{
		if(evt.permission==="media"||evt.permission==="fullscreen")
			evt.request.allow();
	});
	this.last_uuid=null;

	this.superstar_errored=false;

	var _this=this;

	this.connection=new connection_t
	(
		function(message){_this.status_viewer.show(message);},
		function()
		{
			_this.name.disabled=false;
			_this.auth_input.enable();
			_this.serial_selector.disconnect();
			_this.sound_player.disconnect();
			_this.pilot_status.reset();
		},
		function()
		{
			_this.name.disabled=true;
			_this.auth_input.disable();
			_this.sound_player.connect();
		}
	);

	this.chat_div=document.createElement("div");
	maximize(this.chat_div);
	this.chat_div.style.height="100%";
	this.chat=new chatter_t(this.chat_div,20,"Caretaker");

	this.version_div=document.createElement(this.version_div);
	this.main_div.appendChild(this.version_div);
	var manifest=chrome.runtime.getManifest();
	this.version_div.appendChild(document.createTextNode('Version: '+manifest.name+' '+manifest.version));

	this.name=new name_t
	(
		this.main_div,
		function(message){_this.status_viewer.show(message);},
		function(robot)
		{
			_this.connection.gui_robot(robot);
			_this.chat.set_robot(robot);
		}
	);

	this.auth_input=new auth_input_t(this.main_div,function(auth)
	{
		_this.connection.gui_auth(auth);
		var pilot=null;
		//if(_this.pilot_status)
		//	pilot=_this.pilot_status.current_pilot;
		_this.load_gruveo(_this.connection.robot,pilot);
		_this.chat.set_robot(_this.connection.robot);
	});

	this.serial_selector=new serial_selector_t
	(
		this.main_div,
		function(port_name)
		{
			_this.connection.gui_connect(port_name);
		},
		function(port_name)
		{
			_this.connection.gui_disconnect(port_name);
		},
		function()
		{
			return valid_robot(_this.name.get_robot());
		}
	);

	//this.media_selector=new media_selector_t(this.main_div, this.gruveo);
	this.connection.on_name_set=function(robot)
	{
		_this.name.load(robot);
		_this.sound_player.load(robot);
	};

	this.connection.on_auth_error=function(err)
	{
		chrome.runtime.sendMessage({message:err});
	}
	this.connection.load();

	this.status_viewer=new status_viewer_t(this.main_div);

	this.state_side_bar=document.createElement("div");

	$("#content").w2layout
	({
		name:"app_layout",
		panels:
		[
			{type:"left",resizable:true,content:this.gruveo_div,size:"60%"},
			{type:"main",resizable:true,content:this.main_div},
			{type:"preview",resizable:true,content:this.status_viewer.el,size:"40%"},
			{type:"bottom",resizable:true,content:this.chat_div,size:"20%"}
		]
	});

	this.fullscreen_button=document.createElement("input");
	this.serial_selector.el.appendChild(this.fullscreen_button);
	this.fullscreen_button.type="button";
	var fullscreen_text="Fullscreen";
	var not_fullscreen_text="Exit Fullscreen";
	this.fullscreen_button.value=fullscreen_text;
	this.fullscreen_button.style.width="50%";
	this.fullscreen_button.onclick=function()
	{
		window.open("https://google.com/");
		if(document.webkitIsFullScreen)
			document.webkitExitFullscreen();
		else
			document.body.webkitRequestFullscreen();
	}
	this.fullscreen_button.addEventListener("permissionrequest",function(evt)
	{
		if(evt.permission==="fullscreen")
			evt.request.allow();
	});

	document.addEventListener("webkitfullscreenchange",function()
	{
		if(document.webkitIsFullScreen)
			_this.fullscreen_button.value=not_fullscreen_text;
		else
			_this.fullscreen_button.value=fullscreen_text;
	});

	this.sound_player=new sound_player_t(this.name);

	this.pilot_checkmark=new checkmark_t(this.main_div);
	this.pilot_status_text=this.pilot_checkmark.getElement();
	this.pilot_status_text.align="center";
	this.pilot_status_text.style.fontSize="large";
	this.pilot_status_text.innerHTML="Pilots connected (0)";
	this.main_div.appendChild(document.createElement("br"));

	this.pilot_status=new pilot_status_t(this.connection,this.pilot_checkmark,
		function() {
			_this.connection.pilot_connected=true;
		},
		function() {
			_this.connection.pilot_connected=false;
		}
	);
	this.pilot_status.onchange=function(num)
	{
		_this.pilot_status_text.innerHTML="Pilots connected ("+num+")";
	}
	this.pilot_status.onvideohangup=function()
	{
		_this.load_gruveo();
	}
	this.pilot_status.onvideocall=function()
	{
		_this.load_gruveo(_this.connection.robot);
	}
}

gui_t.prototype.destroy=function()
{
	this.connection.destroy();
	this.name.destroy();
	this.status_viewer.destroy();
	this.div.removedChild(this.el);
}

gui_t.prototype.load_gruveo=function(robot)
{
	var url="https://gruveo.com/";
	var robot_url="";
	if(!robot)
		robot={};
	if(valid_robot(robot))
	{
		robot_url=superstar.superstar+robot.year+robot.school+robot.name;
		robot_url=robot_url.replace(/[^A-Za-z0-9\s!?]/g,'');
		if(!robot.auth)
			robot.auth="";
		robot_url=CryptoJS.HmacSHA256(robot_url,robot.auth).toString(CryptoJS.enc.Hex);
	}
	url+=encodeURIComponent(robot_url);
	this.gruveo.src=url;
}
