/*
 Manage the UI of the "Code" tab of the main robot pilot interface

 Mike Moss, 2015-08 (Public Domain)
*/

//Members
//		onrefresh() - callback triggered when window needs updating (resizes)
//		onrun() - callback triggered when run button is hit
//		onstop() - callback triggered when stop button is hit

function state_table_t(doorway,modal_el)
{
	if(!doorway||!doorway.content)
		return null;

	var _this=this;
	this.doorway=doorway;
	this.div=doorway.content;

	this.robot=null;
	this.entries=[];
	this.active_experiment=null;
	this.next_active_experiment=null;

	this.el=document.createElement("div");
	this.div.appendChild(this.el);

	this.drop=new dropdown_t(this.el,null,function(value)
	{
		_this.rebuild_dropdown_m(value);
	});
	this.drop.set_width(200);

	this.createnew_modal=new modal_createnew_t
	(
		modal_el,
		"New Experiment",
		"",
		function(value)
		{
			return _this.experiment_exists(value,function(ok)
			{
				if(ok)
					_this.createnew_modal.hide_error();
				else
					_this.createnew_modal.show_error();
			});
		},
		function(value)
		{
			_this.create_new_experiment(value);
		}
	);

	this.new_button=document.createElement("input");
	this.el.appendChild(this.new_button);
	this.new_button.type="button";
	this.new_button.className="btn btn-primary";
	this.new_button.style.width="64px";
	this.new_button.style.marginLeft=10;
	this.new_button.disabled=false;
	this.new_button.value="New";
	this.new_button.title="Click here to create a new experiment.";
	this.new_button.onclick=function()
	{
		_this.createnew_modal.show();
	};

	this.run_button=document.createElement("input");
	this.el.appendChild(this.run_button);
	this.run_button.type="button";
	this.run_button.className="btn btn-primary";
	this.run_button.style.width="64px";
	this.run_button.style.marginLeft=10;
	this.run_button.disabled=false;
	this.run_button.value="Run";
	this.run_button.title_run="Click here to save this code and make it execute.";
	this.run_button.title=this.run_button.title_run;
	this.run_button.title_stop="Click to stop this code.";
	this.run_button.onclick=function()
	{
		_this.run_button_pressed_m();
	};

	this.load_file_modal=new modal_loadstate_t(modal_el, this);

	this.load_file_button=document.createElement("input");
	this.el.appendChild(this.load_file_button);
	this.load_file_button.type="button";
	this.load_file_button.className="btn btn-primary";
	this.load_file_button.style.float="right";
	this.load_file_button.style.marginLeft=10;
	this.load_file_button.disabled=false;
	this.load_file_button.value="Load from File";
	this.load_file_button.title="Click here to load a state from a local file.";
	this.load_file_button.onclick=function(event){_this.load_file_button_pressed_m();};

	this.save_file_button=document.createElement("input");
	this.el.appendChild(this.save_file_button);
	this.save_file_button.type="button";
	this.save_file_button.className="btn btn-primary";
	this.save_file_button.style.float="right";
	this.save_file_button.style.marginLeft=10;
	this.save_file_button.disabled=false;
	this.save_file_button.value="Save to File";
	this.save_file_button.title="Click here to save a state to a local file.";
	this.save_file_button.onclick=function(event){_this.save_file_button_pressed_m();};


	this.add_button=document.createElement("input");
	this.el.appendChild(this.add_button);
	this.add_button.type="button";
	this.add_button.className="btn btn-primary";
	this.add_button.style.width="96px";
	this.add_button.style.float="right";
	this.add_button.disabled=false;
	this.add_button.value="Add State";
	this.add_button.title="Click here to add a new blank robot state to this list.";
	this.add_button.onclick=function(event){_this.add_button_pressed_m();};

	this.el.appendChild(document.createElement("br"));
	this.el.appendChild(document.createElement("br"));

	this.global_errors=this.make_error_span_m();
	this.div.appendChild(this.global_errors);
	this.last_error_entry=null;

	this.drag_list=new drag_list_t(this.el);
}

