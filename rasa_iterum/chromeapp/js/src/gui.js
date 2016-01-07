function gui_t(div)
{
	if(!div)
		return null;

	this.div=div;
	this.el=document.createElement("div");
	this.div.appendChild(this.el);
	this.superstar_errored=false;

	var _this=this;

	this.connection=new connection_t
	(
		this.el,
		function(message){_this.status_viewer.show(message);},
		function(){_this.serial_selector.disconnect();}
	);

	this.top_bar=document.createElement("div");
	this.el.appendChild(this.top_bar);

	this.serial_selector=new serial_selector_t
	(
		this.top_bar,
		function(port_name){_this.connection.gui_connect(port_name);},
		function(port_name){_this.connection.gui_disconnect(port_name);}
	);
	this.serial_selector.el.style.float="left";

	this.name=new name_t(this.top_bar);
	this.name.el.style.float="left";

	this.status_viewer=new status_viewer_t(this.el);

/*
	this.download_config();
	setInterval(function(){_this.download_config();},2000);
	this.download_write();
*/
}

gui_t.prototype.destroy=function()
{
	this.status_viewer.destroy();
}

