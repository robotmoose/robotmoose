/**
  Draw an onscreen map of the robot's position, orientation, and sensor data.

  Dr. Orion Lawlor, lawlor@alaska.edu, 2015-07-23 (Public Domain)
*/

function robot_map_t(div, modal_div, robot)
{
	this.div=div;
	this.div.title="Shows where the robot thinks it is in the world.  The grid lines are 1 meter apart.  The robot's right and left wheels leave red and purple tracks";
	this.modal_div = modal_div;


	if (robot) this.robot = robot;

	var myself=this;

	this.offset_x = 0;
	this.offset_y = 0;
	this.offset_angle = 0;

	this.element=document.createElement("div");
	this.div.appendChild(this.element);
	this.element.title="Select map image overlay";

	this.controls_div=document.createElement("div");
	this.element.appendChild(this.controls_div);
	this.controls_div.className="form-horizontal";

	//Map dropdown...
	this.map_select=document.createElement("select");
	this.controls_div.appendChild(this.map_select);
	this.change_map_image=function()
	{
		myself.last_map_select=myself.map_select.value;
		myself.load_button_pressed_m();
	};
	this.map_select.addEventListener("change",this.change_map_image);
	this.map_select.className="form-control";
	this.map_select.style.marginBottom="10px";

	// ********** Add maps here:
		var option=document.createElement("option");
		option.text="No Map Image";
		var opt0 = {};
		opt0.path="";
		opt0.width=10;
		opt0.height=10;
		option.value=JSON.stringify(opt0);
		//option.value="";
		this.map_select.appendChild(option);

		var option1=document.createElement("option");
		option1.text="Map 1";
		var opt1 = {};
		opt1.path="maps/map1.jpg";
		opt1.width=3;
		opt1.height=10;
		option1.value=JSON.stringify(opt1);
		//option1.value="maps/map1.jpg";
		this.map_select.appendChild(option1);

		var option2=document.createElement("option");
		option2.text="Map 2";
		var opt2 = {};
		opt2.path="maps/map2.jpg";
		opt2.width=10;
		opt2.height=10;
		option2.value=JSON.stringify(opt2);
		//option2.value="maps/map2.jpg"
		this.map_select.appendChild(option2);

		var option3=document.createElement("option");
		option3.text="Map 3";
		var opt3 = {};
		opt3.path="maps/map3.jpg";
		opt3.width=10;
		opt3.height=10;
		option3.value=JSON.stringify(opt3);
		//option3.value="maps/map3.jpg";
		this.map_select.appendChild(option3);


	//Upload button...
	this.upload_map_button=document.createElement("input");
	this.controls_div.appendChild(this.upload_map_button);
	this.upload_map_button.className="btn btn-primary";
	this.upload_map_button.style.marginBottom="10px";
	this.upload_map_button.style.width = "100%";
	this.upload_map_button.disabled=false;
	this.upload_map_button.type="button";
	this.upload_map_button.value="Upload Map Image";
	this.upload_map_button.title="Click here to upload a map image";
	this.upload_map_button.addEventListener("click",function(event)
	{
		myself.upload_map_button_pressed_m();
	});




	this.map_display = document.createElement("div");
	this.map_display.title="Shows where the robot thinks it is in the world.  The grid lines are 1 meter apart.  The robot's right and left wheels leave red and purple tracks";

	this.need_redraw=true;
	this.renderer=new renderer_t(myself.map_display,function() {myself.setup();}, function() {return myself.loop();} );
	if(!this.renderer.setup()) {
		var p=document.createElement("p");
		p.innerHTML="<p>WebGl seems to be disabled: <a rel='noopener noreferrer' target='_blank' href='https://get.webgl.org'>Click here to test</a><br> <u>If disabled, Try the following steps:</u></p> "
		p.innerHTML+="<p><b>Firefox:</b> Go to about:config in your address bar,search for webgl and check if webgl.disabled is true <br> No Luck? <a rel='noopener noreferrer' target='_blank' href='https://support.mozilla.com/en-US/kb/how-do-i-upgrade-my-graphics-drivers'>Help Page</a></p>";
		p.innerHTML+="<p><b>Chrome:</b> <ol><li><b>Is hardware acceleration enabled?</b> Type chrome://settings, show advanced settings -> under system > check Use hardware acceleration when available</li><b><li>Check WebGl:</b> Type chrome://flags into the address bar and confirm that Disable WebGl is gray</li></ol></p>";
		p.innerHTML+= "<p><b>Safari:</b> <ol><li>Go to Safari's Preferences</li><li>Select the Advanced tab</li><li>Check the Show Develop menu in menu bar checkbox</li><li>In the Develop menu, check Enable WebGl</li><li>Confused?<a rel='noopener noreferrer' target='_blank' href='http://voicesofaliveness.net/webgl'>Pictures</a></li></ol></p>";
		div.appendChild(p);
		this.renderer=null;
	}

	this.div.appendChild(myself.map_display);

}

robot_map_t.prototype.setup=function(texture_file, width, height) {
	if (this.renderer===null) return;

	var myself=this;

	this.resize_map();

	// Add grid
	var grid_cells=10;
	if (!width) width = 10;
	if (!height) height = 10;
	var per_cell=1000; // one meter cells (in mm)
	this.grid=this.renderer.create_grid(per_cell,width,height,20,texture_file);
	this.grid.rotation.x=0;


	// Add light source
	var size=100000;
	var intensity=0.8;
	this.light=this.renderer.create_light(intensity,
		new THREE.Vector3(-size/2,-size/2,+size));

	// FIXME: add 2D room overlay

	// Add a robot
	this.mapRobot=new roomba_t(this.renderer,null);

	// Set initial camera
	this.renderer.controls.center.set(0,0,0); // robot?
	this.renderer.controls.object.position.set(0,-1200,1400);

}



