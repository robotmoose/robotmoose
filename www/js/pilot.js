/*
  This is the old pre-summer 2015 robot piloting interface.
  It's full featured, but everything is in fixed positions onscreen,
  so you can't watch sensor data as you drive.
  
  Dr. Orion Lawlor, lawlor@alaska.edu, 2012-2015 (Public Domain)
*/



/* Walk the DOM to get the client X,Y position of this element's topleft corner.
  From www.kirupa.com/snippets/examples/move_to_click_position.htm */
 /*Gets the client X,Y position with .getBoundingClientRectangle
		https://developer.mozilla.org/en-US/docs/Web/API/Element.getBoundingClientRect*/
function getClientPosition(element) {
	var temp = element.getBoundingClientRect(); //Gets coordinate data
	var xPosition = temp.left;
	var yPosition = temp.top;
	return { x: xPosition, y: yPosition };
}
/* Return the fractional mouse position of the mouse relative to this element.
   Returns x=0.0 at the left edge, x=1.0 at the right edge;
           y=0.0 at the top edge, y=1.0 at the bottom edge. */
function getMouseFraction(event,domElement) {
  var corner=getClientPosition(domElement);
  var xFrac=(event.clientX-corner.x)/domElement.scrollWidth;
  var yFrac=(event.clientY-corner.y)/domElement.scrollHeight;
  return { x:xFrac, y:yFrac };
}

// Round this number to the nearest thousandth, making it look pretty.
function pretty(number) {
	return Math.round(number*1000)/1000.0;
}

// Clamp this value between lo and hi
function clamp(v,lo,hi) {
	if (v<lo) return lo;
	if (v>hi) return hi;
	else return v;
}

// Return an "empty" robot power object, with everything stationary
function emptyPower()
{
	return {L:0, R:0, dump:0, mine:0, arms:0};
}

function emptyLED()
{
 	//return {R:this.color.rgb[0], G:this.color.rgb[1], B:this.color.rgb[3]};
         return{On:false, Demo:false, R:0, G:0, B:0};
}
//Returns empty UltraSonic Sensors
function emptyUSonic()
{
	return {USound1:0, USound2:0, USound3:0, USound4:0, USound5:0};
}


// This class stores our last-used piloting command
var pilot={
	/* Power to each actuator */
	power: emptyPower(),

	/* Time, in seconds, of last pilot command */
	time:0,

	/*LED bits */
	LED: emptyLED(),

        /* Scripted command to run */
	cmd: { run: "", arg:"" }
};


var Sensors={
	USonic:emptyUSonic()
};

function LEDtoggle(){
	pilot.LED.On = document.getElementById("LEDtoggle").checked;

// If LED is Off, make LED Demo unclickable
	if(document.getElementById("LEDtoggle").checked) {document.getElementById("LEDdemo").disabled=false;}
	else {document.getElementById("LEDdemo").disabled=true;}
	pilot_send();

}
//Toggle Demo mode. LED ramp up and turn off cycle
function LEDdemo()
{
         pilot.LED.Demo=document.getElementById("LEDdemo").checked;
	 pilot_send();
}



// Return our robot's name.  This is also the superstar path.
function robot_name() {
	return document.getElementById('robot_name').value;
}

// Send this command off to superstar:
function superstar_send(starpath,starcmd,callback) {
	var url_start="/superstar/"; // absolute URL
	var url=url_start+starpath+starcmd

	var xmlhttp=new XMLHttpRequest();
	xmlhttp.onreadystatechange=function()
	{ // this function is called when network progress happens
		if(xmlhttp.readyState!=4) return;
		else callback(xmlhttp);
	};
	xmlhttp.open("GET",url,true);
	xmlhttp.send(null);
}


// Return the current wall clock time, in seconds
function pilot_time() {
	return (new Date()).getTime()/1000.0;
}

// This counts outstanding network requests
var networkBusy=0;
var delayedRequest =false;
// Send our last-used piloting command out across the network
function pilot_send() {
	if (networkBusy>1)
	{ // prevent overloading network: skip sending if already busy
		document.getElementById('p_outputNet').innerHTML="network lag";
		delayedRequest = true;
		return;
	}
	networkBusy++;

	var send_time=pilot.time=pilot_time();
	var pilotStr=JSON.stringify(pilot);
	//console.log(pilotStr);

	var starpath=robot_name()+"/pilot";
	var starcmd="?set="+encodeURIComponent(pilotStr);

	var password=document.getElementById('robot_auth').value;
	if (password) { // append authentication code
		var seq="0"; //<- FIXME: get sequence number on first connection
		var auth=getAuthCode(password,starpath,pilotStr,seq);
		starcmd+="&auth="+auth;
	}

	superstar_send(starpath,starcmd,function(xmlhttp) {
		var status="Network Error";
		if (xmlhttp.status==200)
		{
			status="Data sent at t="+send_time+" ("+pretty(pilot_time()-send_time)+" second roundtrip)";
			if (networkBusy>1) status+=" ["+networkBusy+" requests in flight]";
		}
		document.getElementById('p_outputNet').innerHTML=status;
		networkBusy--;
		if(networkBusy == 0)
		{
			if(delayedRequest)
			{
				delayedRequest = false;
				pilot_send();
			}
		}
	});

	sensor_receive(); // Read Sensor Data at every Send
}

