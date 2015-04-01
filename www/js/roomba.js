var url_path="/"; // http://robotmoose.com/";

function load_js(js)
{
	var scr=document.createElement("script");
	scr.src=js;
	document.head.appendChild(scr);
};

function load_dependencies()
{
	load_js(url_path+"js/3d.js");
};

(function(){load_dependencies()})();


function obstacles_t(renderer)
{
	this.renderer=renderer;
	this.obstacles=[];
}
obstacles_t.prototype.add=function (options)
{
	// console.log("Added obstacle at "+options.position);
	var obs={};
	obs.mesh=new THREE.Mesh(
		new THREE.CylinderGeometry(options.radius,options.radius,
			options.ht,16),
		new THREE.MeshPhongMaterial({color:options.color})
	);
	obs.position=options.position.clone();
	obs.P2=obs.position.clone();
	obs.P2.z=0.0; // 2D position
	obs.radius=options.radius;
	obs.radius2=options.radius*options.radius;

	obs.mesh.rotation.set(Math.PI/2,0,0);
	obs.mesh.position.copy(obs.position);

	// Return true if this point is inside us
	obs.contains=function(P) {
		var l2=obs.P2.distanceToSquared(P);
		// console.log("Comparing length "+Math.sqrt(l2)+" for P="+P+" P2="+obs.P2);
		return l2<obs.radius2;
	}
	this.renderer.scene.add(obs.mesh);
	this.obstacles.push(obs);
}


/* Tracks the wheel positions onscreen */
function wheel_tracker_t(renderer,lineOptions)
{
	this.max=10000; // maximum vertex count
	this.cur=0; // current vertex number

	this.geom=new THREE.Geometry();
	/* for some reason THREE.js doesn't let you change
	   vertex counts, so preallocate lots of vertices. */
	for (var x=0;x<this.max;x++)
	{
		this.geom.vertices.push(new vec3(0,0,0));
	}


	this.line=new THREE.Line(this.geom,
	  new THREE.LineBasicMaterial(lineOptions)
	);
	renderer.scene.add(this.line);
}

/* Add a new wheel location to this chart. */
wheel_tracker_t.prototype.add=function(pos) {
  if (!this.last || pos.m(this.last).length()>10.0) { // moved enough to be visible:
    if (this.cur<this.max) { // still have space to write
	// stupid: write all future vertices as this point
	for (var i=this.cur++;i<this.max;i++)
	     this.geom.vertices[i]=pos;
	this.geom.verticesNeedUpdate=true;
	this.geom.computeBoundingSphere();
    }
    this.last=pos;
  }
}

wheel_tracker_t.prototype.reset=function() {
  this.cur=0;
  this.last=undefined;
}



function roomba_t(renderer,obstacles)
{
	var myself=this;

	myself.model=new Array();

	if(obstacles)
		myself.obstacles=obstacles.obstacles;
	else
		myself.obstacles=null;

	myself.left=0;
	myself.right=0;
	myself.left_tracker=new wheel_tracker_t(renderer,{
		color: 0x8f00ff, linewidth:2 }); // purple left
	myself.right_tracker=new wheel_tracker_t(renderer,{
		color: 0xff0000, linewidth:2 }); // red right

	var model_path=url_path+"models/roomba/body.obj";
	myself.model=null;
	var model_color=0x404040;

	this.renderer=renderer;

	myself.model=renderer.load_obj(model_path);
	myself.model.set_color(model_color);
	myself.model.castShadow=true;
	myself.model.receiveShadow=true;

	myself.sensorObject3D=new THREE.Object3D(); // container object (OBJ isn't loaded yet)
	renderer.scene.add(myself.sensorObject3D);

	// Clear everything else in our object
	this.reset();


	myself.set_position=function(x,y,z)
	{
		myself.model.position.set(x,y,z);
	};

	myself.get_position=function()
	{
		return myself.model.position;
	};

	myself.set_rotation=function(x,y,z)
	{
		myself.model.rotation.set(x,y,z);
	};

	myself.get_rotation=function()
	{
		return myself.model.rotation;
	};

	myself.drive=function(left,right)
	{
		myself.left=left;
		myself.right=right;
	};
};


