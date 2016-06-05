function gui_t(div)
{
	if(!div)
		return null;

	this.main_div=document.createElement("div");
	maximize(this.main_div);

	this.gruveo_div=document.createElement("div");
	maximize(this.gruveo_div);
	this.gruveo_div.style.overflow="hidden";
	this.gruveo=document.getElementById("gruveo");
	this.gruveo_div.appendChild(gruveo);
	this.gruveo.addEventListener('permissionrequest',
	function(e)
	{
		if(e.permission==='media')
			e.request.allow();
	});

	this.superstar_errored=false;

	var _this=this;

	this.connection=new connection_t
	(
		function(message){_this.status_viewer.show(message);},
		function(){_this.name.disabled=false;_this.serial_selector.disconnect();},
		function(){_this.name.disabled=true;}
	);

	this.name=new name_t
	(
		this.main_div,
		function(message){_this.status_viewer.show(message);},
		function(robot)
		{
			_this.connection.gui_robot(robot);
			url="https://gruveo.com/";
			robot_url="";
			if(robot&&robot.year&&robot.school&&robot.name)
				robot_url=robot.year+robot.school+robot.name;
			url+=encodeURIComponent(robot_url.replace(/_/g,''));
			_this.gruveo.src=url;
		}

	);
	this.connection.on_name_set=function(robot){_this.name.reload(robot);};
	this.connection.load();

	this.serial_selector=new serial_selector_t
	(
		this.name.el,
		function(port_name){_this.connection.gui_connect(port_name);},
		function(port_name){_this.connection.gui_disconnect(port_name);},
		function()
		{
			return (_this.name.get_robot().school!=null
					&&_this.name.get_robot().name!=null
					&&_this.name.get_robot().year!=null);
		}
	);

	this.pilot_status=new pilot_status_t(this);
	this.status_viewer=new status_viewer_t(this.main_div);

	this.state_side_bar=document.createElement("div");

	$('#content').w2layout
	({
		name:'app_layout',
		panels:
		[
			{type:'left',resizable:true,content:this.gruveo_div,size:"65%"},
			{type:'main',resizable:true,content:this.name.el,},
			{type:'preview',resizable:true,content:this.status_viewer.el,size:"80%"}
		]
	});
}

gui_t.prototype.destroy=function()
{
	this.connection.destroy();
	this.name.destroy();
	this.status_viewer.destroy();
	this.div.removedChild(this.el);
}