//Read sensor data every 250 ms
window.setInterval(function(){sensor_receive()},250);

var sensors = {"error":"network not connected"};

// Recieve Sensor Values sent by backend
function sensor_receive()
{
	var starpath=robot_name()+"/sensors";
	var starcmd="?get";

	superstar_send(starpath,starcmd,function(xmlhttp) {
		if (xmlhttp.status==200) {
			var sensor_data=xmlhttp.responseText;
			//console.log("sensors: "+sensor_data);
			try {
				sensors=JSON.parse(sensor_data);
			}
			catch (err) {
				sensors.error="Exception: "+err;
				sensors.data=sensor_data;
    			}
		}

		var out=document.getElementById('p_outputSensors');
		if (out) {
			var prettySensors=JSON.stringify(sensors,null,4);
			//prettySensors=prettySensors.replace(/["]/g,"");
			out.innerHTML=prettySensors;
		}
	});
}

// Return the drive power the user has currently selected
function get_pilot_power() {
	var maxPower=0.7;
	var powerUI=document.getElementById('robot_power').value*0.01;
	if (isNaN(powerUI)) maxPower=0.0;
	else if (powerUI<maxPower) { maxPower=powerUI; }
	return powerUI;
}


// Mike's input keydown handler
var keyInput;
var keyboardIsDriving=false;

document.addEventListener("visibilitychange",handleVisibilityChange); // Listens for any window visibilty change

window.setInterval(focusCheck,100); // Check if window is in focus every 100 ms

// If window/tab is hidden, stop the robot
function handleVisibilityChange()
{
	if(document.hidden)
	{
		//console.log("Hidden :" + document.hidden)
		pilot.power = emptyPower();
		pilot_send();
	}
}

//Stop robot if window is not in focus
function focusCheck()
{
	    //console.log(document.hasFocus());
		if(!document.hasFocus())
		{
			pilot.power = emptyPower();
			pilot_send();
		}
}
// Return true if this key (as a string) is pressed
function keyDown(key,alternateKey) {
	var code=key.charCodeAt(0);
	// console.log("Key code "+code+" : "+keyInput.keys_down[code]);

	if (keyInput.keys_down[code]) return true;

	if (alternateKey) return keyDown(alternateKey);  // hacky recursion
	return false;
}

//This function is called at every keypress event
// FIXME: Add proportional control
function keyboardDrive()
{
	// console.log("Keyboard activity");
	var maxPower=get_pilot_power();

	var forward=0.0, turn=0.0;

	if(keyDown('a','A')) turn-=1.0; // 'a' is pressed, turn left
	if(keyDown('d','D')) turn+=1.0; //'d' is pressed, turn right
	if(keyDown('s','S')) forward-=1.0; //'s' is pressed, reverse
	if(keyDown('w','W')) forward+=1.0;
	if(keyDown(' ')) turn=forward=0.0; // stop!

	if (turn==0.0 && forward==0.0) keyboardIsDriving=false;
	else keyboardIsDriving=true;

	var newPower=emptyPower();
	newPower.L=clamp(maxPower*(forward+turn),-maxPower,+maxPower);
	newPower.R=clamp(maxPower*(forward-turn),-maxPower,+maxPower);
	pilot.power = newPower;
	pilot_send();
}

// This function is called at every mouse event.
//   upState: 0 if down, 1 if up, 2 if not changing
var mouse_down=0;
function pilot_mouse(event,upState) {
// Allow user to set maximum power
	var maxPower=get_pilot_power();

	var arrowDiv=document.getElementById('pilot_arrows');
	var frac=getMouseFraction(event,arrowDiv);
	var str="";

	var dir={ forward: pretty(0.5-frac.y), turn:pretty(frac.x-0.5) };

	str+="Move "+dir.forward+" forward, "+dir.turn+" turn<br>\n";

//Proportional control.  The 2.0 is because mouse is from -0.5 to +0.5
	dir.forward=dir.forward*2.0*maxPower;
	dir.turn=dir.turn*2.0*maxPower;

	var totPower=Math.abs(dir.forward)+Math.abs(dir.turn);
	var newPower=emptyPower();
	if (frac.x<0.9) { /* normal driving */
		newPower.L=pretty(clamp(dir.forward+dir.turn,-maxPower,maxPower));
		newPower.R=pretty(clamp(dir.forward-dir.turn,-maxPower,maxPower));
	} else { /* right hand corner of screen: special magic values */
		if (frac.y<0.33) {
			newPower.dump=pretty((1.0/6-frac.y)*3);
		} else if (frac.y<0.66) {
			newPower.mine=pretty((3.0/6-frac.y)*3);
		} else {
			newPower.arms=pretty((5.0/6-frac.y)*3);
		}
	}
	var newPowerStr=JSON.stringify(newPower);
	str+="Power "+newPower.L+" left, "+newPower.R+" right, "+newPowerStr+" ";

	if (upState==1) mouse_down=0;
	if (upState==0) mouse_down=1;
	if (mouse_down) {
		arrowDiv.style.backgroundColor='#222222';
		str+=" SENDING";
	} else {
		arrowDiv.style.backgroundColor='#404040';
		newPower=emptyPower();
		totPower=0;
		str+=" (click to send)";
	}
	pilot.power=newPower;
	pilot_send();

	document.getElementById('p_outputPilot').innerHTML=str;

	document.getElementById('glow').style.opacity=clamp(totPower*3.0,0.0,1.0);
	event.stopPropagation();
}

