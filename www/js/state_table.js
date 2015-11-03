/*
 Manage the UI of the "Code" tab of the main robot pilot interface

 Mike Moss, 2015-08 (Public Domain)
*/


//Members
//		onrefresh() - callback triggered when window needs updating (resizes)
//		onrun() - callback triggered when run button is hit
//		onstop() - callback triggered when stop button is hit

function state_table_t(doorway)
{
	if(!doorway||!doorway.content)
		return null;

	var myself=this;
	this.old_robot_name=null;
	this.old_robot_auth=null;
	this.doorway=doorway;
	this.overwrite_modal=new modal_yesno_t(this.doorway.parent_div,"Warning","");
	this.div=doorway.content;
	this.last_experiment="testing";

	this.make_error_span=function () {
		var errors=document.createElement("span");
		errors.style.color="#800000"; // dark red text
		errors.style.background="#ffa0a0";  // light red background
		return errors;
	}
	this.global_errors=this.make_error_span();
	// this.global_errors.textContent="Sample error here";
	this.div.appendChild(this.global_errors);
	this.last_error_entry=null;

	this.element=document.createElement("div");
	this.drag_list_div=document.createElement("div");
	this.drag_list=new drag_list_t(this.drag_list_div);
	this.controls_div=document.createElement("div");
	this.experiment=
	{
		div:document.createElement("div"),
		name:document.createElement("input"),
		glyph:document.createElement("span")
	};
	this.run_button=document.createElement("input");
	this.load_button=document.createElement("input");
	this.add_button=document.createElement("input");
	this.entries=[];

	if(!this.drag_list)
	{
		this.div=null;
		this.element=null;
	}

	this.element.style.width="auto";
	this.element.style.minWidth=640;
	this.div.appendChild(this.element);

	this.element.appendChild(this.drag_list_div);

	this.element.appendChild(this.controls_div);

	this.experiment.div.className="form-group";
	this.experiment.div.style.float="left";
	this.controls_div.appendChild(this.experiment.div);

	this.experiment.name.type="text";
	this.experiment.name.value=this.last_experiment;
	this.experiment.name.placeholder="Experiment Name";
	this.experiment.name.className="form-control";
	this.experiment.name.style.width=240;
	this.experiment.name.spellcheck=false;
	this.experiment.name.onchange=function(event){myself.update_experiment_m();};
	this.experiment.name.onkeydown=function(event){myself.update_experiment_m();};
	this.experiment.name.onkeyup=function(event){myself.update_experiment_m();};
	this.experiment.name.onkeypress=function(event){myself.update_experiment_m();};
	this.update_experiment_m();
	this.experiment.div.appendChild(this.experiment.name);

	this.experiment.glyph.className="glyphicon form-control-feedback glyphicon glyphicon-remove";
	this.experiment.div.appendChild(this.experiment.glyph);

	this.run_button.type="button";
	this.run_button.className="btn btn-primary";
	this.run_button.style.marginLeft=10;
	this.run_button.disabled=true;
	this.run_button.value="Run";
	this.run_button.title_run="Click here to make this code execute.";
	this.run_button.title=this.run_button.title_run;
	this.run_button.title_stop="Click to stop this code.";
	this.run_button.onclick=function(event){myself.run_button_pressed_m();};
	this.controls_div.appendChild(this.run_button);

	this.load_button.type="button";
	this.load_button.className="btn btn-primary";
	this.load_button.style.marginLeft=10;
	this.load_button.disabled=false;
	this.load_button.value="Load";
	this.load_button.title="Click here to open the code with the name in the box to the left.";
	this.load_button.onclick=function(event){myself.load_button_pressed_m();};
	this.controls_div.appendChild(this.load_button);

	this.add_button.type="button";
	this.add_button.className="btn btn-primary";
	this.add_button.style.float="right";
	this.add_button.disabled=false;
	this.add_button.value="Add State";
	this.add_button.title="Click here to add a new blank robot state to this list.";
	this.add_button.onclick=function(event){myself.add_button_pressed_m();};
	this.controls_div.appendChild(this.add_button);
}

