/**
  Draw an onscreen map of the robot's position, orientation, and sensor data.
  
  Dr. Orion Lawlor, lawlor@alaska.edu, 2015-07-23 (Public Domain)
*/

function robot_map_t(div)
{
	this.div=div;
	//this.div.title="Shows where the robot thinks it is in the world.  The grid lines are 1 meter apart.  The robot's right and left wheels leave red and purple tracks";
	var myself=this;
		
	this.element=document.createElement("div");
	this.element.title="Select map image overlay";
	this.drag_list_div=document.createElement("div");
	this.drag_list=new drag_list_t(this.drag_list_div);
	this.controls_div=document.createElement("div");
	this.map_select=
	{
		div:document.createElement("div"),
		drop:document.createElement("select")
	};
	this.load_button=document.createElement("input");

	this.entries=[];

	if(!this.drag_list)
	{
		this.div=null;
		this.element=null;
	}

	this.element.style.width="auto";
	this.element.style.minWidth=640;
	this.div.appendChild(this.element);

	this.element.appendChild(this.controls_div);
	this.element.appendChild(document.createElement("br"));

	this.element.appendChild(this.drag_list_div);

	this.map_select.div.className="form-group";
	this.map_select.div.style.float="left";
	this.controls_div.appendChild(this.map_select.div);

	this.map_select.div.appendChild(this.map_select.drop);
	this.map_select.drop.onchange=function()
	{
		myself.last_map_select=myself.map_select.drop.value;
		myself.load_button_pressed_m();
	};
	this.map_select.drop.className="form-control";
	
	
	// ********** Add maps here:
	
	var option=document.createElement("option");
	option.text="No Map Image";
	option.value="none";
	this.map_select.drop.appendChild(option);
	
	var option1=document.createElement("option");
	option1.text="Map 1";
	option1.value="maps/map1.jpg"
	this.map_select.drop.appendChild(option1);
	
	var option2=document.createElement("option");
	option2.text="Map 2";
	option2.value="maps/map2.jpg"
	this.map_select.drop.appendChild(option2);
		
	var option3=document.createElement("option");
	option3.text="Map 3";
	option3.value="maps/map3.jpg"
	this.map_select.drop.appendChild(option3);
	
	

	
	var map_display = document.createElement("div");
	map_display.title="Shows where the robot thinks it is in the world.  The grid lines are 1 meter apart.  The robot's right and left wheels leave red and purple tracks";
		
	this.need_redraw=true;
	this.renderer=new renderer_t(map_display,function() {myself.setup();}, function() {return myself.loop();} );
	if(!this.renderer.setup()) {
		var p=document.createElement("p");
		p.innerHTML="<p>WebGl seems to be disabled: <a target=_blank href=https://get.webgl.org>Click here to test</a><br> <u>If disabled, Try the following steps:</u></p> "
		p.innerHTML+="<p><b>Firefox:</b> Go to about:config in your address bar,search for webgl and check if webgl.disabled is true <br> No Luck? <a target=_blank href=https://support.mozilla.com/en-US/kb/how-do-i-upgrade-my-graphics-drivers>Help Page</a></p>";
		p.innerHTML+="<p><b>Chrome:</b> <ol><li><b>Is hardware acceleration enabled?</b> Type chrome://settings, show advanced settings -> under system > check Use hardware acceleration when available</li><b><li>Check WebGl:</b> Type chrome://flags into the address bar and confirm that Disable WebGl is gray</li></ol></p>";
		p.innerHTML+= "<p><b>Safari:</b> <ol><li>Go to Safari's Preferences</li><li>Select the Advanced tab</li><li>Check the Show Develop menu in menu bar checkbox</li><li>In the Develop menu, check Enable WebGl</li><li>Confused?<a target=_blank href=http://voicesofaliveness.net/webgl>Pictures</a></li></ol></p>";
		div.appendChild(p);
		this.renderer=null;
	}
	
	this.div.appendChild(map_display);
	
}