state_table_t.prototype.destroy=function()
{
	try
	{
		this.div.removeChild(this.el);
		if(this.update_interval)
		{
			clearInterval(this.update_interval);
			this.update_interval=null;
		}
	}
	catch(error)
	{}
}

state_table_t.prototype.get_active_experiment=function(onfinish)
{
	var _this = this;
	if (this.robot.sim) onfinish(_this.robot.active_experiment);
	onfinish(robot_network.active_experiment);
	/*if(valid_robot(this.robot))
		superstar_get(this.robot,"active_experiment",function(active)
		{
			if(onfinish)
				onfinish(active);
		});*/
}

state_table_t.prototype.get_experiments=function(active,onfinish)
{

	var _this=this;
	if(valid_robot(this.robot))
	{
		/*superstar_sub(this.robot,"experiments",function(json)
		{*/	if (this.robot.sim)
				var json = this.robot.experiments_list;
			else{
				var json=robot_network.experiments;
			}
			//console.log("getting experiments: " + JSON.stringify(json))
			_this.drop.build(json,active||_this.next_active_experiment);
			_this.next_active_experiment=null;
			if(onfinish)
				onfinish();
		//});
	}
}

state_table_t.prototype.download=function(robot,onfinish)
{
	var _this=this;
	if(valid_robot(robot))
		if(!this.robot)
		{
			this.robot=robot;
			this.get_active_experiment(function(active)
			{
				_this.get_experiments(active,function(active)
				{
					if(onfinish)
						onfinish();
					_this.update_interval=setInterval(function()
					{
						_this.get_experiments();
					},250);
				});
			});
		}
		else
		{
			this.robot=robot;
			if(onfinish)
				onfinish();
		}
}

state_table_t.prototype.upload=function(robot,onfinish)
{
	var _this=this;
	if(valid_robot(robot))
	{
		if(robot.sim) sim_set_experiment(robot, _this.active_experiment, _this.get_states());
		else
		superstar_set(robot,"experiments/"+this.active_experiment+"/code",this.get_states(),function()
		{
			if(onfinish)
				onfinish();
		});
	}
}

state_table_t.prototype.get_experiment=function(experiment,onfinish)
{
	if(valid_robot(this.robot))
		if(this.robot.sim)
			sim_get_experiment(this.robot,experiment,function(json)
			{
				if(onfinish)
					onfinish(json);
			});
		else
			superstar_get(this.robot,"experiments/"+experiment+"/code",function(json)
			{
				if(onfinish)
					onfinish(json);
			});
}

state_table_t.prototype.experiment_exists=function(name,onfinish)
{
	if(valid_robot(this.robot))
	{
		var _this=this;
		//superstar_sub(this.robot,"experiments",function(experiments)
		//{
			if (this.robot.sim)
				var experiments=this.robot.experiments;
			else
				var experiments=robot_network.experiments;
			var found=false;
			for(k in experiments)
				if(decodeURIComponent(experiments[k])==name)
				{
					found=true;
					break;
				}
			if(onfinish)
				onfinish(!found);
		//});
	}
}

//Takes a json of states and builds the states list:
//	json should look like: [{name:"state_name",code:"//The Code",time:123},...]
state_table_t.prototype.build=function(json)
{
	//if (json)console.log("Experiments: " + json.toString());
	this.drag_list.clear(false);
	this.entries=[];
	for(let key in json)
		this.create_entry_m(json[key].name,json[key].time,json[key].code);
	for(let key in this.entries)
		this.validate_state_m(this.entries[key].input);
}