state_table_t.prototype.download_with_check=function(robot,skip_get_active,ondownload)
{
	if(!robot||!robot.name)
		return;

	var myself=this;

	if(myself.last_experiment)
	{
		superstar_get(robot.name+"/experiments/"+myself.last_experiment+"/","code",
			function(obj)
			{
				if(JSON.stringify(obj)!=JSON.stringify(myself.get_states()))
				{
					myself.overwrite_modal.set_message("All unsaved changes to the experiment named \""+
						myself.last_experiment+"\" will be lost. Proceed?");
					myself.overwrite_modal.onok=
						function()
						{
							myself.download(robot,skip_get_active,ondownload);
						}
					myself.overwrite_modal.show();
				}
				else
				{
					myself.download(robot,skip_get_active,ondownload);
				}
			});
	}
	else
	{
		myself.download(robot,skip_get_active,ondownload);
	}
}

state_table_t.prototype.download=function(robot,skip_get_active,callback)
{
	this.old_robot_name=robot.name;
	this.old_robot_auth=robot.auth;

	this.clear();

	if(!robot||!robot.name)
		return;

	var myself=this;

	if(skip_get_active)
	{
		myself.download_m(robot,callback);
	}
	else
	{
		superstar_get(robot.name,"active_experiment",function(obj)
		{
			if(obj&&obj.length>0)
			{
				myself.last_experiment=myself.experiment.name.value=obj;
				myself.download_m(robot,callback);
			}
		});
	}

	this.update_states_m();
	this.update_buttons_m();
}

state_table_t.prototype.upload_with_check=function(robot,onupload)
{
	if(!robot||!robot.name)
		return;

	var myself=this;

	superstar_get(robot.name+"/experiments/"+myself.experiment.name.value+"/","code",
		function(obj)
		{
			if(myself.last_experiment&&obj&&obj.length>0&&myself.last_experiment!=myself.experiment.name.value)
			{
				myself.overwrite_modal.set_message("This will overwrite the non-empty experiment named \""+
					myself.experiment.name.value+"\". Proceed?");
				myself.overwrite_modal.onok=function(){myself.upload(robot,onupload);}
				myself.overwrite_modal.show();
			}
			else
			{
				myself.upload(robot,onupload);
			}
		});
}

state_table_t.prototype.upload=function(robot,onupload)
{
	if(!robot||!robot.name)
		return;

	var myself=this;

	superstar_set(robot.name+"/experiments/"+this.experiment.name.value+"/","code",this.get_states(),
		function(obj)
		{
			superstar_set(robot.name,"active_experiment",myself.experiment.name.value,
				function(obj)
				{
					myself.last_experiment=myself.experiment.name.value;

					if(onupload)
						onupload();
				},
				robot.auth);
		},
		robot.auth);
}

