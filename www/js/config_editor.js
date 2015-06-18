function load_js(js)
{
	var scr=document.createElement("script");
	scr.src=js;
	document.head.appendChild(scr);
};

function load_link(link)
{
	var ln=document.createElement("link");
	ln.rel="stylesheet";
	ln.href=link;
	document.head.appendChild(ln);
};

function load_style(style)
{
	var sty=document.createElement("style");
	sty.appendChild(document.createTextNode(style));
	document.head.appendChild(sty);
};

function load_dependencies()
{
	load_js("/js/xmlhttp.js");
	load_js("/js/jquery/jquery.min.js");
	load_js("/js/codemirror/codemirror.js");
	load_js("/js/bootstrap/bootstrap.min.js");
	load_link("/css/bootstrap.min.css");
	load_js("/js/codemirror/clike_arduino_nxt.js");
	load_js("/js/codemirror/addon/edit/matchbrackets.js");
	load_js("/js/codemirror/addon/dialog/dialog.js");
	load_js("/js/codemirror/addon/search/search.js");
	load_js("/js/codemirror/addon/search/searchcursor.js");
	load_link("/js/codemirror/codemirror.css");
	load_link("/js/codemirror/addon/dialog/dialog.css");
	load_style(".CodeMirror{border:1px solid #000000;}");
	load_style(".lint-error{background:#ff8888;color:#a00000;padding:1px}\r\n.lint-error-icon{background:#ff0000;color:#ffffff;border-radius:50%;margin-right:7px;}");
	load_link("/js/jquery/jquery.sortable.css");
	load_js("/js/jquery/jquery.sortable.min.js");
};

(function(){load_dependencies()})();

function config_editor_t(div,robot_name)
{
	if(!div)
		return null;

	this.div=div;
	this.config="";
	this.options=new Array();
	this.nonce=0;
	this.robot_name=robot_name;

	this.get_config();
	this.get_options();
}

config_editor_t.prototype.get_config=function()
{
	var myself=this;

	send_request("GET","/superstar/"+this.robot_name,"config","?get",
		function(response)
		{
			try
			{
				var json=JSON.parse(response);
				myself.config="";

				for(var ii=0;ii<json.configs.length;++ii)
					myself.config+=json.configs[ii]+"\n";

				if(myself.onconfigchange)
					myself.onconfigchange(myself.config);
			}
			catch(error)
			{
				console.log("config_editor_t::get_config() - "+error);
				setInterval(1000,function(){myself.get_config();});
			}
		},
		function(error)
		{
			console.log("config_editor_t::get_config() - "+error);
			setInterval(1000,function(){myself.get_config();});
		},
		"application/json");
}

config_editor_t.prototype.get_options=function()
{
	var myself=this;

	send_request("GET","/superstar/"+this.robot_name,"options","?get",
		function(response)
		{
			try
			{
				var json=JSON.parse(response);

				for(var ii=0;ii<json.length;++ii)
				{
					var parts=json[ii].split(" ");

					if(parts.length!=2)
						throw "Invalid tabula option \""+json[ii]+"\".";

					var tabula={};
					tabula.type=parts[0];
					tabula.args=new Array();

					for(var jj=0;jj<parts[1].length;++jj)
						tabula.args[jj]=parts[1][jj];

					myself.options.push(tabula);
				}
			}
			catch(error)
			{
				console.log("config_editor_t::get_options() - "+error);
				setInterval(1000,function(){myself.get_options();});
			}
		},
		function(error)
		{
			console.log("config_editor_t::get_options() - "+error);
			setInterval(1000,function(){myself.get_options();});
		},
		"application/json");
}

config_editor_t.prototype.configure=function(config_text)
{
	try
	{
		var configs=new Array();

		if(config_text.length>0)
		{
			configs=this.lex(config_text);
			this.validate(configs);
		}

		var validated_configs=new Array();
		var config_json={counter:this.nonce++,configs:new Array()};

		for(var ii=0;ii<configs.length;++ii)
			config_json.configs.push(configs[ii].type+"("+configs[ii].args+");");

		var myself=this;

		send_request("GET","/superstar/"+this.robot_name,"config","?set="+JSON.stringify(config_json),
			function(response)
			{
				myself.config=config_text;
			},
			function(error)
			{
				throw "config_editor_t::configure() - "+error;
			},
			"application/json");
	}
	catch(e)
	{
		console.log(e);
	}
}