// Reset positions of wheels:
roomba_t.prototype.reset=function()
{
	this.wheelbase=250; // mm
	this.wheel=[];
	this.wheel[0]=new vec3(-0.5*this.wheelbase,0.3*this.wheelbase,0.01);
	this.wheel[1]=new vec3(+0.5*this.wheelbase,0,0.01);
	// Robot coordinate system:
	this.P=new vec3(0,0,0); // position
	this.UP=new vec3(0,0,1); // Z is up
	this.LR=new vec3(1,0,0); // left-to-right wheel
	this.FW=new vec3(0,1,0); // drive forward

	this.left_tracker.reset();
	this.right_tracker.reset();

	this.falling=false;
	this.sensorObject3D.rotation.set(0,0,0);
	this.model.rotation.set(0,0,0);
	this.model.position.set(0,0,0);
	this.model.scale.set(1,1,1);
}

// Simulate robot motion
roomba_t.prototype.loop=function(dt)
{
	dt = dt || 0.001; // weird startup, null dt?
	var mm=1; // from mm to model coordinates

	// Limit motor power
	var limit=function (power) {
		if (power>500) return 500;
		if (power<-500) return -500;
		return power;
	}
	var L=limit(this.left);
	var R=limit(this.right);

	var speed=mm*dt; // world units per step

	// Drive wheels forward
	this.wheel[0].pe(this.FW.t(L*speed));
	this.wheel[1].pe(this.FW.t(R*speed));

	// Enforce wheelbase
	this.P=this.wheel[0].p(this.wheel[1]).te(0.5);
	this.LR=this.wheel[1].m(this.wheel[0]); // left-to-right
	this.LR.normalize();
	this.FW=new vec3(-this.LR.y,this.LR.x,0.0); // forward
	var wheeloff=this.LR.clone();
	wheeloff.te(this.wheelbase*0.5);
	this.wheel[0]=this.P.m(wheeloff);
	this.wheel[1]=this.P.p(wheeloff);
	this.left_tracker.add(this.wheel[0]);
	this.right_tracker.add(this.wheel[1]);

	// Robot's Z rotation rotation, in radians
	this.angle_rad=Math.atan2(this.LR.x,-this.LR.y);
	this.angle=180.0/Math.PI*this.angle_rad;
	// console.log("Roomba P="+this.P+" mm and angle="+this.angle+" degrees");


	if (this.falling) {
		for (var i=0;i<2;i++) this.wheel[i].z-=2000.0*dt;
		this.model.rotation.x=this.model.rotation.x-dt;
		this.sensorObject3D.rotation.x=this.model.rotation.x;
	}

	this.model.rotation.z=this.angle_rad-Math.PI/2;
	this.sensorObject3D.rotation.z=this.angle_rad;

	// Update robot center position
	this.model.position.copy(this.P);
	this.sensorObject3D.position.copy(this.P);

	// Update camera position to follow robot
	renderer.controls.center.set(this.P.x,this.P.y,this.P.z);
	renderer.controls.object.position.set(
		this.P.x,this.P.y-1200,this.P.z+1400);

	this.sensor_check();
};

roomba_t.prototype.sensor_check=function() {
	// Update emulated sensors:
	if (this.light && emulator) {
		if (emulator.roomba) {
			this.sensors_to_emulator(emulator.roomba.get_sensors());
		}
	}
}

// Return the world-coordinates XYZ of this robot polar coordinates position
roomba_t.prototype.world_from_robot=function(radius,angle_rad,height)
{
	var W=new vec3(
		radius*Math.cos(angle_rad+this.angle_rad),
		radius*Math.sin(angle_rad+this.angle_rad),
		height || 0.0
	);
	W.pe(this.P); // robot-relative
	return W;
}

// Return true if this XYZ coordinate is "off the table"
roomba_t.prototype.out_of_bounds=function(C)
{
	var bounds=1200; // mm from origin to edge (FIXME: parameterize this)
	return Math.abs(C.x)>bounds || Math.abs(C.y)>bounds;
}

// Return true if this XYZ coordinate is inside an obstacle
roomba_t.prototype.obstructed=function(C)
{
	for (var i=0;i<this.obstacles.length;i++)
		if (this.obstacles[i].contains(C))
			return true;
	return false;
}

