//Members
//		clone_target - name of robot to clone (set before calling show)
//		onclone(name,settings) - triggered when clone button is hit

function modal_clone_t(div)
{
	this.modal=new modal_t(div);
	this.clone_target="";
	this.clone_info={};
	this.clone_info.has_typed=false;
	this.clone_info.div=document.createElement("div");
	this.clone_info.input=document.createElement("input");
	this.clone_info.glyph=document.createElement("span");
	this.clone_info.label=document.createElement("label");
	this.clone_info.timeout=null;
	this.settings={};
	this.settings.text=document.createTextNode("Settings to clone:");
	this.settings.div=document.createElement("div");
	this.settings.checkboxes=[];
	this.settings.buttons={};
	this.settings.buttons.select_all=document.createElement("input");
	this.settings.buttons.select_none=document.createElement("input");
	this.clone_button=document.createElement("input");
	this.cancel_button=document.createElement("input");

	if(!this.modal)
	{
		this.modal=null;
		return null;
	}

	var myself=this;

	this.clone_info.div.className="has-feedback";
	this.modal.get_content().appendChild(this.clone_info.div);

	this.clone_info.input.type="text";
	this.clone_info.input.className="form-control";
	this.clone_info.input.spellcheck=false;
	this.clone_info.input.placeholder="Enter robot clone's name here..."
	this.clone_info.input.onkeydown=this.clone_info.input.onkeyup=function()
	{
		myself.set_input_normal_m();

		if(myself.clone_info.timeout)
			clearTimeout(myself.clone_info.timeout);

		if(myself.valid_robot_name_m(this.value))
		{
			var input=this;
			myself.clone_info.timeout=setTimeout(function(){myself.check_exists_m(input.value);},300);
			myself.clone_button.disabled=false;
			myself.set_input_success_m();
		}
		else if(myself.clone_info.has_typed||this.value.length!=0)
		{
			myself.clone_button.disabled=true;
			myself.set_input_error_m();
		}

		myself.clone_info.has_typed=true;
	};
	this.clone_info.div.appendChild(this.clone_info.input);

	this.clone_info.div.appendChild(this.clone_info.glyph);

	this.clone_info.label.className="help-inline";
	this.clone_info.div.appendChild(this.clone_info.label);

	this.modal.get_content().appendChild(this.settings.text);

	this.settings.div.style.marginLeft=10;
	this.modal.get_content().appendChild(this.settings.div);

	this.modal.get_content().appendChild(document.createElement("br"));

	this.settings.buttons.select_all.type="button";
	this.settings.buttons.select_all.className="btn btn-primary";
	this.settings.buttons.select_all.style.marginRight=10;
	this.settings.buttons.select_all.value="Select All";
	this.settings.buttons.select_all.onclick=function(){myself.select_all_m();};
	this.modal.get_content().appendChild(this.settings.buttons.select_all);

	this.settings.buttons.select_none.type="button";
	this.settings.buttons.select_none.className="btn btn-primary";
	this.settings.buttons.select_none.value="Select None";
	this.settings.buttons.select_none.onclick=function(){myself.select_none_m();};
	this.modal.get_content().appendChild(this.settings.buttons.select_none);

	this.clone_button.type="button";
	this.clone_button.className="btn btn-primary";
	this.clone_button.disabled=true;
	this.clone_button.value="Clone";
	this.clone_button.onclick=function(){myself.onclone_m();};
	this.modal.get_footer().appendChild(this.clone_button);

	this.cancel_button.type="button";
	this.cancel_button.className="btn btn-primary";
	this.cancel_button.value="Cancel";
	this.cancel_button.onclick=function(){myself.hide();};
	this.modal.get_footer().appendChild(this.cancel_button);
}

modal_clone_t.prototype.show=function()
{
	this.modal.set_title("Clone Robot \""+this.clone_target+"\"");
	this.clone_info.has_typed=false;
	this.clone_info.input.value="";
	this.set_input_normal_m();
	this.build_checkboxes_m();
	this.modal.show();
}

modal_clone_t.prototype.hide=function()
{
	this.modal.hide();
}

