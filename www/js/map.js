/**
  Draw an onscreen map of the robot's position, orientation, and sensor data.
  
  Dr. Orion Lawlor, lawlor@alaska.edu, 2015-07-23 (Public Domain)
*/

function robot_map_t(div)
{
	this.div=div;
	this.div.title="Shows where the robot thinks it is in the world.  The grid lines are 1 meter apart.  The robot's right and left wheels leave red and purple tracks";
	var myself=this;
	this.need_redraw=true;
	this.renderer=new renderer_t(div,function() {myself.setup();}, function() {return myself.loop();} );
	if(!this.renderer.setup()) {
		var p=document.createElement("p");
		p.innerHTML="<p>WebGl seems to be disabled: <a target=_blank href=https://get.webgl.org>Click here to test</a><br> <u>If disabled, Try the following steps:</u></p> "
		p.innerHTML+="<p><b>Firefox:</b> Go to about:config in your address bar,search for webgl and check if webgl.disabled is true <br> No Luck? <a target=_blank href=https://support.mozilla.com/en-US/kb/how-do-i-upgrade-my-graphics-drivers>Help Page</a></p>";
		p.innerHTML+="<p><b>Chrome:</b> <ol><li><b>Is hardware acceleration enabled?</b> Type chrome://settings, show advanced settings -> under system > check Use hardware acceleration when available</li><b><li>Check WebGl:</b> Type chrome://flags into the address bar and confirm that Disable WebGl is gray</li></ol></p>";
		p.innerHTML+= "<p><b>Safari:</b> <ol><li>Go to Safari's Preferences</li><li>Select the Advanced tab</li><li>Check the Show Develop menu in menu bar checkbox</li><li>In the Develop menu, check Enable WebGl</li><li>Confused?<a target=_blank href=http://voicesofaliveness.net/webgl>Pictures</a></li></ol></p>";
		div.appendChild(p);
		this.renderer=null;
	}
}

robot_map_t.prototype.setup=function() {
	if (this.renderer===null) return;
	this.renderer.set_size(this.div.offsetWidth,this.div.offsetWidth); // FIXME resize this

	// Add grid
	var grid_cells=100;
	var per_cell=1000; // one meter cells (in mm)
	this.grid=this.renderer.create_grid(per_cell,grid_cells,grid_cells,20);
	this.grid.rotation.x=0;

	// Add light source
	var size=100000;
	var intensity=0.8;
	var light=new this.renderer.create_light(intensity,
		new THREE.Vector3(-size/2,-size/2,+size));
	
	// FIXME: add 2D room overlay

	// Add a robot
	this.mapRobot=new roomba_t(this.renderer,null);

	// Set initial camera
	this.renderer.controls.center.set(0,0,0); // robot?
	this.renderer.controls.object.position.set(0,-1200,1400);
	console.log("Set up renderer");
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
	var need_redraw=this.need_redraw;
	this.need_redraw=false;
	return need_redraw;
}