//Open correct AppRTC feed based on robot name entered. Gets run every time the robot name is changed
function updateRTCVideo()
{
  var currentRobot = document.getElementById('robot_videoname').value;
  var robotVideo = currentRobot.replace(/[/]/g,"");


 // Use AppRTC instead of gruveo for video
  var videoURL = "https://appear.in/" + robotVideo;
  document.getElementById("video_frame").src = videoURL;

}

//Open Gruveo Video
function openGruveoVideo()
{
  var robotVideo = robot_name().replace(/[/]/g,"");

  var videoURL = "http://gruveo.com/#" + robotVideo;

  // On Firefox or Chrome, specifying a width and height opens a new window, not a tab.
  //  (On IE, it's up to the user's preferences)
  window.open(videoURL,"video",'width=800,height=600');
}


//Update pilot.RGB variable with currently selected RGB values
function updateRGB()
{
  var the_color_picker=document.getElementById("color_picker");
  var rgb=the_color_picker.color.rgb;


  pilot.LED.R=rgb[0];
  document.getElementById('LED_red').value=rgb[0];

  pilot.LED.G=rgb[1];
  document.getElementById('LED_green').value=rgb[1];

  pilot.LED.B=rgb[2];
  document.getElementById('LED_blue').value=rgb[2];

  pilot_send();
}

//Run scripted command, like say or play
function runCmd(run,arg) {
	pilot.cmd.run=run;
	pilot.cmd.arg=arg;
	pilot_send();
}


var map;
var mapRobot;
var renderer;
var last_lidar_change=-1;

// Map renderer loop
function mapLoop(dt) {
	if (sensors.location) {
		// Convert angle from degrees to radians
		var angle_rad=sensors.location.angle*Math.PI/180.0;
		// Convert position from meters to mm (rendering units)
		var P=new vec3(sensors.location.x,sensors.location.y,0.0).te(1000.0);

		// Move onscreen robot there
		mapRobot.set_location(P,angle_rad);

		// Place the wheels (so wheel tracks work)
		mapRobot.wheel[0]=mapRobot.world_from_robot(150,+Math.PI*0.5);
		mapRobot.wheel[1]=mapRobot.world_from_robot(150,-Math.PI*0.5);

		// Check for obstacle sensors
		if (sensors.lidar) {
			if (sensors.lidar.change!=last_lidar_change) {
				mapRobot.draw_lidar(renderer,sensors.lidar);
				last_lidar_change=sensors.lidar.change;
			}
		}
	}
}

/* Main setup */
function pilotReady() {
  document.getElementById('p_errorout').innerHTML="";

  // Keyboard driving
  keyInput=new input_t(keyboardDrive,window);

//  try {
	map=document.getElementById("map_area");
	if (map) {
		var setup=function() {
			renderer.set_size(map.offsetWidth,map.offsetWidth); // FIXME resize this

			// Add grid
			var grid_cells=100;
			var per_cell=1000; // one meter cells (in mm)
			grid=renderer.create_grid(per_cell,grid_cells,grid_cells,20);
			grid.rotation.x=0;

			// Add light source
			var size=100000;
			var intensity=0.8;
			var light=new renderer.create_light(intensity,
				new THREE.Vector3(-size/2,-size/2,+size));

			// Add a robot
			mapRobot=new roomba_t(renderer,null);

			// Set initial camera
			renderer.controls.center.set(0,0,0); // robot?
			renderer.controls.object.position.set(0,-1200,1400);
			console.log("Set up renderer");
		};
		renderer=new renderer_t(map,setup,mapLoop);
		if(!renderer)
			alert("Is WebGL enabled?");
		else
			renderer.setup();
	}
//  } catch (err) {
//	document.getElementById('p_errorout').innerHTML="Setup exception: "+err;
//  }
}

