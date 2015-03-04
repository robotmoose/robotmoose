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
	
	myself.compile=function(code, error_handler)
	{
		(function()
		{
			try
			{
				myself.json.controller=myself;
				code=code.replace(/\#include/ig,"//#include");
				code=code.replace(/\#\s+include/ig,"//#include");
				code=code.replace(/\#if/ig,"//#if");
				code=code.replace(/\#\s+if/ig,"//#if");
				code=code.replace(/\#ifdef/ig,"//#ifdef");
				code=code.replace(/\#\s+ifdef/ig,"//#ifdef");
				code=code.replace(/\#ifndef/ig,"//#ifndef");
				code=code.replace(/\#\s+ifndef/ig,"//#ifndef");
				code=code.replace(/\#else/ig,"//#else");
				code=code.replace(/\#\s+else/ig,"//#else");
				code=code.replace(/\#endif/ig,"//#endif");
				code=code.replace(/\#\s+endif/ig,"//#endif");
				code=code.replace(/\Servo\s(.*)[;]/ig,"var $1=new arduino_servo_t(get_controller());");

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