state_table_t.prototype.get_entries=function()
{
	this.entries.sort(function(lhs,rhs)
	{
		if(lhs&&rhs)
			return lhs.drag_list.li.offsetTop-rhs.drag_list.li.offsetTop;
	});

	var entries=[];

	for(let key in this.entries)
		if(this.entries[key])
			entries.push(this.entries[key]);

	this.entries=entries;

	return this.entries;
}

state_table_t.prototype.set_active=function(state)
{
	var entries=this.get_entries();

	for(let key in entries)
	{
		var e=entries[key];
		if(e)
		{
			var color="";
			if(e.input.text.value==state) { // currently running state
				color="#337ab7";
			}
			if (e.errors.textContent!="") { // reporting an error
				color="#ffd0d0";
			}

			e.drag_list.li.style.backgroundColor=color;
		}
	}
}

//Returns a json array of the current state table:
//	json looks like: [{name:"state_name",code:"//The Code",time:123},...]
state_table_t.prototype.get_states=function()
{
	var data=[];

	var entries=this.get_entries();

	for(let key in entries)
	{
		if(entries[key])
		{
			var obj={};
			obj.name=entries[key].input.text.value;
			obj.time=entries[key].time.value;
			obj.code=entries[key].code_editor.getValue();
			data.push(obj);
		}
	}

	return data;
}

state_table_t.prototype.upload_active_experiment=function(experiment)
{
	if(this.robot.sim)
		sim_set(this.robot,"active_experiment",experiment);
	else if(valid_robot(this.robot))
		superstar_set(this.robot,"active_experiment",experiment);
}

state_table_t.prototype.rebuild_dropdown_m=function(value)
{
	var _this=this;
	if(valid_robot(this.robot))
		this.download(this.robot,function()
		{
			_this.get_experiment(value,function(json)
			{
				_this.build(json);
				_this.active_experiment=value;
				_this.upload_active_experiment(value);
				_this.update_states_m();
			});
		});
}
























state_table_t.prototype.create_entry_m=function(name,time,code)
{
	var _this=this;
	if(!name)
		name="";
	if(!time)
		time="";
	if(!code)
		code="";
	var entry={};
	entry.drag_list=this.drag_list.create_entry();
	entry.drag_list.li.style.minWidth=740;
	entry.drag_list.state_table_t=entry;
	entry.drag_list.ondrag=function()
	{
		setTimeout(function()
		{
			_this.onstop_m();
			_this.update_states_m();
			_this.upload(_this.robot);
		},100);
	};
	entry.drag_list.onremove=function(entry)
	{
		_this.onstop_m();
		_this.remove_entry_m(entry.state_table_t);
		_this.update_states_m();
		_this.upload(_this.robot);
	};

	entry.table={};
	entry.table.element=document.createElement("table");
	entry.table.row=entry.table.element.insertRow(0);
	entry.table.left=entry.table.row.insertCell(0);
	entry.table.code=entry.table.row.insertCell(1);
	entry.table.right=entry.table.row.insertCell(2);
	entry.input={};
	entry.input.div=document.createElement("div");
	entry.input.glyph=document.createElement("span");
	entry.input.text=document.createElement("input");
	entry.time=document.createElement("input");
	entry.download_button=document.createElement("span");

	entry.table.row.style.verticalAlign="middle";
	entry.table.left.style.paddingRight=10;
	entry.drag_list.content.appendChild(entry.table.element);

	entry.input.div.className="form-group has-feedback";
	entry.table.left.appendChild(entry.input.div);

	entry.input.text.type="text";
	entry.input.text.placeholder="Name";
	entry.input.text.className="form-control";
	entry.input.text.style.width="128px";
	entry.input.text.spellcheck=false;
	entry.input.text.size=10;
	entry.input.text.value=name;
	entry.input.text.entry_input=entry.input;
	var onchange=function(event)
	{
		_this.onstop_m();
		_this.update_states_m();
	};
	var onchange_and_save=function(event)
	{
		_this.upload(_this.robot,onchange);
	}
	entry.input.text.onchange=onchange_and_save;
	entry.input.text.onkeydown=onchange;
	entry.input.text.onkeyup=onchange;
	entry.input.text.onkeypress=onchange;
	entry.input.div.appendChild(entry.input.text);

	entry.input.glyph.className="glyphicon form-control-feedback glyphicon glyphicon-remove";
	entry.input.glyph.style.visibility="hidden";
	entry.input.div.appendChild(entry.input.glyph);

	entry.time.type="text";
	entry.time.placeholder="Run Time (ms)";
	entry.time.className="form-control";
	entry.time.style.width="128px";
	entry.time.spellcheck=false;
	entry.time.size=10;
	entry.time.value=time;
	var validate=function(event)
	{
		_this.onstop_m();
		_this.validate_time_m(this);
		_this.update_states_m();
	};
	entry.time.onchange=validate;
	entry.time.onkeydown=validate;
	entry.time.onkeyup=validate;
	entry.time.onkeypress=validate;
	entry.table.left.appendChild(entry.time);
	this.validate_time_m(entry.time);

	entry.errors=this.make_error_span_m();
	entry.table.code.appendChild(entry.errors);

	entry.textarea=document.createElement("textarea");
	entry.textarea.value=code;
	entry.table.code.appendChild(entry.textarea);

	entry.prints=document.createElement("span");
	entry.table.right.appendChild(entry.prints);

	entry.code_editor=CodeMirror.fromTextArea(entry.textarea,
	{
		indentUnit:4,
		indentWithTabs:true,
		lineNumbers:true,
		matchBrackets:true,
		mode:"text/x-javascript"
	});
	entry.code_editor.on("change",onchange_and_save);
	entry.code_editor.setSize(500);
	entry.code_editor_event=function(event){entry.code_editor.refresh();};
	window.addEventListener("click",entry.code_editor_event);

	this.entries.push(entry);
}