function parse_identifier(str)
{
	var identifier="";

	if(str.length>0&&!((str[0]>='a'&&str[0]<='z')||str[0]=='_'))
		return "";

	for(var ii=0;ii<str.length;++ii)
	{
		if((str[ii]>='a'&&str[ii]<='z')||(str[ii]>='0'&&str[ii]<='9')||str[ii]=='_')
			identifier+=str[ii];
		else
			break;
	}

	return identifier;
}

function parse_argument(str)
{
	var argument="";

	for(var ii=0;ii<str.length;++ii)
	{
		if((str[ii]>='a'&&str[ii]<='z')||(str[ii]>='0'&&str[ii]<='9'))
			argument+=str[ii];
		else
			break;
	}

	return argument;
}

function parse_symbol(str,symbol)
{
	return (str.length>0&&str[0]==symbol);
}

function skip_whitespace(str,col,line)
{
	while(str.length>0&&(str[0]==' '||str[0]=='\t'||str[0]=='\n'||str[0]=='\r'))
	{
		if(str[0]=='\n'||str[0]=='\r')
		{
			col=0;
			++line;
		}
		else
		{
			++col;
		}

		str=str.substring(1,str.length);
	}

	return {str:str,col:col,line:line};
}

config_editor_t.prototype.lex=function(config)
{
	var col=0;
	var line=0;
	var temp;
	var copy=config.toLowerCase();
	var configs=new Array();

	while(copy.length>0)
	{
		temp=skip_whitespace(copy,col,line);
		copy=temp.str;
		col=temp.col;
		line=temp.line;

		var type=parse_identifier(copy);
		var args=new Array();

		if(type.length==0)
			throw "Line: "+line+" Col: "+col+" - Invalid device type!";

		copy=copy.substring(type.length,copy.length);
		col+=type.length;

		temp=skip_whitespace(copy,col,line);
		copy=temp.str;
		col=temp.col;
		line=temp.line;

		if(!parse_symbol(copy,'('))
			throw "Line: "+line+" Col: "+col+" - Expected '('!";

		copy=copy.substring(1,copy.length);
		++col;

		var arg="";
		var saw_comma=false;

		while(true)
		{
			temp=skip_whitespace(copy,col,line);
			copy=temp.str;
			col=temp.col;
			line=temp.line;

			arg=parse_argument(copy);

			if(arg.length>0)
				args.push(arg);

			copy=copy.substring(arg.length,copy.length);
			col+=arg.length;

			temp=skip_whitespace(copy,col,line);
			copy=temp.str;
			col=temp.col;
			line=temp.line;

			if(parse_symbol(copy,')'))
			{
				if(saw_comma&&arg.length<=0)
					throw "Line: "+line+" Col: "+col+" - Unexpected ','.";
				else
					break;
			}

			temp=skip_whitespace(copy,col,line);
			copy=temp.str;
			col=temp.col;
			line=temp.line;

			if(arg.length=="")
			{
				if(parse_identifier(copy).length>0)
					throw "Line: "+line+" Col: "+col+" - Invalid argument!";

				throw "Line: "+line+" Col: "+col+" - Expected argument!";
			}

			if(!parse_symbol(copy,','))
				throw "Line: "+line+" Col: "+col+" - Expected ',' or ')'!";
			else
				saw_comma=true;

			copy=copy.substring(1,copy.length);
			++col;
		}

		if(!parse_symbol(copy,')'))
			throw "Line: "+line+" Col: "+col+" - Expected ')'!";

		copy=copy.substring(1,copy.length);
		++col;

		temp=skip_whitespace(copy,col,line);
		copy=temp.str;
		col=temp.col;
		line=temp.line;

		if(!parse_symbol(copy,';'))
			throw "Line: "+line+" Col: "+col+" - Expected ';'!";

		copy=copy.substring(1,copy.length);
		++col;

		temp=skip_whitespace(copy,col,line);
		copy=temp.str;
		col=temp.col;
		line=temp.line;

		configs.push({"type":type,"args":args});
	}

	return configs;
}

