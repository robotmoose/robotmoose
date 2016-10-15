/*
  This is the main operator control interface for a driving robot,
  in the new 2015-07 UI style.

Members
	onpilot(pilotdata) - callback triggered when pilot data needs to be sent
*/


// Pythagorean Theorem, finds distance between two points.
function getDistance(p1, p2)
{
	var a=p1.x-p2.x;
	var b=p1.y-p2.y;
	return Math.sqrt((a*a)+(b*b));
}

// Rotates an element by the specified degrees.
function rotate(el, deg)
{
	el.style.transform='rotate('+deg+'deg)';
	el.style.oTransform='rotate('+deg+'deg)';
	el.style.msTransform='rotate('+deg+'deg)';
	el.style.webkitTransform='rotate('+deg+'deg)';
};

// Returns the offset position for the mouse relative to a specified element.
//http://stackoverflow.com/questions/442404/retrieve-the-position-x-y-of-an-html-element
function getOffset(el)
{
	var _x=0;
	var _y=0;
	while(el&&!isNaN(el.offsetLeft)&&!isNaN(el.offsetTop))
	{
		_x+=el.offsetLeft-el.scrollLeft;
		_y+=el.offsetTop-el.scrollTop;
		el=el.offsetParent;
	}
	return {y:_y,x:_x};
};

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
	return {L:0, R:0};
}


// Round this number to the nearest thousandth, making it look pretty.
function pretty(number)
{
	return Math.round(number*1000)/1000.0;
}

// Clamp this value between lo and hi
function clamp(v,lo,hi)
{
	if(v<lo)
		return lo;
	if(v>hi)
		return hi;
	return v;
}



// Return the current wall clock time, in seconds
function pilot_time()
{
	return (new Date()).getTime()/1000.0;
}

function pilot_interface_t(div,doorway)
{
	var _this=this;
	if(!div||!doorway)
		return null;

	this.pilot={
		/* Power to each actuator */
		power: emptyPower(),

		/* Time, in seconds, of last pilot command */
		time:0,

		/* Scripted command to run */
		cmd: { run: "", arg:"" }
	};

	this.div=div;
	this.doorway=doorway;

	this.element=document.createElement("div");
	this.element.style.display="table";
	this.element.style.margin="auto auto";
	this.div.appendChild(this.element);

	// Keyboard driving
	this.keyboardIsDriving=false;
	this.keyInput=new input_t(function() {_this.pilot_keyboard()},window);

	// Gamepad support
	if(navigator.getGamepads()[0])
	{
		$.notify({message: 'Gamepad connected.'}, {type: 'success'});
		_this.gamepad_interval = setInterval(function() {
			_this.handle_gamepad_input(_this);
		},50);
	}
	else
	{
		this.gamepad_interval=null;
	}
}

// Configure our pilot GUI for the current firmware setup
pilot_interface_t.prototype.reconfigure=function(config_editor)
{
	var entries=config_editor.get_entries();

	// Check if we need to reconfigure at all
	var entries_string="";
	for(let key in entries) if(entries[key]) entries_string+=entries[key].type+",";
	if (entries_string==this.last_entries_string) return; // no changes to report
	this.last_entries_string=entries_string;

	// Clear out existing pilot GUI elements from our div
	while (this.element.firstChild) {
		this.element.removeChild(this.element.firstChild);
	}
	// this.pilot.power=emptyPower(); //<- cleaner, but loses servo positions on reconfigure

	// Add pilot GUI elements for each configured device
	var servos=0, pwms=0, blinks=0;
	for(let key in entries)
	if(entries[key])
	{
		var type=entries[key].type;
		switch (type) {
		case "create2":
		case "bts":
		case "sabertooth1":
		case "sabertooth2":
			this.make_drive(entries[key]);
			break;

		case "servo":
			this.make_slider(entries[key],"servo",servos, 0,180);
			servos++;
			break;

		case "pwm":
			this.make_slider(entries[key],"pwm",pwms, 0,100);
			pwms++;
			break;

		default: // ignore unknown object
			break;
		};
	}
}

