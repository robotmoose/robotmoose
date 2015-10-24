//Members
//		onrefresh() - callback triggered when window needs updating (resizes)
//		onconfigure() - callback triggered when configure button is hit
//		onchange() - callback triggered when configuration is updated (download or UI)

function config_editor_t(div)
{
	if(!div)
		return null;

	var myself=this;
	this.div=div;
	this.element=document.createElement("div");
	this.counter=1;
	this.drag_list=new drag_list_t(this.element);

	if(!this.drag_list)
	{
		this.div=null;
		this.element=null;
	}

	this.entries=[];
	this.add_div=document.createElement("div");
	this.tabula={};
	this.tabula.select={};
	this.tabula.select.element=document.createElement("select");
	this.tabula.select.options=[];
	this.tabula.select.element.title="Pick a hardware device, then click 'Add'";
	this.tabula.options=[];
	this.add_button=document.createElement("input");
	this.configure_button=document.createElement("input");

	this.element.style.width=480;
	this.div.appendChild(this.element);

	this.add_div.className="form-inline";
	this.element.appendChild(this.add_div);

	this.tabula.select.element.className="form-control";
	this.tabula.select.element.disabled=true;
	this.tabula.select.element.style.width=256;
	this.tabula.select.element.style.marginRight=10;
	this.add_div.appendChild(this.tabula.select.element);

	this.add_button.className="btn btn-primary";
	this.add_button.disabled=true;
	this.add_button.type="button";
	this.add_button.value="Add";
	this.add_button.title="Click here to add this hardware device to this list";
	this.add_button.onclick=function(event)
	{
		var obj=myself.tabula.select.options[myself.tabula.select.element.selectedIndex].tabula;
		myself.create_entry(obj.type,obj.args);
	};
	this.add_div.appendChild(this.add_button);

	this.element.appendChild(document.createElement("br"));

	this.configure_button.className="btn btn-primary";
	this.configure_button.disabled=true;
	this.configure_button.type="button";
	this.configure_button.value="Configure";
	this.configure_button.title="Click here to send this list to the Arduino on the robot";
	this.configure_button.onclick=function(event)
	{
		if(myself.onconfigure)
			myself.onconfigure(myself);
		if (myself.onchange)
			myself.onchange(myself);
	};
	this.element.appendChild(this.configure_button);
}

config_editor_t.prototype.download=function(robot)
{
	if(!robot||!robot.name)
		return;

	var myself=this;

	superstar_get(robot.name,"options",function(options)
	{
		myself.get_options_m(options);
		myself.download_m(robot);
	});
}

config_editor_t.prototype.upload=function(robot)
{
	if(!robot||!robot.name)
		return;

	this.get_entries();

	var data={};
	data.counter=this.counter++;
	data.configs=[];

	this.get_entries();

	for(var key in this.entries)
	{
		if(this.entries[key])
		{
			var str=this.entries[key].type+"(";

			for(var ii=0;ii<this.entries[key].args.length;++ii)
			{
				if(this.entries[key].args[ii].type=="select-one")
					str+=this.entries[key].args[ii].options[this.entries[key].args[ii].selectedIndex].value;
				else if(this.entries[key].args[ii].type=="number")
					str+=this.entries[key].args[ii].value;
				else
					throw "Unsupported type!";

				if(ii+1!=this.entries[key].args.length)
					str+=",";
			}

			str+=");";
			data.configs.push(str);
		}
	}

	superstar_set(robot.name,"config",data,null,robot.auth);
}

config_editor_t.prototype.get_entries=function()
{
	this.entries.sort(function(lhs,rhs)
	{
		if(lhs&&rhs)
			return lhs.drag_list.li.offsetTop-rhs.drag_list.li.offsetTop;
	});

	return this.entries;
}

config_editor_t.prototype.create_entry=function(type,arg_types,arg_values)
{
	if(!type||!arg_types)
		return null;

	var myself=this;
	var entry={};
	entry.drag_list=this.drag_list.create_entry();
	entry.drag_list.config_editor_t=entry;
	entry.drag_list.onremove=function(entry){myself.remove_entry_m(entry.config_editor_t);};
	this.create_entry_m(entry,type,arg_types,arg_values);
	this.entries.push(entry);

	return entry;
}