config_editor_t.prototype.is_int=function(str)
{
	for(var ii=0;ii<str.length;++ii)
		if(str[ii]<'0'||str[ii]>'9')
			return false;

	return true;
}

config_editor_t.prototype.is_pin=function(arg)
{
	arg=arg.toLowerCase();

	return ((arg.length>0&&this.is_int(arg))||
		(arg.length>0&&arg[0]=='a'&&this.is_int(arg.substr(1,arg.length-1))));
}

config_editor_t.prototype.is_serial=function(arg)
{
	arg=arg.toLowerCase();

	if(arg.length>0&&arg[0]=='x'&&this.is_int(arg.substr(1,arg.length-1)))
	{
		var port=parseInt(arg.substr(1,arg.length-1));
		return (port>=0&&port<=3);
	}

	return false;
}

config_editor_t.prototype.arrays_equal=function(lhs,rhs)
{
	if(!lhs||!rhs||lhs.length!=rhs.length)
		return false;

	for(var ii=0;ii<lhs.length;++ii)
		if(lhs[ii]!=rhs[ii])
			return false;

	return true;
}

config_editor_t.prototype.validate=function(configs)
{
	for(var ii=0;ii<configs.length;++ii)
	{
		var arg_types=new Array();

		for(var jj=0;jj<configs[ii].args.length;++jj)
		{
			if(this.is_pin(configs[ii].args[jj]))
				arg_types.push("P");
			else if(this.is_serial(configs[ii].args[jj]))
				arg_types.push("S");
			else
				throw "config_editor_t::validate - Invalid Argument type \""+configs[ii].args[jj]+"\".";
		}

		var matched_name=false;
		var matched=false;
		var possible_args=new Array();

		for(var jj=0;jj<this.options.length;++jj)
		{
			if(configs[ii].type==this.options[jj].type)
			{
				matched_name=true;
				possible_args.push(this.options[jj].args);
			}

			if(matched_name&&this.arrays_equal(arg_types,this.options[jj].args))
			{
				matched=true;
				break;
			}
		}

		if(!matched)
		{
			if(matched_name)
			{
				var error="config_editor_t::validate - Invalid argument list for device "+
					configs[ii].type+", got ";

				if(arg_types.length==0)
					error+="null";
				else
					error+=arg_types;

				error+="; expected:\n";

				for(var jj=0;jj<possible_args.length;++jj)
					error+="    "+possible_args[jj]+"\n";

				throw error;
			}

			throw "config_editor_t::validate - Invalid device type "+configs[ii].type+".";
		}
	}
}

function config_cli_t(div,robot_name)
{
	this.editor=new config_editor_t(div,robot_name);

	if(!this.editor)
		return null;

	this.textarea=document.createElement("textarea");
	this.textarea.innerHTML=this.editor.config;
	this.editor.div.appendChild(this.textarea);

	this.code_editor=CodeMirror.fromTextArea(this.textarea,
		{indentUnit:4,indentWithTabs:true,lineNumbers:true,matchBrackets:true,mode:"text/x-arduino"});
	this.code_editor.setSize(320,240);

	this.break0=document.createElement("br");
	this.editor.div.appendChild(this.break0);

	this.button=document.createElement("input");
	this.button.type="button";
	this.button.value="Configure";
	this.button.className="btn btn-sm btn-primary";

	var myself=this;
	this.button.onclick=function(){myself.editor.configure(myself.code_editor.getValue());};
	this.editor.div.appendChild(this.button);

	this.editor.onconfigchange=function(config_text){myself.update(config_text);};
}

config_cli_t.prototype.update=function(config_text)
{
	this.code_editor.setValue(config_text);
}