robot_map_t.prototype.setup=function() {
	if (this.renderer===null) return;
	this.renderer.set_size(this.div.offsetWidth,this.div.offsetWidth); // FIXME resize this

	// Add grid
	var grid_cells=10;
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

robot_map_t.prototype.add_map=function(filename) {
	
	
	var myself = this;
	
	var size = 1000;
	var width = 10;
	var height = 10;
	showOrigin = 20;
	
		var texture_file = filename;
		var texture = this.renderer.load_texture(texture_file);
		texture.minFilter = THREE.LinearFilter;
		var plane_material=new THREE.MeshBasicMaterial({map: texture,depthWrite:false,
			side:THREE.DoubleSide});
		var plane_geometry=new THREE.PlaneBufferGeometry(size*width,size*height,size,size);
		this.grid.geometry = plane_geometry;
		this.grid.material = plane_material;

		
		
		var line_geometry=new THREE.Geometry();
		var line_material=new THREE.LineBasicMaterial({color:0x0488c8,linewidth:1.5});

		for(var xx=0;xx<=width;++xx)
		{
			line_geometry.vertices.push(new THREE.Vector3(-size*width/2,-size*height/2.0+xx*size,0));
			line_geometry.vertices.push(new THREE.Vector3(size*width/2,-size*height/2.0+xx*size,0));
		}

		for(var yy=0;yy<=height;++yy)
		{
			line_geometry.vertices.push(new THREE.Vector3(-size*width/2.0+yy*size,-size*height/2,0));
			line_geometry.vertices.push(new THREE.Vector3(-size*width/2.0+yy*size,size*height/2,0));
		}
		if (showOrigin) { // add extra lines around origin
		  for (var del=-showOrigin;del<=showOrigin;del+=showOrigin/32) {
			line_geometry.vertices.push(new THREE.Vector3(del,-size*height/2,0));
			line_geometry.vertices.push(new THREE.Vector3(del,+size*height/2,0));
			line_geometry.vertices.push(new THREE.Vector3(-size*width/2.0,del,0));
			line_geometry.vertices.push(new THREE.Vector3(+size*width/2.0,del,0));
		  }
		}

				var line=new THREE.Line(line_geometry,line_material,THREE.LinePieces);
		this.grid.add(line);
		

}

robot_map_t.prototype.clear_map=function(){
	
	var myself = this;
	
	var size = 1000;
	var width = 10;
	var height = 10;
	showOrigin = 20;
	
	
	var plane_material=new THREE.MeshBasicMaterial({color:0xd8eef4,depthWrite:false,
			side:THREE.DoubleSide});
	var plane_geometry=new THREE.PlaneBufferGeometry(size*width,size*height,size,size);
	this.grid.geometry = plane_geometry;
	this.grid.material = plane_material;
	
	var line_geometry=new THREE.Geometry();
	var line_material=new THREE.LineBasicMaterial({color:0x0488c8,linewidth:1.5});

	for(var xx=0;xx<=width;++xx)
	{
		line_geometry.vertices.push(new THREE.Vector3(-size*width/2,-size*height/2.0+xx*size,0));
		line_geometry.vertices.push(new THREE.Vector3(size*width/2,-size*height/2.0+xx*size,0));
	}

	for(var yy=0;yy<=height;++yy)
	{
		line_geometry.vertices.push(new THREE.Vector3(-size*width/2.0+yy*size,-size*height/2,0));
		line_geometry.vertices.push(new THREE.Vector3(-size*width/2.0+yy*size,size*height/2,0));
	}
	if (showOrigin) { // add extra lines around origin
	  for (var del=-showOrigin;del<=showOrigin;del+=showOrigin/32) {
		line_geometry.vertices.push(new THREE.Vector3(del,-size*height/2,0));
		line_geometry.vertices.push(new THREE.Vector3(del,+size*height/2,0));
		line_geometry.vertices.push(new THREE.Vector3(-size*width/2.0,del,0));
		line_geometry.vertices.push(new THREE.Vector3(+size*width/2.0,del,0));
	  }
	}

			var line=new THREE.Line(line_geometry,line_material,THREE.LinePieces);
	this.grid.add(line);

	
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
	
	//console.log("New Map Selected.");
	
	var filename = this.last_map_select;
	if (filename === "none") this.clear_map();
	else this.add_map(filename);
	
	//Reset robot	
	this.mapRobot.model.destroy();
	this.mapRobot=null;
	var new_roomba = new roomba_t(this.renderer,null);
	this.mapRobot=new_roomba;
	this.reset_tracks = true;
	
}