// Add GUI element for a simple adjustable value, stored in pilot.name[number]
pilot_interface_t.prototype.make_slider=function(config_entry,name,number, minval,maxval)
{
	var _this=this;
	if (!_this.pilot.power[name])
		_this.pilot.power[name]=[];
	if (!_this.pilot.power[name][number])
		_this.pilot.power[name][number]=0.0;
	if(name=="servo")
		_this.pilot.power[name][number]=90.0;

	var p=document.createElement("p");
	var label_name=document.createTextNode(name+"["+number+"] = ");
	var label_value=document.createTextNode(""+(0xffFFffFF&_this.pilot.power[name][number]));

	var slider=document.createElement("input");
	slider.type="range";
	slider.min=minval;
	slider.max=maxval;
	slider.value=_this.pilot.power[name][number];

	var set_power=function()
	{
		_this.pilot.power[name][number]=parseInt(slider.value);
		label_value.nodeValue=""+(0xffFFffFF&_this.pilot.power[name][number]);
	};


	slider.addEventListener("change",set_power);
	slider.addEventListener("input",set_power);

	p.appendChild(label_name);
	p.appendChild(label_value);
	p.appendChild(slider);
	this.element.appendChild(p);
	robot_network.update_pilot(_this.pilot);
}

// Add GUI elements for driving around (arrows)
pilot_interface_t.prototype.make_drive=function(config_entry)
{
	var column_left_width = 160;
	var column_right_width = 160;
	var column_padding = 10;

	// Make arrow div
	this.arrowDiv=document.createElement("div");
	this.arrowDiv.title="Click to drive the robot.  Hold down to keep driving.";
	this.arrowDiv.style.backgroundColor = "#BBBBBB";
	this.arrowDiv.style.position="relative";
	this.arrowDiv.style.width=this.arrowDiv.style.height=column_left_width+column_padding+column_right_width;
	this.element.appendChild(this.arrowDiv);

	// Drive power input
	var starting_percent=20;
	this.drive=
	{
		div:document.createElement("div"),
		label:document.createElement("label"),
		slider:document.createElement("input")
	};

	this.drive.div.className="form-group";
	this.drive.div.style.marginTop=column_padding;
	this.element.appendChild(this.drive.div);

	this.drive.label.style.width=column_left_width;
	this.drive.label.style.float="left";
	this.drive.label.style.marginRight=column_padding;

	var update_text=function(){_this.update_drive_text();};
	this.drive.slider.addEventListener("change",update_text);
	this.drive.slider.addEventListener("input",update_text);
	this.drive.div.appendChild(this.drive.label);

	this.drive.slider.type="range";
	this.drive.slider.size=8;
	this.drive.slider.min=0;
	this.drive.slider.max=100;
	this.drive.slider.step=1;
	this.drive.slider.value=starting_percent;
	this.drive.slider.style.width=column_right_width;
	this.drive.div.appendChild(this.drive.slider);

	this.update_drive_text();

	var _this=this;

	// Gamepad event handlers
	window.addEventListener('gamepadconnected', function(e) {
		$.notify({message: 'Gamepad connected.'}, {type: 'success'});
		_this.gamepad_interval = setInterval(function() {
			_this.handle_gamepad_input();
		}, 50);
	});
	window.addEventListener('gamepaddisconnected', function(e) {
		$.notify({message: 'Gamepad disconnected.'}, {type: 'danger'});
		clearInterval(_this.gamepad_interval);
	});


	this.images = {};

    // Add wheel image -- rotates, un-restrained motion
	this.images.wheel = document.createElement("img");
	this.images.wheel.src = "/images/steering_wheel.png";
	this.images.wheel.style.position = "absolute";
	this.images.wheel.style.left = this.images.wheel.style.top = "0px";
	this.images.wheel.style.width = this.images.wheel.style.height = "100%";
	this.images.wheel.style.pointerEvents = "none";
	this.arrowDiv.appendChild(this.images.wheel);

    // Add needlewell image -- static
	this.images.gradient = document.createElement("img");
	this.images.gradient.src = "/images/gauge_needlewell.png";
	this.images.gradient.style.position = "absolute";
	this.images.gradient.style.left = this.images.gradient.style.top = "33.7%";
	this.images.gradient.style.width = "auto";
	this.images.gradient.style.height = "32.5%";
	this.images.gradient.style.pointerEvents = "none";
	this.arrowDiv.appendChild(this.images.gradient);

    // Add needle image -- rotates, restrained motion
    // max (in-red) is [-11.75deg] & min (in-green) is [-77.5deg]
	this.images.needle = document.createElement("img");
	this.images.needle.src = "/images/gauge_needle.png";
	this.images.needle.style.position = "absolute";
	this.images.needle.style.left = this.images.needle.style.top = "33.7%";
	this.images.needle.style.width = "auto";
	this.images.needle.style.height = "32.5%";
	this.images.needle.style.transformOrigin = "83% 83%";
	this.images.needle.style.oTransformOrigin = "83% 83%";
	this.images.needle.style.msTransformOrigin = "83% 83%";
	this.images.needle.style.webkitTransformOrigin = "83% 83%";
	this.images.needle.style.pointerEvents = "none";
	this.arrowDiv.appendChild(this.images.needle);

    // Add gauge image -- static
	this.images.shell = document.createElement("img");
	this.images.shell.src = "/images/gauge_shell.png";
	this.images.shell.style.position = "absolute";
	this.images.shell.style.left = this.images.shell.style.top = "33.7%";
	this.images.shell.style.width = "auto";
	this.images.shell.style.height = "32.5%";
	this.images.shell.style.pointerEvents = "none";
	this.arrowDiv.appendChild(this.images.shell);
	rotate(this.images.wheel,0);
	rotate(this.images.needle,-77.5);

	var downfunc=function(evt)
	{
		if(!_this.dragging)
		{
			_this.dragging=true;
			rotatefunc(evt);
		}
	};

	var upfunc=function()
	{
		_this.dragging=false;
		rotate(_this.images.wheel,0);
		rotate(_this.images.needle,-77.5);
		_this.pilot.power.L=0;
		_this.pilot.power.R=0;
		robot_network.update_pilot(_this.pilot);
	};

	var rotatefunc=function(evt)
	{
		if(_this.dragging)
		{
			var size=
			{
				w:_this.images.wheel.offsetWidth,
				h:_this.images.wheel.offsetHeight
			};

			var pos=
			{
				x:evt.pageX+_this.doorway.manager.div.scrollLeft,
				y:evt.pageY+_this.doorway.manager.div.scrollTop
			};

			var focal_point=getOffset(_this.arrowDiv);
			focal_point.x+=size.w/2+_this.doorway.manager.div.scrollLeft;
			focal_point.y+=size.h/2+_this.doorway.manager.div.scrollTop;

			var rads=Math.PI/2-Math.atan2(focal_point.y-pos.y,pos.x-focal_point.x);
			var degs=rads*180/Math.PI;
			rotate(_this.images.wheel,degs);

			var dist=getDistance(pos,focal_point);
			var max_dist=Math.min(size.w/2,size.h/2);

			if(dist>max_dist)
				dist=max_dist;

			var speed_rot=(65.75/max_dist)*dist-77.5;
			rotate(_this.images.needle,speed_rot);

			var max_power=parseInt(""+_this.get_pilot_power()*100);
			var forward=max_power*(pos.x-focal_point.x)/size.w*2;
			var turn=max_power*(pos.y-focal_point.y)/size.h*2;

			_this.pilot.power.L=parseInt(""+clamp(forward-turn,
				-max_power,max_power));
			_this.pilot.power.R=parseInt(""+clamp(-forward-turn,
				-max_power,max_power));
			robot_network.update_pilot(_this.pilot);
		}
	};
	this.arrowDiv.addEventListener('mousedown', downfunc,true);
	document.addEventListener('mousemove', rotatefunc,true);
	document.addEventListener('mouseup', upfunc,true);

	//Kill power when tab/window isn't in focus.
	window.addEventListener("blur",function()
	{
		_this.pilot.power.L=0;
		_this.pilot.power.R=0;
		robot_network.update_pilot(_this.pilot);
	});
	robot_network.update_pilot(_this.pilot);
}

