/**
  Draw an onscreen map of the robot's position, orientation, and sensor data.
  
  Dr. Orion Lawlor, lawlor@alaska.edu, 2015-07-23 (Public Domain)
*/

function robot_map_t(div)
{
	this.div=div;
	this.div.title="Shows where the robot thinks it is in the world.  The grid lines are 1 meter apart.  The robot's right and left wheels leave red and purple tracks";
	var myself=this;
	this.renderer=new renderer_t(div,function() {myself.setup();}, function() {myself.loop();} );
	if(!this.renderer.setup()) {
		var p=document.createElement("p");
		p.innerHTML="Is WebGL enabled?";
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

robot_map_t.prototype.refresh=function(sensors) {
	if (this.renderer===null) return;
	this.sensors=sensors;
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
}