state_table_t.prototype.get_states=function()
{
	var data=[];

	var entries=this.get_entries();

	for(var key in entries)
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


// Debug prints
state_table_t.prototype.clear_prints=function()
{
	var entries=this.get_entries();

	for(var key in entries)
		if(entries[key])
			entries[key].prints.textContent="";
}

state_table_t.prototype.show_prints=function(print_text,current_state)
{
	var print_entry=this.find_entry(current_state);
	if (!print_entry) return;

	print_entry.prints.textContent=print_text;
}


// Error reporting onscreen
state_table_t.prototype.clear_error=function()
{
	this.show_error(null,null); // hacky way to clear errors
}

state_table_t.prototype.show_error=function(error_text,current_state)
{
	var error_entry=null;
	if (current_state) error_entry=this.find_entry(current_state);

	var global_report="", local_report="";
	if (error_text) {
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

state_table_t.prototype.get_entries=function()
{
	this.entries.sort(function(lhs,rhs)
	{
		if(lhs&&rhs)
			return lhs.drag_list.li.offsetTop-rhs.drag_list.li.offsetTop;
	});

	var entries=[];

	for(var key in this.entries)
		if(this.entries[key])
			entries.push(this.entries[key]);

	this.entries=entries;

	return this.entries;
}

state_table_t.prototype.create_entry=function(state,time,code)
{
	if(!state)
		state="";

	if(!code)
		code="";

	if(!time)
		time="";

	var old_size=
	{
		body:
		{
			offsetWidth:parseInt(this.doorway.body.offsetWidth,10),
			offsetHeight:parseInt(this.doorway.body.offsetHeight,10),
			scrollHeight:parseInt(this.doorway.body.scrollHeight,10)
		},
		content:
		{
			scrollHeight:parseInt(this.doorway.content.scrollHeight,10)
		}
	};

	var myself=this;
	var entry={};
	entry.drag_list=this.drag_list.create_entry();
	entry.drag_list.li.style.minWidth=740;
	entry.drag_list.ondrag=function(){myself.onstop_m();};
	entry.drag_list.state_table_t=entry;
	entry.drag_list.onremove=function(entry){myself.remove_entry_m(entry.state_table_t);};
	this.create_entry_m(entry,state,time,code);
	this.entries.push(entry);
	this.update_states_m();

	if(this.run_button.value!="Run")
		this.onstop_m();

	var new_size=
	{
		body:
		{
			offsetWidth:parseInt(this.doorway.body.offsetWidth,10),
			offsetHeight:parseInt(this.doorway.body.offsetHeight,10),
			scrollHeight:parseInt(this.doorway.body.scrollHeight,10)
		},
		content:
		{
			scrollHeight:parseInt(this.doorway.content.scrollHeight,10)
		}
	};

	var delta_size=
	{
		body:
		{
			offsetWidth:new_size.body.offsetWidth-old_size.body.offsetWidth,
			offsetHeight:new_size.body.offsetHeight-old_size.body.offsetHeight,
			scrollWidth:new_size.body.scrollWidth-old_size.body.scrollWidth,
			scrollHeight:new_size.body.scrollHeight-old_size.body.scrollHeight
		},
		content:
		{
			scrollHeight:new_size.content.scrollHeight-old_size.content.scrollHeight
		}
	};

	var size=
	{
		width:new_size.body.offsetWidth,
		height:new_size.body.offsetHeight+delta_size.body.scrollHeight
	};

	if(old_size.content.scrollHeight!=new_size.content.scrollHeight)
		this.doorway.resize(size);

	this.doorway.body.scrollTop=this.doorway.body.scrollHeight;

	return entry;
}

state_table_t.prototype.find_entry=function(state_name)
{
	var entries=this.get_entries();

	for(var key in entries)
	{
		if(entries[key])
		{
			if(entries[key].input.text.value==state_name)
				return entries[key];
		}
	}

	return null;
}

state_table_t.prototype.remove_entry=function(entry)
{
	if(!entry||!entry.drag_list)
		return;

	this.drag_list.remove_entry(entry.drag_list);
}

state_table_t.prototype.set_active=function(state)
{
	var entries=this.get_entries();

	for(var key in entries)
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

state_table_t.prototype.run=function()
{
	this.run_button.value="Stop";
	this.run_button.title=this.run_button.title_stop;
}

state_table_t.prototype.clear=function()
{
	this.drag_list.clear();
}










state_table_t.prototype.download_m=function(robot,callback)
{
	var myself=this;

	superstar_get(robot.name+"/experiments/"+this.experiment.name.value+"/","code",function(obj)
	{
		myself.last_active=myself.experiment.name.value;

		for(var key in obj)
			myself.create_entry(obj[key].name,obj[key].time,obj[key].code);

		if(callback)
			callback();
	});
}

state_table_t.prototype.run_button_pressed_m=function()
{
	if(this.run_button.value=="Run")
		this.onrun_m();
	else
		this.onstop_m();
}

state_table_t.prototype.add_button_pressed_m=function()
{
	this.onstop_m();

	var state_name="";

	if(this.get_states().length==0)
		state_name="start";

	this.create_entry(state_name,"","// JavaScript code\n");
}

state_table_t.prototype.load_button_pressed_m=function()
{
	var myself=this;

	this.onstop_m();

	var old_robot={name:myself.old_robot_name,auth:myself.old_robot_auth}

	this.download_with_check(old_robot,true,
		function()
		{
			myself.upload(old_robot);
		});
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

state_table_t.prototype.refresh_m=function()
{
	if(this.onrefresh)
		this.onrefresh();
}

state_table_t.prototype.create_entry_m=function(entry,state,time,code)
{
	if(!entry||!entry.drag_list)
		return;

	if(!state)
		state="";

	if(!time)
		time="";

	if(!code)
		code="";

	var myself=this;
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

	entry.table.row.style.verticalAlign="middle";
	entry.table.left.style.paddingRight=10;
	entry.drag_list.content.appendChild(entry.table.element);

	entry.input.div.className="form-group has-feedback has-error";
	entry.table.left.appendChild(entry.input.div);

	entry.input.text.type="text";
	entry.input.text.placeholder="Name";
	entry.input.text.className="form-control";
	entry.input.text.style.width="128px";
	entry.input.text.spellcheck=false;
	entry.input.text.size=10;
	entry.input.text.value=state;
	entry.input.text.entry_input=entry.input;
	entry.input.text.onchange=function(event){myself.update_states_m();};
	entry.input.text.onkeydown=function(event){myself.update_states_m();};
	entry.input.text.onkeyup=function(event){myself.update_states_m();};
	entry.input.text.onkeypress=function(event){myself.update_states_m();};
	this.update_states_m();
	entry.input.div.appendChild(entry.input.text);

	entry.input.glyph.className="glyphicon form-control-feedback glyphicon glyphicon-remove";
	entry.input.div.appendChild(entry.input.glyph);

	entry.table.left.appendChild(document.createElement("br"));

	entry.time.type="text";
	entry.time.placeholder="Run Time (ms)";
	entry.time.className="form-control";
	entry.time.style.width="128px";
	entry.time.spellcheck=false;
	entry.time.size=10;
	entry.time.value=time;
	entry.time.onchange=function(event){myself.validate_time_m(this);};
	entry.time.onkeydown=function(event){myself.validate_time_m(this);};
	entry.time.onkeyup=function(event){myself.validate_time_m(this);};
	entry.time.onkeypress=function(event){myself.validate_time_m(this);};
	this.validate_time_m(entry.time);
	entry.table.left.appendChild(entry.time);

	entry.errors=this.make_error_span();
	entry.table.code.appendChild(entry.errors);

	entry.textarea=document.createElement("textarea");
	entry.textarea.innerHTML=code;
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
	entry.code_editor.on("change",function(){myself.code_change_m()});
	entry.code_editor.setSize(500);
	entry.code_editor_event=function(event){entry.code_editor.refresh();};
	window.addEventListener("click",entry.code_editor_event);

	this.refresh_m();
}

state_table_t.prototype.remove_entry_m=function(entry)
{
	if(!entry)
		return;

	for(var key in this.entries)
	{
		if(this.entries[key]&&this.entries[key]===entry)
		{
			window.removeEventListener("click",this.entries[key].code_editor_event);
			this.entries[key]=undefined;
			break;
		}
	}

	var new_entries=[];

	for(var key in this.entries)
		if(this.entries[key])
			new_entries.push(this.entries[key]);

	this.entries=new_entries;

	this.refresh_m();
	this.update_states_m();

	if(this.run_button.value!="Run")
		this.onstop_m();
}

state_table_t.prototype.update_states_m=function()
{
	for(var key in this.entries)
		if(this.entries[key])
			this.set_state_name_valid_m(this.entries[key].input,this.validate_state_m(this.entries[key].input));

	this.update_buttons_m();
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

	for(var key in entries)
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

state_table_t.prototype.validate_time_m=function(input)
{
	while(input.value.length>0&&!is_time(input.value))
		input.value=input.value.substr(0,input.value.length-1);

	if(this.run_button.value!="Run")
		this.onstop_m();
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

state_table_t.prototype.update_buttons_m=function(valid)
{
	var entries=this.get_entries();
	var valid=true;

	for(var key in entries)
	{
		if(entries[key])
		{
			if(!this.validate_state_m(entries[key].input))
				valid=false;
		}
	}

	if(this.experiment.name.value.length==0)
		valid=false;

	this.run_button.disabled=!valid;
	this.load_button.disabled=(this.experiment.name.value.length==0);

	if(this.run_button.value!="Run")
		this.onstop_m();
}

state_table_t.prototype.update_experiment_m=function()
{
	if(this.experiment.name.value.length == 0)
	{
		this.experiment.div.className = "form-group has-feedback has-error";
		this.experiment.glyph.style.visibility="visible";
	}
	else
	{
		this.experiment.div.className = "form-group";
		this.experiment.glyph.style.visibility="hidden";
	}

	this.update_buttons_m();
}

state_table_t.prototype.code_change_m=function()
{
	if(this.run_button.value!="Run")
		this.onstop_m();
}
