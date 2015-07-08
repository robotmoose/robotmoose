function config_editor_t(div,robot_name)
{
	if(!div)
		return null;

	this.div=div;
	this.element=document.createElement("div");
	this.element.style.width="100%";
	this.element.style.height="100%";
	this.div.appendChild(this.element);
	this.config="";
	this.options=new Array();
	this.counter=0;
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
				if(response)
				{
					var json=JSON.parse(response);
					myself.config="";

					for(var ii=0;ii<json.configs.length;++ii)
						myself.config+=json.configs[ii]+"\n";

					myself.counter=json.counter+1;

					if(myself.onconfigschange)
						myself.onconfigschange(myself.config);
				}
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
				if(response)
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

					if(myself.onoptionschange)
						myself.onoptionschange(myself.options);
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
		var config_json={counter:this.counter++,configs:new Array()};

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
	catch(error)
	{
		console.log(error);
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

	return ((arg.length>0&&this.is_int(arg)&&arg>1)||
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

config_editor_t.prototype.set_robot_name=function(robot_name)
{
	try
	{
		if(!robot_name)
			throw "config_editor_t::set_robot_name - Invalid robot name."

		this.robot_name=robot_name;
		this.get_config();
		this.get_options();
	}
	catch(error)
	{
		console.log(error);
	}
}

function config_cli_t(div,robot_name)
{
	this.editor=new config_editor_t(div,robot_name);

	if(!this.editor)
		return null;

	this.textarea=document.createElement("textarea");
	this.textarea.innerHTML=this.editor.config;
	this.editor.element.appendChild(this.textarea);

	this.code_editor=CodeMirror.fromTextArea(this.textarea,
		{indentUnit:4,indentWithTabs:true,lineNumbers:true,matchBrackets:true,mode:"text/x-arduino"});
	this.code_editor.setSize(320,240);

	this.break0=document.createElement("br");
	this.editor.element.appendChild(this.break0);

	this.configure_button=document.createElement("input");
	this.configure_button.type="button";
	this.configure_button.value="Configure";
	this.configure_button.className="btn btn-primary";

	var myself=this;
	this.configure_button.onclick=function(){myself.editor.configure(myself.code_editor.getValue());};
	this.editor.element.appendChild(this.configure_button);

	this.editor.onconfigschange=function(config_text){myself.update_configs(config_text);};
}

config_cli_t.prototype.update_configs=function(config_text)
{
	this.code_editor.setValue(config_text);
}

config_cli_t.prototype.set_robot_name=function(robot_name)
{
	try
	{
		if(!robot_name)
			throw "config_cli_t::set_robot_name - Invalid robot name."

		this.textarea.value="";
		this.editor.set_robot_name(robot_name);
	}
	catch(error)
	{
		console.log(error);
	}
}

function config_gui_t(div,robot_name)
{
	var myself=this;
	this.editor=new config_editor_t(div,robot_name);

	if(!this.editor)
		return null;

	this.editor.onconfigschange=function(config_text){myself.update_configs(config_text);};
	this.editor.onoptionschange=function(options){myself.update_options(options);};

	this.configs_list_prettifier=document.createElement("div");
	this.configs_list_prettifier.className="form-inline";

	this.configs_list=document.createElement("ul");
	this.configs_list.className="sortable";

	this.break0=document.createElement("br");

	this.configure_button=document.createElement("input");
	this.configure_button.type="button";
	this.configure_button.value="Configure";
	this.configure_button.className="btn btn-primary";
	this.configure_button.onclick=function(){myself.configure()};

	this.break1=document.createElement("br");

	this.adder_prettifier=document.createElement("div");
	this.adder_prettifier.className="form-inline";

	this.adder_select=document.createElement("select");
	this.adder_select.className="form-control";
	this.adder_select.onchange=function(){myself.adder_button_update();};

	var adder_title_option=document.createElement("option");
	adder_title_option.text="Add Device";
	this.adder_select.appendChild(adder_title_option);

	this.adder_button=document.createElement("input");
	this.adder_button.type="button";
	this.adder_button.value="Add";
	this.adder_button.disabled=true;
	this.adder_button.className="btn btn-primary";
	this.adder_button.style.marginLeft=10;
	this.adder_button.onclick=function()
	{
		var option=myself.adder_select.options[myself.adder_select.selectedIndex];
		myself.create_row(option.text,option.args,true);
	};

	this.configs_list_prettifier.appendChild(this.configs_list);
	this.adder_prettifier.appendChild(this.adder_select);
	this.adder_prettifier.appendChild(this.adder_button);
	this.editor.element.appendChild(this.configs_list_prettifier);
	this.editor.element.appendChild(this.break0);
	this.editor.element.appendChild(this.adder_prettifier);
	this.editor.element.appendChild(this.break1);
	this.editor.element.appendChild(this.configure_button);

	$("ul.sortable").sortable();
}

config_gui_t.prototype.configure=function()
{
	this.editor.configure(this.get_value());
}

config_gui_t.prototype.configure_button_update=function()
{
	var ok=true;

	for(var ii=0;ii<this.configs_list.children.length;++ii)
	{
		var child=this.configs_list.children[ii];

		for(var jj=0;jj<child.children.length;++jj)
		{
			var grandchild=child.children[jj];

			if(grandchild.tagName=="SELECT"&&grandchild.selectedIndex==0)
			{
				ok=false;
				break;
			}
		}

		if(!ok)
			break;
	}

	this.configure_button.disabled=!ok;
}

config_gui_t.prototype.adder_button_update=function()
{
	this.adder_button.disabled=this.adder_select.selectedIndex==0;
}

config_gui_t.prototype.get_value=function()
{
	var config_text="";

	try
	{
		for(var ii=0;ii<this.configs_list.children.length;++ii)
		{
			var arg_count=0;
			var child=this.configs_list.children[ii];
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
	catch(error)
	{
		console.log("config_gui_t::get_value() - "+error);
	}

	return config_text;
}

config_gui_t.prototype.create_pin_drop=function(value)
{
	var drop=document.createElement("select");
	drop.style.width=80;
	drop.className="form-control"

	var title_option=document.createElement("option");
	title_option.text="Pin";
	title_option.selected=true;
	drop.add(title_option);

	for(var ii=2;ii<=21;++ii)
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
		{
			title_option.selected=false;
			option.selected=true;
		}

		drop.add(option);
	}

	var myself=this;
	drop.onchange=function(){myself.configure_button_update();};

	return drop;
}

config_gui_t.prototype.create_serial_drop=function(value)
{
	var drop=document.createElement("select");
	drop.style.width=80;
	drop.className="form-control"

	var title_option=document.createElement("option");
	title_option.text="Port";
	title_option.selected=true;
	drop.add(title_option);

	for(var ii=1;ii<=3;++ii)
	{
		var option=document.createElement("option");
		option.text="X"+ii;

		if(value&&option.text.toLowerCase()==value+"")
		{
			title_option.selected=false;
			option.selected=true;
		}

		drop.add(option);
	}

	var myself=this;
	drop.onchange=function(){myself.configure_button_update();};

	return drop;
}

config_gui_t.prototype.create_row=function(type,args,create_new)
{
	var myself=this;

	(function()
	{
		var li=document.createElement("li");
		li.className="list-group-item";
		li.tabula={type:type,args:new Array()};

		var text=document.createTextNode(type+" ");
		li.appendChild(text);

		for(var jj=0;jj<args.length;++jj)
		{
			if(create_new)
			{
				li.tabula.args.push(args[jj]);

				if(args[jj]=="P")
					li.appendChild(myself.create_pin_drop());
				else if(args[jj]=="S")
					li.appendChild(myself.create_serial_drop());
			}
			else
			{
				if(myself.editor.is_pin(args[jj]))
				{
					li.appendChild(myself.create_pin_drop(args[jj]));
					li.tabula.args.push("P");
				}
				else if(myself.editor.is_serial(args[jj]))
				{
					li.appendChild(myself.create_serial_drop(args[jj]));
					li.tabula.args.push("S");
				}
			}
		}

		var button=document.createElement("span");
		button.className="close";
		button.innerHTML="x";
		button.li=li;
		button.onclick=function(){myself.configs_list.removeChild(li);myself.configure_button_update();};
		li.appendChild(button);

		myself.configs_list.appendChild(li);
		myself.configure_button_update();
	})();
}

config_gui_t.prototype.update_configs=function(config_text)
{
	try
	{
		var configs=new Array();

		if(config_text.length>0)
		{
			configs=this.editor.lex(config_text);

			for(var ii=0;ii<configs.length;++ii)
				this.create_row(configs[ii].type,configs[ii].args,false);
		}
	}
	catch(error)
	{
		console.log("config_gui_t::update_configs() - "+error);
	}
}

config_gui_t.prototype.update_options=function(options)
{
	try
	{
		var myself=this;
		myself.adder_select.selectedIndex=0;
		myself.adder_select.options.length=1;

		for(var ii=0;ii<options.length;++ii)
		{
			(function()
			{
				var option=document.createElement("option");
				option.text=options[ii].type;
				option.args=options[ii].args;
				option.args.length=options[ii].args.length;
				myself.adder_select.appendChild(option);
			})();
		}

	}
	catch(error)
	{
		console.log("config_gui_t::update_options() - "+error);
	}
}

config_gui_t.prototype.set_robot_name=function(robot_name)
{
	try
	{
		if(!robot_name)
			throw "config_gui_t::set_robot_name - Invalid robot name."

		while(this.configs_list.firstChild)
			this.configs_list.removeChild(this.configs_list.firstChild);

		this.adder_select.options.length=1;
		this.editor.set_robot_name(robot_name);
	}
	catch(error)
	{
		console.log(error);
	}
}