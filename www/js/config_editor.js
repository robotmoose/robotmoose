//Members
//		onrefresh() - callback triggered when window changes
//		onconfigure() - callback triggered when configure button is hit

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
	this.configure_button.onclick=function(event)
	{
		if(myself.onconfigure)
			myself.onconfigure(myself);
	};
	this.element.appendChild(this.configure_button);
}

config_editor_t.prototype.get_options=function(robot_name)
{
	if(!robot_name)
		return;

	var myself=this;

	try
	{
		send_request("GET","/superstar/"+robot_name,"options","?get",
			function(response)
			{
				if(response)
				{
					var options=JSON.parse(response);
					myself.get_options_m(options);
				}
			},
			function(error)
			{
				throw error;
			},
			"application/json");
		}
		catch(error)
		{
			console.log("config_editor_t::get_options() - "+error);
		}
}

config_editor_t.prototype.download=function(robot_name)
{
	if(!robot_name)
		return;

	var myself=this;

	try
	{
		send_request("GET","/superstar/"+robot_name,"config","?get",
			function(response)
			{
				if(response)
				{
					for(var key in myself.entries)
						myself.remove_entry(myself.entries[key]);

					var obj=JSON.parse(response);

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
				}
			},
			function(error)
			{
				throw error;
			},
			"application/json");
	}
	catch(error)
	{
		console.log("config_editor_t::download() - "+error);
	}
}

config_editor_t.prototype.upload=function(robot_name)
{
	if(!robot_name)
		return;

	this.get_entries();

	try
	{
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
					str+=this.entries[key].args[ii].options[this.entries[key].args[ii].selectedIndex].value;

					if(ii+1!=this.entries[key].args.length)
						str+=",";
				}

				str+=");";
				data.configs.push(str);
			}
		}

		send_request("GET","/superstar/"+robot_name,"config",
			"?set="+encodeURIComponent(JSON.stringify(data)),
			function(response)
			{
			},
			function(error)
			{
				throw error;
			},
			"application/json");
	}
	catch(error)
	{
		console.log("config_editor_t::upload() - "+error);
	}
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
	entry.drag_list.onremove=function(entry){myself.remove_entry(entry.config_editor_t);};
	this.create_entry_m(entry,type,arg_types,arg_values);
	this.entries.push(entry);

	return entry;
}

config_editor_t.prototype.remove_entry=function(entry)
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

		if(arg_types[ii]=='P')
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

config_editor_t.prototype.create_pin_drop_m=function(value)
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

	return drop;
}

config_editor_t.prototype.create_serial_drop_m=function(value)
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

	return drop;
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
		this.tabula.select.element.removeChild(this.tabula.select.options[key]);

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
					if(this.entries[key].args[drop].selectedIndex==0)
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