modal_clone_t.prototype.onclone_m=function()
{
	if(this.onclone)
	{
		var settings=[];

		for(var key in this.settings.checkboxes)
			if(this.settings.checkboxes[key].box.checked)
				settings.push(this.settings.checkboxes[key].value);

		this.onclone(this.clone_info.input.value,settings);
	}

	this.hide();
}

modal_clone_t.prototype.build_checkboxes_m=function()
{
	var myself=this;

	for(var key in this.settings.checkboxes)
	{
		this.settings.div.removeChild(this.settings.checkboxes[key].box);
		this.settings.div.removeChild(this.settings.checkboxes[key].text);
		this.settings.div.removeChild(this.settings.checkboxes[key].br);
	}

	this.settings.checkboxes.length=0;
	this.settings.buttons.select_all.disabled=true;
	this.settings.buttons.select_none.disabled=true;

	try
	{
		send_request("GET","/superstar/",this.clone_target,"?sub",
			function(response)
			{
				var settings=JSON.parse(response);
				settings.sort();

				for(var key in settings)
					myself.create_checkbox_m(settings[key]);

				if(myself.settings.checkboxes.length>0)
				{
					myself.settings.buttons.select_all.disabled=false;
					myself.settings.buttons.select_none.disabled=false;
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
		console.log("modal_clone_t::build_checkboxes_m() - "+error);
	}
}

modal_clone_t.prototype.create_checkbox_m=function(value)
{
	var checkbox={};
	checkbox.value=value;
	checkbox.box=document.createElement("input");
	checkbox.text=document.createTextNode(" "+value);
	checkbox.br=document.createElement("br");

	checkbox.box.type="checkbox";
	checkbox.box.checked=true;
	this.settings.div.appendChild(checkbox.box);

	this.settings.div.appendChild(checkbox.text);

	this.settings.div.appendChild(checkbox.br);

	this.settings.checkboxes.push(checkbox);
}

modal_clone_t.prototype.select_all_m=function()
{
	for(var key in this.settings.checkboxes)
		this.settings.checkboxes[key].box.checked=true;
}

modal_clone_t.prototype.select_none_m=function()
{
	for(var key in this.settings.checkboxes)
		this.settings.checkboxes[key].box.checked=false;
}

modal_clone_t.prototype.check_exists_m=function(name)
{
	this.robots=[];
	var myself=this;

	try
	{
		send_request("GET","/superstar/",".","?sub",
			function(response)
			{
				myself.robots=[];
				myself.robots=JSON.parse(response);

				for(var key in myself.robots)
				{
					if(name==myself.robots[key])
					{
						myself.set_input_warning_m();
						break;
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
		console.log("modal_clone_t::show() - "+error);
	}
}

modal_clone_t.prototype.valid_robot_name_m=function(name)
{
	return isident(name);
}

modal_clone_t.prototype.set_input_normal_m=function()
{
	this.clone_info.div.className="form-group has-feedback";
	this.clone_info.glyph.className="glyphicon form-control-feedback";
	this.clone_info.glyph.style.visibility="hidden";
	this.clone_info.label.innerHTML="";
}

modal_clone_t.prototype.set_input_success_m=function()
{
	this.clone_info.div.className="form-group has-feedback has-success";
	this.clone_info.glyph.className="glyphicon form-control-feedback glyphicon-ok";
	this.clone_info.glyph.style.visibility="visible";
	this.clone_info.label.innerHTML="";
}

modal_clone_t.prototype.set_input_warning_m=function()
{
	this.clone_info.div.className="form-group has-feedback has-warning";
	this.clone_info.glyph.className="glyphicon form-control-feedback glyphicon-warning-sign";
	this.clone_info.glyph.style.visibility="visible";
	this.clone_info.label.innerHTML="Warning, this robot name already exists!";
}

modal_clone_t.prototype.set_input_error_m=function()
{
	this.clone_info.div.className="form-group has-feedback has-error";
	this.clone_info.glyph.className="glyphicon form-control-feedback glyphicon glyphicon-remove";
	this.clone_info.glyph.style.visibility="visible";
	this.clone_info.label.innerHTML="Invalid robot name!";
}