state_table_t.prototype.remove_entry_m=function(entry)
{
	if(!entry||!entry.drag_list)
		return;

	for(let key in this.entries)
	{
		if(this.entries[key]&&this.entries[key]===entry)
		{
			window.removeEventListener("click",this.entries[key].code_editor_event);
			this.entries[key]=undefined;
			break;
		}
	}

	var new_entries=[];

	for(let key in this.entries)
		if(this.entries[key])
			new_entries.push(this.entries[key]);

	this.entries=new_entries;
}

state_table_t.prototype.make_error_span_m=function(text)
{
	var errors=document.createElement("span");
	if(text)
		errors.innerHTML=text;
	errors.style.color="#800000"; // dark red text
	errors.style.background="#ffa0a0";  // light red background
	return errors;
}

state_table_t.prototype.validate_time_m=function(input)
{
	while(input.value.length>0&&!is_time(input.value))
		input.value=input.value.substr(0,input.value.length-1);

	if(this.run_button.value!="Run")
		this.onstop_m();
}

state_table_t.prototype.validate_state_m=function(input)
{
	if(!input)
		return true;
	while(input.text.value.length>0&&!is_ident(input.text.value))
		input.text.value=input.text.value.substr(0,input.text.value.length-1);
	var valid=input.text.value.length>0&&input.text.value!="state";
	var entries=this.get_entries();
	var counts={};
	for(let key in entries)
	{
		if(entries[key])
		{
			if(!counts[entries[key].input.text.value])
				counts[entries[key].input.text.value]=1;
			else
				counts[entries[key].input.text.value]+=1;
		}
	}
	if(counts[input.text.value]>1)
		valid=false;
	return valid;
}

state_table_t.prototype.update_states_m=function()
{
	for(let key in this.entries)
		if(this.entries[key])
			this.set_state_name_valid_m(this.entries[key].input,
				this.validate_state_m(this.entries[key].input));
}