pilot_interface_t.prototype.update_drive_text=function()
{
	this.drive.label.innerHTML="Drive power ("+this.drive.slider.value+"%):";
}


// Return the drive power the user has currently selected
pilot_interface_t.prototype.get_pilot_power=function()
{
	var maxPower=1.0;

	var powerUI=0.2;
	if (this.drive&&this.drive.slider) powerUI=parseFloat(this.drive.slider.value)*0.01;
	if (isNaN(powerUI)) powerUI=0.2;
	else if (powerUI<maxPower) { maxPower=powerUI; }

	return maxPower;
}

pilot_interface_t.prototype.pilot_keyboard=function()
{
	if(!this.doorway.active||this.dragging)
		return;

	var maxPower=this.get_pilot_power();

	//Return true if this key (as a string) is pressed
	var keyInput=this.keyInput;
	var keyDown=function(key,alternateKey)
	{
		var code=key.charCodeAt(0);
		if(keyInput.keys_down[code])
			return true;

		//hacky recursion
		if(alternateKey)
			return keyDown(alternateKey);
		return false;
	}

	var forward=0.0;
	var turn=0.0;
	if(keyDown('a','A'))
		turn-=1.0;
	if(keyDown('d','D'))
		turn+=1.0;
	if(keyDown('s','S'))
		forward-=1.0;
	if(keyDown('w','W'))
		forward+=1.0;
	if(keyDown(' '))
		turn=forward=0.0;
	if(!keyDown('a','A')&&!keyDown('d','D')&&!keyDown('s','S')&&!keyDown('w','W'))
		forward=turn=0;

	this.keyboardIsDriving=(turn!=0.0||forward!=0.0);

	//steering wheel rotation
	//upper half cases
	if(forward>0||(forward>0&&turn!=0))
		dir=turn*45;

	//lower half cases
	else if(forward<0||(forward<0&&turn!=0))
		dir=180-turn*45;

	//side cases
	else if(forward==0&&turn!=0)
		dir=turn*90;

	//starting & ending point
	else
		dir=0;

	if(this.images&&this.images.wheel)
		rotate(this.images.wheel,dir);

	//spedometer needle rotation
	var ang = maxPower * 65.75 - 77.5;
	if(!this.keyboardIsDriving)
		ang=-77.5;
	if(this.images&&this.images.needle)
		rotate(this.images.needle, ang);

	this.pilot.power.L=100.0*clamp(maxPower*(forward+turn),-maxPower,+maxPower);
	this.pilot.power.R=100.0*clamp(maxPower*(forward-turn),-maxPower,+maxPower);
	robot_network.update_pilot(this.pilot);
}


pilot_interface_t.prototype.download=function(robot)
{
}

pilot_interface_t.prototype.upload=function(robot)
{
	robot_network.update_pilot(this.pilot);
}

// This is a simple placeholder, to get things working for now:
pilot_interface_t.prototype.pilot_send=function()
{
	if(this.onpilot)
		this.onpilot(this.pilot);
};

pilot_interface_t.prototype.handle_gamepad_input=function()
{
	var axes=navigator.getGamepads()[0].axes;
	this.pilot.power.L=0;
	this.pilot.power.R=0;

	if(axes[0]<-0.1)
		this.pilot.power.R-=axes[0];
	if(axes[0]>0.1)
		this.pilot.power.L+=axes[0];
	if(axes[1]<-0.1)
	{
		this.pilot.power.L-=axes[1];
		this.pilot.power.R-=axes[1];
	}
	if(axes[1]>0.1)
	{
		this.pilot.power.L-=axes[1];
		this.pilot.power.R-=axes[1];
	}
	this.pilot.power.L*=50;
	this.pilot.power.R*=50;
	robot_network.update_pilot(this.pilot);
}












/*
  This is the real version, with network spam prevention and authentication and such.

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
