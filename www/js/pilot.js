/*
  JSON Testing
  Dr. Orion Lawlor, lawlor@alaska.edu, 2012-02-17
*/



/* Main update: display costs for user's selected values. */
function run() {
  document.getElementById('p_errorout').innerHTML="";
  try {

  } catch (err) {
    document.getElementById('p_errorout').innerHTML="Exception: "+err;
  }
}


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



// Return the current wall clock time, in seconds
function pilot_time() {
	return (new Date()).getTime()/1000.0;
}

// This counts outstanding network requests
var networkBusy=0;

// Send our last-used piloting command out across the network
function pilot_send() {
	var host=// "http://localhost:8080"+ // <- explicit host not needed, browser will use the .html server by default.
		"/superstar/";
	var url_start=host;

	if (networkBusy>1)
	{ // prevent overloading network: skip sending if already busy
		document.getElementById('p_outputNet').innerHTML="network lag";
		return;
	}

	networkBusy++;
	var send_time=pilot.time=pilot_time();
	var pilotStr=JSON.stringify(pilot);
	console.log(pilotStr);
	
	var robot;
	function set_robot() // Set global robot depending on checkbox selected 
	{
		if (document.getElementById("layla_gray").checked) robot=layla/gray;
		else if(document.getElementById("layla_orange").checked) robot=layla/orange;
		else if(document.getElementById("create_2").checked) robot=create/'2';
	}
	var starpath=robot+"/pilot";
	var url=url_start+starpath+"?set="+encodeURIComponent(pilotStr);
	
	var password=document.getElementById('robot_auth').value;
	if (password) { // append authentication code
		var seq="0"; //<- FIXME: get sequence number on first connection
		var auth=getAuthCode(password,starpath,pilotStr,seq);
		url+="&auth="+auth;
	}
	
	var xmlhttp=new XMLHttpRequest();
	xmlhttp.onreadystatechange=function()
	{ // this function called when network progress happens
		if(xmlhttp.readyState!=4) return;
		var status="Network Error";
		if (xmlhttp.status==200)
		{
			status="Data sent at t="+send_time+" ("+pretty(pilot_time()-send_time)+" second roundtrip)";
			if (networkBusy>1) status+=" ["+networkBusy+" requests in flight]";
		}
		document.getElementById('p_outputNet').innerHTML=status;
		networkBusy--;
	}
	xmlhttp.open("GET",url,true);
	xmlhttp.send(null);
	data_receive(); // Read Sensor Data at every Send
}
//Read sensor data every 250 ms
window.setInterval(function(){data_receive()},250);

// Recieve Sensor Values sent by backend 
function data_receive()
{
	var xmlrec=new XMLHttpRequest();
	var host=// "http://localhost:8080"+ // <- explicit host not needed, browser will use the .html server by default.
		"/superstar/";
	var url_start=host;
	var robot=document.getElementById('robot_name').value;
	var starpath=robot+"/data";
	var url_rec=url_start+starpath+"?get";
	
	xmlrec.open("GET",url_rec,true);
	xmlrec.send(null);
	var pilot_received = emptyUSonic();
	console.log(pilot_received);
	xmlrec.onreadystatechange = function(){
	if(this.readyState==4){ //check for network change
	var pilot_received = JSON.parse(xmlrec.responseText);
	console.log(pilot_received);
/* Read Ultrasonic sensor data sent by backend*/
	document.getElementById('USonic1').value=Sensors.USonic.USensor1 = pilot_received.uSound1;
	document.getElementById('USonic2').value=Sensors.USonic.USensor2 = pilot_received.uSound2;
	document.getElementById('USonic3').value=Sensors.USonic.USensor3 = pilot_received.uSound3;
	document.getElementById('USonic4').value=Sensors.USonic.USensor4 = pilot_received.uSound4;
	document.getElementById('USonic5').value=Sensors.USonic.USensor5 = pilot_received.uSound5; 
	}
}
	
}


// This function is called at every mouse event.
//   upState: 0 if down, 1 if up, 2 if not changing
var mouse_down=0;
function pilot_mouse(event,upState) {
// Allow user to set maximum power
	var maxPower=0.7;
	var powerUI=document.getElementById('robot_power').value*0.01;
	if (isNaN(powerUI)) maxPower=0.0;
	else if (powerUI<maxPower) { maxPower=powerUI; }
	

	var arrowDiv=document.getElementById('pilot_arrows');
	var frac=getMouseFraction(event,arrowDiv);
	var str="";
	

	var dir={ forward: pretty(0.5-frac.y), turn:pretty(frac.x-0.5) };
	
	str+="Move "+dir.forward+" forward, "+dir.turn+" turn<br>\n";

//Proportional control.  The 2.0 is because mouse is from -0.5 to +0.5
	dir.forward=dir.forward*2.0*powerUI;
	dir.turn=dir.turn*2.0*powerUI;

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
  var splitName = currentRobot.split('/');


 // Use AppRTC instead of gruveo for video
  var videoURL = "https://appear.in/" + splitName[0] + splitName[1];
  document.getElementById("video_frame").src = videoURL;

}

//Open Gruveo Video
function openGruveoVideo()
{
  var currentRobot = robot;
  var splitName = currentRobot.split('/');

 
  var videoURL = "http://gruveo.com/#" + splitName[0] + splitName[1];

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




