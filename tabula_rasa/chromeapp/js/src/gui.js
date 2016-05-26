function gui_t(div)
{
	if(!div)
		return null;

	this.div=div;
	this.el=new_div(this.div);
	maximize(this.el);

	this.layout=new_table(this.el,2,1);
	maximize(this.layout);

	this.top_bar=new_table(this.layout.rows[0].cells[0],1,2);
	this.layout.rows[1].cells[0].style.height="100%";

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
		this.top_bar.rows[0].cells[0],
		function(message){_this.status_viewer.show(message);},
		function(robot)
		{
			console.log("callback!");
			_this.connection.gui_robot(robot);
		}
	);
	this.connection.on_name_set=function(robot)
	{
		console.log("name set");
		_this.name.reload(robot);
	};
	this.connection.load();

	this.serial_selector=new serial_selector_t
	(
		this.top_bar.rows[0].cells[1],
		function(port_name){_this.connection.gui_connect(port_name);},
		function(port_name){_this.connection.gui_disconnect(port_name);},
		function(){return (_this.name.get_robot().school!=null&&_this.name.get_robot().name!=null);}
	);

	this.status_viewer=new status_viewer_t(this.layout.rows[1].cells[0]);
}

gui_t.prototype.destroy=function()
{
	this.connection.destroy();
	this.name.destroy();
	this.status_viewer.destroy();
	this.div.removedChild(this.el);
}

chrome.app.runtime.onLaunched.addListener(function(){
	chrome.app.window.create('gruveo.html');
});