config_editor_t.prototype.remove_entry=function(entry)
{
	if(!entry||!entry.drag_list)
		return;

	this.drag_list.remove_entry(entry.drag_list);
}










config_editor_t.prototype.download_m=function(robot)
{
	if(!robot||!robot.name)
		return;

	var myself=this;

	for(var key in this.entries)
		this.remove_entry(this.entries[key]);

	this.entries=[];

	superstar_get(robot.name,"config",function(obj)
	{
		if (obj==null) { // backend not connected, make fake config object
			obj={counter:0};
		}
		myself.counter=obj.counter+1;

		var config_text="";

		for(var key in obj.configs)
			config_text+=obj.configs[key]+"\n";

		var configs=myself.lex_m(config_text);

		for(var key in configs)
		{
			var lookup=myself.find_option_m(configs[key]);

			if(lookup)
				myself.create_entry(configs[key].type,lookup.args,configs[key].args);
			else
				console.log("Invalid tabula config: "+configs[key].type+"("+configs[key].args+");");
		}

		// Finally finished downloading--fire onchange
		if (myself.onchange)
			myself.onchange(myself);

		myself.refresh_m();
	});
}

config_editor_t.prototype.create_entry_m=function(entry,type,arg_types,arg_values)
{
	if(!entry||!entry.drag_list||!type||!arg_types)
		return;

	var myself=this;
	entry.type=type;
	entry.table={};
	entry.table.element=document.createElement("table");
	entry.table.row=entry.table.element.insertRow(0);
	entry.table.cells=[];
	entry.table.text=document.createTextNode(type);
	entry.args=[];

	entry.table.row.style.verticalAlign="center";
	entry.drag_list.content.appendChild(entry.table.element);

	entry.table.cells.push(entry.table.row.insertCell(0));
	entry.table.cells[0].style.paddingRight=10;
	entry.table.cells[0].appendChild(entry.table.text);

	for(var ii=0;ii<arg_types.length;++ii)
	{
		entry.table.cells.push(entry.table.row.insertCell(ii+1));

		var drop=null;
		var value=null;

		if(arg_values)
			value=arg_values[ii];

		if(arg_types[ii]=='C')
		{
			if(!is_count(value))
				value=null;

			drop=this.create_count_drop_m(value);
		}
		else if(arg_types[ii]=='P')
		{
			if(!is_pin(value))
				value=null;

			drop=this.create_pin_drop_m(value);
		}
		else if(arg_types[ii]=='S')
		{
			if(!is_serial(value))
				value=null;

			drop=this.create_serial_drop_m(value);
		}

		if(drop)
		{
			drop.onchange=function(){myself.refresh_m()};
			entry.args.push(drop);

			if(drop.div)
				entry.table.cells[ii+1].appendChild(drop.div);
			else
				entry.table.cells[ii+1].appendChild(drop);
		}
		else
		{
			console.log("config_editor_t::create_entry_m - Invalid tabula argument type.");
		}

		if(ii+1<arg_types.length)
			entry.table.cells[ii+1].style.paddingRight=10;
	}

	this.refresh_m();
}

config_editor_t.prototype.create_count_drop_m=function(value)
{
	var drop=document.createElement("input");
	drop.type="number";
	drop.className="form-control";
	drop.style.paddingRight=4;
	drop.setAttribute("min","1");
	drop.setAttribute("max","200");
	drop.setAttribute("step","1");
	drop.setAttribute("value","1");

	drop.div=document.createElement("div");
	drop.div.className="form-group has-feedback";
	drop.div.style.margin=drop.div.style.padding=0;
	drop.div.appendChild(drop);

	drop.onclick=drop.onkeyup=drop.onkeydown=function()
	{
		this.div.className="form-group has-feedback";

		if(!drop.valueAsNumber)
			this.div.className+=" has-error";
	};

	return drop;
}