function config_gui_t(div,robot_name)
{
	this.editor=new config_editor_t(div,robot_name);

	if(!this.editor)
		return null;

	this.list=document.createElement("ul");
	this.list.className="sortable";
	this.editor.div.appendChild(this.list);

	$("ul.sortable").sortable();

	this.break0=document.createElement("br");
	this.editor.div.appendChild(this.break0);

	this.button=document.createElement("input");
	this.button.type="button";
	this.button.value="Configure";
	this.button.className="btn btn-sm btn-primary";

	var myself=this;
	this.button.onclick=function(){myself.editor.configure(myself.get_value());};
	this.editor.div.appendChild(this.button);

	this.editor.onconfigchange=function(config_text){myself.update(config_text);};
}

config_gui_t.prototype.get_value=function()
{
	var config_text="";

	try
	{
		for(var ii=0;ii<this.list.children.length;++ii)
		{
			var arg_count=0;
			var child=this.list.children[ii];
			config_text+=child.tabula.type+"(";

			for(var jj=0;jj<child.children.length;++jj)
			{
				if(child.children[jj].tagName=="SELECT")
				{
					var select=child.children[jj];
					var value=select.options[select.selectedIndex].value;

					if(child.tabula.args[arg_count]=='P'&&!this.editor.is_pin(select.options[select.selectedIndex].value))
						throw select.options[select.selectedIndex].value+" is not a pin!";
					if(child.tabula.args[arg_count]=='S'&&!this.editor.is_serial(select.options[select.selectedIndex].value))
						throw select.options[select.selectedIndex].value+" is not a serial port!";

					config_text+=select.options[select.selectedIndex].value+",";
					++arg_count;
				}
			}

			if(arg_count!=child.tabula.args.length)
				throw "Could not decode gui row to a valid tabula device.";

			if(config_text.length>0&&config_text[config_text.length-1]==',')
				config_text=config_text.substring(0,config_text.length-1);

			config_text+=");\n";
		}
	}
	catch(e)
	{
		console.log("config_gui_t::get_value() - "+e);
	}

	return config_text;
}

config_gui_t.prototype.create_pin_drop=function(value)
{
	var drop=document.createElement("select");

	for(var ii=0;ii<=21;++ii)
	{
		var option=document.createElement("option");
		option.text=""+ii;

		if(value&&option.text.toLowerCase()==value+"")
			option.selected=true;

		drop.add(option);
	}

	for(var ii=0;ii<=15;++ii)
	{
		var option=document.createElement("option");
		option.text="A"+ii;

		if(value&&option.text.toLowerCase()==value+"")
			option.selected=true;

		drop.add(option);
	}

	return drop;
}

config_gui_t.prototype.create_serial_drop=function(value)
{
	var drop=document.createElement("select");

	for(var ii=0;ii<=3;++ii)
	{
		var option=document.createElement("option");
		option.text="X"+ii;

		if(value&&option.text.toLowerCase()==value+"")
			option.selected=true;

		drop.add(option);
	}

	return drop;
}

config_gui_t.prototype.update=function(config_text)
{
	try
	{
		var myself=this;
		var configs=new Array();

		if(config_text.length>0)
		{
			configs=this.editor.lex(config_text);

			for(var ii=0;ii<configs.length;++ii)
			{
				var li=document.createElement("li");
				li.className="list-group-item";
				li.tabula={type:configs[ii].type,args:new Array()};

				var button=document.createElement("img");
				button.src="js/jquery/close.png";
				button.onmouseover=function(){this.src="js/jquery/close_rollover.png";};
				button.onmouseout=function(){this.src="js/jquery/close.png";};
				button.onclick=function(){alert(myself.editor.config);};//FIXME
				li.appendChild(button);

				var text=document.createTextNode(configs[ii].type);
				li.appendChild(text);

				for(var jj=0;jj<configs[ii].args.length;++jj)
				{
					if(this.editor.is_pin(configs[ii].args[jj]))
					{
						li.appendChild(this.create_pin_drop(configs[ii].args[jj]));
						li.tabula.args.push("P");
					}
					else if(this.editor.is_serial(configs[ii].args[jj]))
					{
						li.appendChild(this.create_serial_drop(configs[ii].args[jj]));
						li.tabula.args.push("S");
					}
				}

				this.list.appendChild(li);
			}
		}
	}
	catch(e)
	{
		console.log("config_gui_t::update() - "+e);
	}
}