// Add sensor objects & render geometry
roomba_t.prototype.add_sensors=function() {

	var shiftGeometry=function (geom,shiftBy) {
		for (var i=0;i<geom.vertices.length;i++)
			geom.vertices[i].pe(shiftBy);
		geom.verticesNeedUpdate=true;
		geom.computeBoundingSphere();
		return geom;
	}
	// Array of light sensors:
	this.light=[];
	for (var i=0;i<6;i++) {
		var l={};
		l.start=150; // start range (mm)
		l.range=250; // max sensor range
		l.mesh=new THREE.Mesh(
			shiftGeometry(new THREE.CylinderGeometry(50.0,2.0, l.range, 8),
				new vec3(0,l.start+l.range*0.5,50)),
			new THREE.MeshPhongMaterial({
				transparent:true, opacity:0.5,
				color:0x0000ff
			})
		);
		l.angle_rad=Math.PI*0.3*(1.0-i/2.5); // radians relative to robot centerline
		l.mesh.rotation.set(0,0,l.angle_rad-Math.PI*0.5);
		this.sensorObject3D.add(l.mesh);
		l.last=0; // assume no obstacles
		this.light[i]=l;
	}

	this.floor=[];
	for (var i=0;i<4;i++) {
		var l={
			"last":2600, // assume the floor is visible
			"angle_rad":Math.PI*0.4*(1.0-i/1.5)
		};
		l.mesh=new THREE.Mesh(
			new THREE.CylinderGeometry(30.0,30.0, 200, 8),
			new THREE.MeshPhongMaterial({
				transparent:true, opacity:0.5,
				color:0x0000ff
			})
		);
		var radius=130;
		l.mesh.position.set(
			radius*Math.cos(l.angle_rad),
			radius*Math.sin(l.angle_rad),
			50
		);
		l.mesh.rotation.set(Math.PI/2,0,0);
		this.sensorObject3D.add(l.mesh);
		this.floor[i]=l;
	}
}


// Copy our simulated sensors into emulator's arduino_roomba_sensor_t
roomba_t.prototype.sensors_to_emulator=function(robot)
{
	robot.position=this.P;
	robot.angle=this.angle;
	robot.angle_rad=this.angle_rad;

	// Floor sensors
	var noff=0;
	for (var i=0;i<4;i++) {
		var L=this.floor[i];
		var v=2600; // assume floor by default
		L.mesh.material.color.setHex(0x0000ff); // blue by default
		if (this.out_of_bounds(this.world_from_robot(180,
			this.floor[i].angle_rad
		   )))
		{ // off the edge!
			L.mesh.material.color.setHex(0xff0000); // orange on error
			v=0;
			noff++;
		}
		robot.floor[i]=L.last=v;
	}
	if (noff>0) {
		console.log("Robot floor sensors off edge: "+noff+"\n");
	}
	if (noff==4 && this.out_of_bounds(this.P)) { // animate falling robot!
		this.falling=true;
	}

	// Obstacle sensors
	for (var i=0;i<6;i++) {
		var v=0; // assume no hits
		var L=this.light[i];
		L.mesh.material.color.setHex(0x0000ff); // blue by default
		var reflect=1000;
		for (var range=20;range<L.range;range+=100) {
			var W=this.world_from_robot(L.start+range,L.angle_rad);
			// console.log("Checking obstruction at W "+W);
			if (this.obstructed(W)) {
				//console.log("Hit obstruction at range "+range);
				v=reflect;
				L.mesh.material.color.setHex(0xff0000); // red on hit
				break; // keep closest value only
			}
			// assume rapid fall-off in reflectance
			reflect=0.5*reflect;
		}
		robot.light[i]=L.last=v;
	}
}

// Print current status
roomba_t.prototype.get_status=function()
{
	this.sensor_check();

	var status="robot.position = ";
	for (var axis=0;axis<3;axis++) {
		var v=0xffFFffFF&this.P.getComponent(axis);
		status+=v;
		if (axis<2) status+=", ";
	}
	status+=" mm<br>";
	status+=" robot.angle = "+(0xffFFffFF&this.angle)+" degrees<br>";
	if (this.light) {
		status+="robot.floor";
		for (var i=0;i<4;i++) {
			status+="["+i+"] = "+this.floor[i].last+"; \n";
		}
		status+="<br> robot.light";
		for (var i=0;i<6;i++) {
			status+="["+i+"] = "+this.light[i].last+";\n";
		}
	}
	return status;
}