config_editor_t.prototype.create_pin_drop_m=function(value)
{
	var drop=document.createElement("select");
	drop.title="Pick the Arduino pin where you plugged in the wires for this device";
	drop.style.width=120;
	drop.className="form-control"

	var title_option=document.createElement("option");
	title_option.text="Pin";
	title_option.selected=true;
	drop.add(title_option);

	for(var ii=3;ii<=21;++ii)
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

	drop.div=document.createElement("div");
	drop.div.className="form-group has-feedback";
	drop.div.style.margin=drop.div.style.padding=0;
	drop.div.appendChild(drop);

	drop.onclick=drop.onkeyup=drop.onkeydown=function()
	{
		this.div.className="form-group has-feedback";

		if(this.selectedIndex==0)
			this.div.className+=" has-error";
	};

	drop.click();

	return drop;
}

config_editor_t.prototype.create_serial_drop_m=function(value)
{
	var drop=document.createElement("select");
	drop.title="Pick the Arduino serial port plugged into this device (X3 means Arduino pins RX3/TX3)";
	drop.style.width=120;
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

	drop.div=document.createElement("div");
	drop.div.className="form-group has-feedback";
	drop.div.style.margin=drop.div.style.padding=0;
	drop.div.appendChild(drop);

	drop.onclick=drop.onkeyup=drop.onkeydown=function()
	{
		this.div.className="form-group has-feedback";

		if(this.selectedIndex==0)
			this.div.className+=" has-error";
	};

	drop.click();

	return drop;
}

config_editor_t.prototype.remove_entry_m=function(entry)
{
	if(!entry)
		return;

	for(var key in this.entries)
	{
		if(this.entries[key]&&this.entries[key]===entry)
		{
			this.entries[key]=null;
			break;
		}
	}

	this.refresh_m();
}

config_editor_t.prototype.lex_m=function(config)
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

config_editor_t.prototype.find_option_m=function(option)
{
	if(!option)
		return null;

	for(var key in this.tabula.options)
		if(this.tabula.options[key].type==option.type&&this.tabula.options[key].args.length==option.args.length)
			return this.tabula.options[key];

	return null;
}

config_editor_t.prototype.get_options_m=function(options)
{
	for(var key in this.tabula.select.options)
		if(this.tabula.select.options[key].element)
			this.tabula.select.element.removeChild(this.tabula.select.options[key].element);

	this.tabula.select.options=[];
	this.tabula.options=[];

	if(!options)
		return;

	options.sort();

	for(var ii=0;ii<options.length;++ii)
	{
		var parts=options[ii].split(" ");

		if(parts.length==1)
			parts.push("");

		if(parts.length!=2)
			continue;

		var obj={};
		obj.type=parts[0];
		obj.args=[];

		for(var jj=0;jj<parts[1].length;++jj)
			obj.args[jj]=parts[1][jj];

		this.tabula.options.push(obj);
	}

	for(var key in this.tabula.options)
	{
		var option={};
		option.tabula=this.tabula.options[key];
		option.element=document.createElement("option");
		option.element.innerHTML=option.tabula.type;
		option.element.value=option.tabula.type;

		this.tabula.select.element.appendChild(option.element);
		this.tabula.select.options.push(option);
	}

	this.refresh_m();
}

config_editor_t.prototype.refresh_m=function()
{
	this.tabula.select.element.disabled=(this.tabula.options.length==0);
	this.add_button.disabled=(this.tabula.options.length==0);

	var configureable=false;

	var entries_count=0;

	for(var key in this.entries)
		if(this.entries[key])
			++entries_count;

	if(entries_count>0)
	{
		configureable=true;

		for(var key in this.entries)
		{
			if(this.entries[key])
			{
				for(var drop in this.entries[key].args)
				{
					if((this.entries[key].args[drop].type=="select-one"&&this.entries[key].args[drop].selectedIndex==0)||
						(this.entries[key].args[drop].type=="number")&&!this.entries[key].args[drop].valueAsNumber)
					{
						configureable=false;
						break;
					}
				}
			}

			if(!configureable)
				break;
		}
	}

	this.configure_button.disabled=!configureable;

	if(this.onrefresh)
		this.onrefresh();
}
