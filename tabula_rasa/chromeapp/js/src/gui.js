function gui_t(div)
{
	if(!div)
		return null;

	var status_view_div=document.createElement("div");
	maximize(status_view_div);
	var gruveo_div=document.createElement("div");
	maximize(gruveo_div);

	$('#content').w2layout
	({
		name: 'app_layout',
		panels:
		[
			{type:'main',resizable:true,content:gruveo_div},
			{type:'right',resizable:true,content:status_view_div,size:"25%"}
		]
	});

	this.gruveo=document.getElementById("gruveo");
	gruveo_div.appendChild(gruveo);

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
		status_view_div,
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
		status_view_div,
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
	this.status_viewer=new status_viewer_t(status_view_div);

}

gui_t.prototype.destroy=function()
{
	this.connection.destroy();
	this.name.destroy();
	this.status_viewer.destroy();
	this.div.removedChild(this.el);
}



