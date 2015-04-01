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
	var obs=options;
	obs.mesh=new THREE.Mesh(
		new THREE.CylinderGeometry(options.radius,options.radius,
			options.ht,16),
		new THREE.MeshPhongMaterial({color:options.color})
	);
	obs.mesh.rotation.set(Math.PI/2,0,0);
	obs.mesh.position.copy(options.position);
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

	myself.left=0;
	myself.right=0;
	myself.left_tracker=new wheel_tracker_t(renderer,{
		color: 0x8f00ff, linewidth:2 }); // purple left
	myself.right_tracker=new wheel_tracker_t(renderer,{
		color: 0xff0000, linewidth:2 }); // red right

	var model_path=url_path+"models/roomba/body.obj";
	myself.model=null;
	var model_color=0x404040;
	
	// Positions of Left and Right wheels
	this.reset();
	
	this.renderer=renderer;

	myself.model=renderer.load_obj(model_path);
	myself.model.rotation.set(0,0,0);
	myself.model.position.set(0,0,0);
	myself.model.scale.set(1,1,1);
	myself.model.set_color(model_color);
	myself.model.castShadow=true;
	myself.model.receiveShadow=true;
	
	myself.sensorObject3D=new THREE.Object3D(); // container object (OBJ isn't loaded yet)
	renderer.scene.add(myself.sensorObject3D);
	

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
				color:0xff0080
			})
		);
		l.angle_rad=Math.PI*0.3*(i/2.5-1.0); // radians relative to robot centerline
		l.mesh.rotation.set(0,0,l.angle_rad);
		this.sensorObject3D.add(l.mesh);
		this.light[i]=l;
	}
}

// Reset positions of wheels:
roomba_t.prototype.reset=function() 
{
	this.wheelbase=250; // mm
	this.wheel=[];
	this.wheel[0]=new vec3(-0.5*this.wheelbase,0,0.01);
	this.wheel[1]=new vec3(+0.5*this.wheelbase,0,0.01);
	// Robot coordinate system:
	this.P=new vec3(0,0,0); // position
	this.UP=new vec3(0,0,1); // Z is up
	this.LR=new vec3(1,0,0); // left-to-right wheel
	this.FW=new vec3(0,1,0); // drive forward
	
	this.left_tracker.reset();
	this.right_tracker.reset();
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
	this.orient_rad=Math.atan2(this.LR.y,this.LR.x);
	this.orient=180.0/Math.PI*this.orient_rad;
	// console.log("Roomba P="+this.P+" mm and orient="+this.orient+" degrees");
	this.model.rotation.z=this.orient_rad;
	this.sensorObject3D.rotation.z=this.orient_rad;

	// Update robot center position
	this.model.position.copy(this.P); 
	this.sensorObject3D.position.copy(this.P); 
	
	// Update camera position to follow robot
	renderer.controls.center.set(this.P.x,this.P.y,this.P.z);
	renderer.controls.object.position.set(
		this.P.x,this.P.y-1200,this.P.z+1400);
	
	// Update emulated sensors:
	if (emulator) {
		if (emulator.roomba) {
			this.sensors_to_emulator(emulator.roomba.get_sensors());
		}
	}
};

// Copy sensors into emulator's arduino_roomba_sensor_t
roomba_t.prototype.sensors_to_emulator=function(robot)
{
	
}

// Print current status
roomba_t.prototype.get_status=function() 
{
	var status="robot.position = ";
	for (var axis=0;axis<3;axis++) {
		var v=0xffFFffFF&this.P.getComponent(axis);
		status+=v;
		if (axis<2) status+=", ";
	}
	status+=" mm<br>";
	status+=" robot.angle = "+(0xffFFffFF&this.orient)+" degrees<br>";
	return status;
}