robot_map_t.prototype.resize_map=function()
{
	var edge_offset = 20;
	var rend_width = this.element.offsetWidth;
	var rend_height = this.div.offsetHeight-this.element.offsetHeight;
	this.renderer.set_size(rend_width,rend_height);

}


// Updated sensor data is available:
robot_map_t.prototype.refresh=function(sensors) {
	if (this.renderer===null) return;
	this.sensors=sensors;
	this.need_redraw=true;
}

robot_map_t.prototype.loop=function() {
	if (this.renderer===null) return;

	var sensors=this.sensors;
	if (!sensors || !sensors.location) return;

	// Convert angle from degrees to radians
	var angle_rad=sensors.location.angle*Math.PI/180.0;
	// Convert position from meters to mm (rendering units)

	var P=new vec3(sensors.location.x,sensors.location.y,0.0).te(1000.0);

	// Move onscreen robot there
	this.mapRobot.set_location(P,angle_rad);

	// Place the wheels (so wheel tracks work)
	this.mapRobot.wheel[0]=this.mapRobot.world_from_robot(150,+Math.PI*0.5);
	this.mapRobot.wheel[1]=this.mapRobot.world_from_robot(150,-Math.PI*0.5);

	// Check for obstacle sensors
	if (sensors.lidar) {
		if (sensors.lidar.change!=this.last_lidar_change) {
			this.mapRobot.draw_lidar(this.renderer,sensors.lidar);
			this.last_lidar_change=sensors.lidar.change;
		}
	}

	if (this.reset_tracks)
	{
		this.mapRobot.left_tracker.reset();
		this.mapRobot.right_tracker.reset();
		this.reset_tracks = false;
	}


	var need_redraw=this.need_redraw;
	this.need_redraw=false;
	return need_redraw;
}

robot_map_t.prototype.load_button_pressed_m=function()
{
	var myself=this;

	if (this.mapRobot)
	{
		this.mapRobot.model.destroy();
		this.mapRobot=null;
		this.reset_tracks = true;
	}


	var opt = JSON.parse(myself.last_map_select);
	this.robot.map_json=opt;


	this.make_new(opt.path, opt.width, opt.height);
	if (this.navigation)
	{
		this.navigation.status="done";
		this.navigation = new navigation_t(this.navigation.div, this.navigation.state_runner, this.robot)
	}

}






robot_map_t.prototype.upload_map_button_pressed_m=function()
{
	var myself = this;

	this.modal = new modal_uploadmap_t(myself.modal_div, function(src, width, height, title){myself.onupload(src, width, height, title)});
	this.modal.show();

}




robot_map_t.prototype.onupload=function(src, width, height, title)
{
	var myself = this;
		// add image to map options

		myself.uploaded_option=document.createElement("option");
		if (title) myself.uploaded_option.text=title;
		else myself.uploaded_option.text="Uploaded Map";
		var opt = {};
		opt.path=src; // image path
		opt.width=width; // width
		opt.height=height; // height
		myself.uploaded_option.value=JSON.stringify(opt);
		myself.map_select.appendChild(myself.uploaded_option);

		myself.modal.hide();

		myself.map_select.selectedIndex=myself.map_select.length - 1;
		myself.change_map_image();

}

robot_map_t.prototype.clean_up=function()
{
	var myself = this;

	if (this.renderer) this.renderer.destroy();
	this.renderer=null;

	if (this.grid)
	{
		this.grid.geometry.dispose();
		this.grid.material.dispose();
		if (this.grid.texture) this.grid.texture.dispose();
		this.grid = null;
	}

	if (this.map_display)
	{
	this.div.removeChild(myself.map_display);
	this.map_display="";
	}
}

robot_map_t.prototype.make_new=function(filename, width, height)
{
	var myself = this;

	this.clean_up();

	this.map_display = document.createElement("div");
	this.map_display.title="Shows where the robot thinks it is in the world.  The grid lines are 1 meter apart.  The robot's right and left wheels leave red and purple tracks";

	this.need_redraw=true;
	this.renderer=new renderer_t(myself.map_display,function() {myself.setup(filename, width, height);}, function() {return myself.loop();} );
	if(!this.renderer.setup()) {
		var p=document.createElement("p");
		p.innerHTML="<p>WebGl seems to be disabled: <a rel='noopener noreferrer' target='_blank' href='https://get.webgl.org'>Click here to test</a><br> <u>If disabled, Try the following steps:</u></p> "
		p.innerHTML+="<p><b>Firefox:</b> Go to about:config in your address bar,search for webgl and check if webgl.disabled is true <br> No Luck? <a rel='noopener noreferrer' target='_blank' href='https://support.mozilla.com/en-US/kb/how-do-i-upgrade-my-graphics-drivers'>Help Page</a></p>";
		p.innerHTML+="<p><b>Chrome:</b> <ol><li><b>Is hardware acceleration enabled?</b> Type chrome://settings, show advanced settings -> under system > check Use hardware acceleration when available</li><b><li>Check WebGl:</b> Type chrome://flags into the address bar and confirm that Disable WebGl is gray</li></ol></p>";
		p.innerHTML+= "<p><b>Safari:</b> <ol><li>Go to Safari's Preferences</li><li>Select the Advanced tab</li><li>Check the Show Develop menu in menu bar checkbox</li><li>In the Develop menu, check Enable WebGl</li><li>Confused?<a rel='noopener noreferrer' target='_blank' href='http://voicesofaliveness.net/webgl'>Pictures</a></li></ol></p>";
		div.appendChild(p);
		this.renderer=null;
	}

	this.div.appendChild(myself.map_display);

}


