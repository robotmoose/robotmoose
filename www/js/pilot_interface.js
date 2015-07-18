/*
  This is the main operator control interface for a driving robot.

Members
	onpilot(pilotdata) - callback triggered when pilot data needs to be sent
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



// Return the current wall clock time, in seconds
function pilot_time() {
	return (new Date()).getTime()/1000.0;
}



function pilot_interface_t(div)
{
	if(!div)
		return null;

	this.pilot={
		/* Power to each actuator */
		power: emptyPower(),

		/* Time, in seconds, of last pilot command */
		time:0,

		/*LED bits (should these still be here, or made PWMs or something?) */
		LED: emptyLED(),

		/* Scripted command to run */
		cmd: { run: "", arg:"" }
	};

	this.mouse_down=0;

	this.div=div;
	this.arrowDiv=document.createElement("div");
	this.arrowDiv.style.backgroundColor="#808080";
	this.arrowDiv.style.position="relative";
	this.arrowDiv.style.width=this.arrowDiv.style.height="200px";
	this.div.appendChild(this.arrowDiv);

	// Mouse event handlers for arrow div
	var myself=this;
	this.mouse_in_div=0;
	this.arrowDiv.onmousedown=function(evt) { myself.pilot_mouse(evt,1); myself.div.click(); };
	this.arrowDiv.ondragstart=function(evt) { myself.pilot_mouse(evt,1); };
	this.arrowDiv.onmouseup=function(evt) { myself.pilot_mouse(evt,-1); };
	this.arrowDiv.onmouseenter=function(evt) { myself.pilot_mouse(evt,0,+1); };
	this.arrowDiv.onmouseleave=function(evt) { myself.pilot_mouse(evt,-1,-1); };
	this.arrowDiv.onmousemove=function(evt) { myself.pilot_mouse(evt,0); };
	this.arrowDiv.ondblclick=function(evt) { myself.pilot_mouse(evt,0); };

	// Add arrow image
	var img=document.createElement("img");
	img.src="/images/arrows_hard.png";
	img.style.position="absolute";
	img.style.left=img.style.top="0px";
	img.style.width=img.style.height="100%";
	img.style.pointerEvents="none";
	this.arrowDiv.appendChild(img);

	// Keyboard driving
	this.keyboardIsDriving=false;
	this.keyInput=new input_t(function() {myself.pilot_keyboard()},window);
}




// Return the drive power the user has currently selected
pilot_interface_t.prototype.get_pilot_power=function() {
	var maxPower=0.7;

	var powerUI=0.2;
	var powerUIdom=document.getElementById('robot_power');
	if (powerUIdom) powerUI=powerUIdom.value*0.01;
	if (isNaN(powerUI)) powerUI=0.2;
	else if (powerUI<maxPower) { maxPower=powerUI; }

	return maxPower;
}

// This function is called at every mouse event.
//   mouse_down_del: +1 if down, -1 if up, 0 if unchanged (move)
//   mouse_in_del: +1 if entering, -1 if leaving, 0 if unchanged
pilot_interface_t.prototype.pilot_mouse=function(event,mouse_down_del,mouse_in_del) {
	if (mouse_in_del) this.mouse_in_div=mouse_in_del;

// Allow user to set maximum power
	var maxPower=this.get_pilot_power();

	var arrowDiv=this.arrowDiv;
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

	if (mouse_down_del==1) this.mouse_down=1;
	if (mouse_down_del==-1) this.mouse_down=-1;
	if (this.mouse_down==1) {
		arrowDiv.style.backgroundColor='#222222';
		str+=" SENDING";
	} else {
		arrowDiv.style.backgroundColor='#404040';
		newPower=emptyPower();
		totPower=0;
		str+=" (click to send)";
	}
	this.pilot_send(newPower);

	// Report debug data:
	console.log(str);
	// document.getElementById('p_outputPilot').innerHTML=str;

	// document.getElementById('glow').style.opacity=clamp(totPower*3.0,0.0,1.0);
	event.stopPropagation();
};

//This function is called at every keypress event
// FIXME: Add proportional control
pilot_interface_t.prototype.pilot_keyboard=function()
{
	if (this.mouse_in_div<1) return; // skip keystroke

	// console.log("Keyboard activity");
	var maxPower=this.get_pilot_power();

	// Return true if this key (as a string) is pressed
	var keyInput=this.keyInput;
	var keyDown=function(key,alternateKey) {
		var code=key.charCodeAt(0);
		// console.log("Key code "+code+" : "+keyInput.keys_down[code]);

		if (keyInput.keys_down[code]) return true;

		if (alternateKey) return keyDown(alternateKey);  // hacky recursion
		return false;
	}

	var forward=0.0, turn=0.0;

	if(keyDown('a','A')) turn-=1.0; // 'a' is pressed, turn left
	if(keyDown('d','D')) turn+=1.0; //'d' is pressed, turn right
	if(keyDown('s','S')) forward-=1.0; //'s' is pressed, reverse
	if(keyDown('w','W')) forward+=1.0;
	if(keyDown(' ')) turn=forward=0.0; // stop!

	if (turn==0.0 && forward==0.0) this.keyboardIsDriving=false;
	else this.keyboardIsDriving=true;

	var newPower=emptyPower();
	newPower.L=clamp(maxPower*(forward+turn),-maxPower,+maxPower);
	newPower.R=clamp(maxPower*(forward-turn),-maxPower,+maxPower);
	this.pilot_send(newPower);
}


// It's not clear a pilot needs to download data, but it's easy:
pilot_interface_t.prototype.download=function(robot_name)
{
	superstar_get(robot_name,"pilot",function(newPilot) { this.pilot=newPilot; });
}

pilot_interface_t.prototype.upload=function(robot_name)
{
	superstar_set(robot_name,"pilot",this.pilot);
}

// This is a simple placeholder, to get things working for now:
pilot_interface_t.prototype.pilot_send=function(newPower) {
	this.pilot.power=newPower;
	if (this.onpilot) this.onpilot(this.pilot);
};

/*
  This is the real version, with network spam prevention and such.

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
*/


