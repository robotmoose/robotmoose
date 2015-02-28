function load_js(js)
{
	var scr=document.createElement("script");
	scr.src=js;
	document.head.appendChild(scr);
};

function load_dependencies()
{
	load_js("js/processing/processing-1.4.1.min.js");
};

(function(){load_dependencies()})();

function arduino_emulator_t()
{
	var myself=this;
	myself.json={};
	myself.pin_count=19;
	myself.pin_directions=new Array();
	myself.pin_outputs=new Array();
	myself.pin_inputs=new Array();
	myself.pin_pwms=new Array();
	myself.pin_analogs=new Array();

	for(var ii=0;ii<myself.pin_count;++ii)
	{
		myself.pin_directions[ii]=1;
		myself.pin_outputs[ii]=false;
		myself.pin_inputs[ii]=false;
		myself.pin_pwms[ii]=0;
		myself.pin_analogs[ii]=0;
	}

	myself.Serial=
	{
		available:function(){return false;},
		begin:function(speed,config){},
		end:function(){},
		flush:function(){},
		peek:function(){return 0;},
		print:function(text){console.log(text);},
		println:function(text){console.log(text+"\n");},
		read:function(){return 0},
		write:function(text){console.log(text);}
	};

	myself.compile=function(code)
	{
		(function()
		{
			try
			{
				code=code.replace(/\#\s+include/ig,"//#include");
				code=code.replace(/\#\s+if/ig,"//#if");
				code=code.replace(/\#\s+ifdef/ig,"//#ifdef");
				code=code.replace(/\#\s+ifndef/ig,"//#ifndef");
				code=code.replace(/\#\s+else/ig,"//#else");
				code=code.replace(/\#\s+endif/ig,"//#endif");

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

				eval(Processing.compile(code).sourceCode)(myself.json);

				myself.json.sqrt=Math.sqrt;
				myself.json.min=Math.min;
				myself.json.max=Math.max;
				myself.json.abs=Math.abs;
				myself.json.pow=Math.pow;
				myself.json.constrain=constrain;
				myself.json.map=map;
			}
			catch(e)
			{
				console.log(e);
			}
		})();

		myself.json.setup();
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
		myself.pin_directions[pin]=direction;
	};

	myself.digitalWrite=function(pin,value)
	{
		if(myself.pin_directions[pin]==0)
			myself.pin_outputs[pin]=(value!=false&&value!=null);
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
		if(myself.pin_directions[pin]==0)
		{
			myself.pin_pwms[pin]=0;

			if(value>=0&&value<=255)
				myself.pin_pwms[pin]=value;
		}
	};

	myself.analogRead=function(pin)
	{
		if(myself.pin_directions[pin]==1||myself.pin_directions[pin]==2)
			return myself.pin_analogs[pin];

		return 0;
	};
};