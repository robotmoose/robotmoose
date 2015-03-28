var url_path="/"; // "http://robotmoose.com/";

function load_js(js)
{
	var scr=document.createElement("script");
	scr.src=js;
	document.head.appendChild(scr);
};

function load_dependencies()
{
	load_js(url_path+"js/processing/processing-1.4.1.min.js");
};

(function(){load_dependencies()})();

function arduino_roomba_sensor_t()
{
	var myself=this;
	myself.mode;
	myself.bumper_drop;
	myself.charge_state;
	myself.batt_temp;
	myself.batt_voltage;
	myself.batt_charge;
	myself.batt_capacity;
	myself.encoder_l;
	myself.encoder_r;
	myself.cliff_l;
	myself.cliff_fl;
	myself.cliff_fr;
	myself.cliff_r;
	myself.light_l;
	myself.light_fl;
	myself.light_cl;
	myself.light_cr;
	myself.light_fr;
	myself.light_r;
	myself.light_field;
	myself.buttons;
};

function arduino_roomba_t(controller,serial)
{
	var myself=this;

	myself.controller=controller;
	myself.controller.roombas[serial]=myself;
	myself.serial_m=serial;
	myself.leds_m=0;
	myself.led_clean_color_m=0;
	myself.led_clean_brightness_m=0;
	myself.sensor_packet_m=new arduino_roomba_sensor_t();
	myself.left=0;
	myself.right=0;

	myself.start=function()
	{
		var json_original={"counter":myself.controller.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.controller.commands[myself.controller.commands.length]=function()
		{
			console.log("Roomba start");

			if(json.counter+1<myself.controller.commands.length)
				myself.controller.commands[json.counter+1]();
		};
	};

	myself.stop=function()
	{
		var json_original={"counter":myself.controller.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.controller.commands[myself.controller.commands.length]=function()
		{
			console.log("Roomba stop");

			if(json.counter+1<myself.controller.commands.length)
				myself.controller.commands[json.counter+1]();
		};
	};

	myself.reset=function()
	{
		var json_original={"counter":myself.controller.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.controller.commands[myself.controller.commands.length]=function()
		{
			console.log("Roomba reset");

			if(json.counter+1<myself.controller.commands.length)
				myself.controller.commands[json.counter+1]();
		};
	};

	myself.update=function()
	{
		var json_original={"counter":myself.controller.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.controller.commands[myself.controller.commands.length]=function()
		{
			console.log("Roomba update");

			if(json.counter+1<myself.controller.commands.length)
				myself.controller.commands[json.counter+1]();
		};
	};

	myself.set_mode=function(mode)
	{
		var json_original={"mode":mode,"counter":myself.controller.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.controller.commands[myself.controller.commands.length]=function()
		{
			console.log("Roomba set_mode "+json.mode);

			if(json.counter+1<myself.controller.commands.length)
				myself.controller.commands[json.counter+1]();
		};
	};

	myself.drive=function(left,right)
	{
		var json_original={"left":left,"right":right,"counter":myself.controller.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.controller.commands[myself.controller.commands.length]=function()
		{
			myself.left=json.left;
			myself.right=json.right;

			if(json.counter+1<myself.controller.commands.length)
				myself.controller.commands[json.counter+1]();
		};
	};

	myself.set_led_check=function(on)
	{
		var json_original={"on":on,"counter":myself.controller.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.controller.commands[myself.controller.commands.length]=function()
		{
			console.log("Roomba set_led_check "+json.on);

			if(json.counter+1<myself.controller.commands.length)
				myself.controller.commands[json.counter+1]();
		};
	};

	myself.set_led_dock=function(on)
	{
		var json_original={"on":on,"counter":myself.controller.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.controller.commands[myself.controller.commands.length]=function()
		{
			console.log("Roomba set_led_dock "+json.on);

			if(json.counter+1<myself.controller.commands.length)
				myself.controller.commands[json.counter+1]();
		};
	};

	myself.set_led_spot=function(on)
	{
		var json_original={"on":on,"counter":myself.controller.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.controller.commands[myself.controller.commands.length]=function()
		{
			console.log("Roomba set_led_spot "+json.on);

			if(json.counter+1<myself.controller.commands.length)
				myself.controller.commands[json.counter+1]();
		};
	};

	myself.set_led_debris=function(on)
	{
		var json_original={"on":on,"counter":myself.controller.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.controller.commands[myself.controller.commands.length]=function()
		{
			console.log("Roomba set_led_debris "+json.on);

			if(json.counter+1<myself.controller.commands.length)
				myself.controller.commands[json.counter+1]();
		};
	};

	myself.set_led_clean=function(color,brightness)
	{
		var json_original={"color":color,"brightness":brightness,"counter":myself.controller.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.controller.commands[myself.controller.commands.length]=function()
		{
			console.log("Roomba set_led_clean "+json.color+" "+json.brightness);

			if(json.counter+1<myself.controller.commands.length)
				myself.controller.commands[json.counter+1]();
		};
	};

	myself.led_update=function()
	{
		var json_original={"counter":myself.controller.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.controller.commands[myself.controller.commands.length]=function()
		{
			console.log("Roomba led_update");

			if(json.counter+1<myself.controller.commands.length)
				myself.controller.commands[json.counter+1]();
		};
	};

	myself.set_7_segment=function(text)
	{
		var json_original={"text":text,"counter":myself.controller.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.controller.commands[myself.controller.commands.length]=function()
		{
			console.log("Roomba set_7_segment "+json.text);

			if(json.counter+1<myself.controller.commands.length)
				myself.controller.commands[json.counter+1]();
		};
	};

	myself.play_song=function(number)
	{
		var json_original={"number":number,"counter":myself.controller.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.controller.commands[myself.controller.commands.length]=function()
		{
			console.log("Roomba play_song "+json.number);

			if(json.counter+1<myself.controller.commands.length)
				myself.controller.commands[json.counter+1]();
		};
	};

	myself.set_receive_sensors=function(on)
	{
		var json_original={"on":on,"counter":myself.controller.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.controller.commands[myself.controller.commands.length]=function()
		{
			console.log("Roomba set_receive_sensors "+json.on);

			if(json.counter+1<myself.controller.commands.length)
				myself.controller.commands[json.counter+1]();
		};
	};

	myself.get_sensors=function()
	{
		console.log("Roomba get_sensors");
		var temp=JSON.stringify(myself.sensor_packet_m);
		return JSON.parse(temp);
	};

	myself.dump_sensors=function()
	{
		var json_original={"sensor_packet":myself.sensor_packet_m,
				"counter":myself.controller.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.controller.commands[myself.controller.commands.length]=function()
		{
			//json.sensor_packet
			console.log("Roomba dump_sensors");

			if(json.counter+1<myself.controller.commands.length)
				myself.controller.commands[json.counter+1]();
		};
	};
};

function arduino_servo_t(controller)
{
	var myself=this;
	myself.controller=controller;
	myself.pin=-1;
	myself.pos=0;

	myself.attach=function(pin)
	{
		var json_original={"value":pin,"counter":myself.controller.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.controller.commands[myself.controller.commands.length]=function()
		{
			myself.pin=json.value;
			myself.controller.pin_directions[myself.pin]=3;

			if(json.counter+1<myself.controller.commands.length)
				myself.controller.commands[json.counter+1]();
		};
	};

	myself.write=function(pos)
	{
		var json_original={"value":pos,"counter":myself.controller.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.controller.commands[myself.controller.commands.length]=function()
		{
			myself.pos=Math.max(0,Math.min(180,json.value));
			myself.controller.pin_servos[myself.pin]=myself.controller.map(myself.pos,0,180,0,255);

			if(json.counter+1<myself.controller.commands.length)
				myself.controller.commands[json.counter+1]();
		};
	};

	myself.writeMicroseconds=function(us)
	{
		var json_original={"value":us,"counter":myself.controller.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.controller.commands[myself.controller.commands.length]=function()
		{
			myself.pos=Math.max(0,Math.min(180,myself.map(json.value,544,2400,0,180)));
			myself.controller.pin_servos[myself.pin]=myself.controller.map(myself.pos,0,180,0,255);

			if(json.counter+1<myself.controller.commands.length)
				myself.controller.commands[json.counter+1]();
		};
	};

	myself.read=function()
	{
		return myself.pos;
	};

	myself.attached=function()
	{
		return (myself.pin>=0&&myself.pin<myself.controller.pin_count);
	};

	myself.detach=function()
	{
		var json_original={"counter":myself.controller.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.controller.commands[myself.controller.commands.length]=function()
		{
			myself.controller.pin_directions[myself.pin]=1;
			myself.controller.pin_servos[myself.pin]=0;
			myself.pin=-1;

			if(json.counter+1<myself.controller.commands.length)
				myself.controller.commands[json.counter+1]();
		};
	};
};

function arduino_emulator_t()
{
	var myself=this;
	myself.commands=new Array();
	myself.json={};
	myself.pin_count=19;
	myself.pin_directions=new Array();
	myself.pin_outputs=new Array();
	myself.pin_inputs=new Array();
	myself.pin_pwms=new Array();
	myself.pin_analogs=new Array();
	myself.pin_servos=new Array();
	myself.roombas={};

	for(var ii=0;ii<myself.pin_count;++ii)
	{
		myself.pin_directions[ii]=1;
		myself.pin_outputs[ii]=false;
		myself.pin_inputs[ii]=false;
		myself.pin_pwms[ii]=0;
		myself.pin_analogs[ii]=0;
		myself.pin_servos[ii]=0;
	}

	myself.Serial=
	{
		available:function(){return false;},
		begin:function(speed,config){},
		end:function(){},
		flush:function(){},
		peek:function(){return 0;},

		print:function(text)
		{
			var json_original={"value":text,"counter":myself.commands.length};
			var json=JSON.parse(JSON.stringify(json_original));

			myself.commands[myself.commands.length]=function()
			{
				console.log(json.value);

				if(myself.pin_directions[pin]==0)
				{
					myself.pin_pwms[pin]=0;

					if(value>=0&&value<=255)
						myself.pin_pwms[pin]=value;
				}
			};
		},

		println:function(text)
		{
			var json_original={"value":text,"counter":myself.commands.length};
			var json=JSON.parse(JSON.stringify(json_original));

			myself.commands[myself.commands.length]=function()
			{
				console.log(json.value);

				if(json.counter+1<myself.commands.length)
					myself.commands[json.counter+1]();
			};
		},

		read:function(){return 0},

		write:function(text)
		{
			var json_original={"value":text,"counter":myself.commands.length};
			var json=JSON.parse(JSON.stringify(json_original));

			myself.commands[myself.commands.length]=function()
			{
				console.log(json.value);

				if(json.counter+1<myself.commands.length)
					myself.commands[json.counter+1]();
			};
		}
	};

	myself.compile=function(code,error_handler)
	{
		(function()
		{
			try
			{
				myself.json.controller=myself;
				code=code.replace(/\#\s*include/ig,"//#include");
				code=code.replace(/\#\s*if/ig,"//#if");
				code=code.replace(/\#\s*define/ig,"//#define");
				code=code.replace(/\#\s*else/ig,"//#else");
				code=code.replace(/\#\s*endif/ig,"//#endif");
				code=code.replace(/Servo\s+(.*);/g,"var $1=new arduino_servo_t(get_controller());");
				code=code.replace(/roomba_t\s+(.*)\(\s*(.*)\s*\)\s*;/g,"var $1=new arduino_roomba_t(get_controller(),\"$2\");");
				//alert(code);

				this.OUTPUT=0;
				this.INPUT=1;
				this.INPUT_PULLUP=2;
				this.HIGH=true;
				this.LOW=false;
				this.Serial=myself.Serial;
				this.constrain=myself.constrain;
				this.map=myself.map;
				this.pinMode=myself.pinMode;
				this.digitalWrite=myself.digitalWrite;
				this.digitalRead=myself.digitalRead;
				this.analogWrite=myself.analogWrite;
				this.analogRead=myself.analogRead;
				this.get_controller=function(){return myself;};

				eval(Processing.compile(code).sourceCode)(myself.json);

				myself.json.delay=myself.delay;

				myself.json.sqrt=Math.sqrt;
				myself.json.min=Math.min;
				myself.json.max=Math.max;
				myself.json.abs=Math.abs;
				myself.json.pow=Math.pow;
				myself.json.constrain=constrain;
				myself.json.map=map;

				myself.commands.length=0;
				myself.json.setup();

				myself.commands[myself.commands.length]=function()
				{
					myself.commands.length=0;
					myself.json.loop();

					if(myself.commands.length>0)
						myself.commands[0]();

					myself.commands[myself.commands.length]=function()
					{
						if(myself.commands.length>0)
							myself.commands[0]();
					};
				};

				if(myself.commands.length>0)
					myself.commands[0]();
			}
			catch(e)
			{
				console.log("Arduino simulator error "+e);
				error_handler("Invalid code: "+e);
			}
		})();
	};

	myself.constrain=function(consNum,lowNum,highNum)
	{
		var numCons=0;
		if(consNum>lowNum&&consNum<highNum)
			numCons=consNum;
		else if(consNum<lowNum)
			numCons=lowNum;
		else if(consNum>highNum)
			numCons=highNum;

		return numCons;
	};

	myself.map=function(value,fromLow,fromHigh,toLow,toHigh)
	{
		return (value-fromLow)*(toHigh-toLow)/(fromHigh-fromLow)+toLow;
	};

	myself.pinMode=function(pin,direction)
	{
		var json_original={"counter":myself.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.commands[myself.commands.length]=function()
		{
			myself.pin_directions[pin]=direction;

			if(json.counter+1<myself.commands.length)
				myself.commands[json.counter+1]();
		};
	};

	myself.digitalWrite=function(pin,value)
	{
		var json_original={"value":value,"counter":myself.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.commands[myself.commands.length]=function()
		{
			if(myself.pin_directions[pin]==0)
				myself.pin_outputs[pin]=(value!=false&&value!=null);

			if(json.counter+1<myself.commands.length)
				myself.commands[json.counter+1]();
		};
	};

	myself.digitalRead=function(pin)
	{
		if(myself.pin_directions[pin]==0)
			return myself.pin_outputs[pin];

		if(myself.pin_directions[pin]==1||myself.pin_directions[pin]==2)
			return myself.pin_inputs[pin];

		return 0;
	};

	myself.analogWrite=function(pin,value)
	{
		var json_original={"value":value,"counter":myself.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.commands[myself.commands.length]=function()
		{
			if(myself.pin_directions[pin]==0)
				myself.pin_outputs[pin]=(value!=false&&value!=null);

			if(myself.pin_directions[pin]==0)
			{
				myself.pin_pwms[pin]=0;

				if(value>=0&&value<=255)
					myself.pin_pwms[pin]=value;
			}
		};
	};

	myself.analogRead=function(pin)
	{
		if(myself.pin_directions[pin]==1||myself.pin_directions[pin]==2)
			return myself.pin_analogs[pin];

		return 0;
	};

	myself.delay=function(time)
	{
		var json_original={"time":time,"counter":myself.commands.length};
		var json=JSON.parse(JSON.stringify(json_original));

		myself.commands[myself.commands.length]=function()
		{
			setTimeout(function(){
				if(json.counter+1<myself.commands.length)
					myself.commands[json.counter+1]();
				},json.time);
		};
	}

	myself.stop=function()
	{
		myself.commands.length=0;
	};

	myself.running=function()
	{
		return (myself.commands.length>0);
	};
};