state_table_t.prototype.set_state_name_valid_m=function(input,valid)
{
	if(valid)
	{
		input.div.className="form-group";
		input.glyph.style.visibility="hidden";
	}
	else
	{
		input.div.className="form-group has-feedback has-error";
		input.glyph.style.visibility="visible";
	}
}

state_table_t.prototype.create_new_experiment=function(value)
{
	var _this=this;
	if(valid_robot(this.robot))
		{
			if (this.robot.sim)
				sim_set_experiment(
					_this.robot,
					value,
					[{name:"HelloWorld",time:0,code:""}],
					function()
					{
						_this.next_active_experiment=value;
					}
				);
			else
			superstar_set
			(
				this.robot,
				"experiments/"+value+"/code",
				[{name:"HelloWorld",time:0,code:""}],
				function()
				{
					_this.next_active_experiment=value;
				}
			);

		}
}

state_table_t.prototype.load_file_button_pressed_m=function()
{
	this.onstop_m();
	this.load_file_modal.modal.show();
}

state_table_t.prototype.save_file_button_pressed_m=function()
{
	this.onstop_m();
	var fake_link=document.createElement("a");
	fake_link.download=this.active_experiment+".json";
	fake_link.href=URL.createObjectURL(new Blob([JSON.stringify(this.get_states())]));
	document.body.appendChild(fake_link);
	fake_link.click();
	document.body.removeChild(fake_link);
}








state_table_t.prototype.onrun_m=function()
{
	if(this.onrun)
		this.onrun(this);
}

state_table_t.prototype.onstop_m=function()
{
	if(this.onstop)
		this.onstop(this);

	this.run_button.value="Run";
	this.run_button.title=this.run_button.title_run;
}

state_table_t.prototype.run=function()
{
	this.run_button.value="Stop";
	this.run_button.title=this.run_button.title_stop;
};

state_table_t.prototype.run_button_pressed_m=function()
{
	if(this.run_button.value=="Run")
		this.onrun_m();
	else
		this.onstop_m();
}

state_table_t.prototype.clear_prints=function()
{
	var entries=this.get_entries();

	for(let key in entries)
		if(entries[key])
			entries[key].prints.textContent="";
}

state_table_t.prototype.clear_error=function()
{
	this.show_error("",""); // hacky way to clear errors
}

state_table_t.prototype.show_error=function(error_text,current_state)
{
	var error_entry=null;
	if (current_state) error_entry=this.find_entry(current_state);

	var global_report="", local_report="";
	if (error_text)
	{
		local_report="Error here: "+error_text;
		if (error_entry) { // detailed error next to state
			global_report="Error in state '"+current_state+"'";
		} else {
			global_report="Error in state '"+current_state+"': "+error_text;
		}
	}

	this.global_errors.textContent=global_report;

	if (this.last_error_entry) {
		this.last_error_entry.errors.textContent=""; // clear last error
		this.last_error_entry.drag_list.li.style.backgroundColor=""; // clear background
	}

	if (!error_entry) return; // a bad state name, or what?

	error_entry.errors.textContent=local_report;
	error_entry.drag_list.li.style.backgroundColor="#ffe000"; // light red background

	this.last_error_entry=error_entry;
}

state_table_t.prototype.find_entry=function(state_name)
{
	var entries=this.get_entries();

	for(let key in entries)
	{
		if(entries[key])
		{
			if(entries[key].input.text.value==state_name)
				return entries[key];
		}
	}

	return null;
}

state_table_t.prototype.show_prints=function(print_text,current_state)
{
	var print_entry=this.find_entry(current_state);
	if (!print_entry) return;

	print_entry.prints.textContent=print_text;
}

state_table_t.prototype.add_button_pressed_m=function()
{
	this.onstop_m();
	var state_name="";
	if(this.get_states().length==0)
		state_name="start";
	this.create_entry_m(state_name,"","// JavaScript code\n",true);
	var _this=this;
	this.upload(this.robot,function()
	{
		_this.update_states_m();
	});
}
