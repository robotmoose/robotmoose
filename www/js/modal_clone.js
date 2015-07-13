//Members
//		onclone(selected_robot) - triggered when connect button is hit

function modal_clone_t(div)
{
	this.modal=new modal_t(div);
	this.clone_target={};
	this.clone_target.div=document.createElement("div");
	this.clone_target.input=document.createElement("input");
	this.clone_target.timeout=null;
	this.clone_target.span=document.createElement("span");
	this.clone_button=document.createElement("input");
	this.cancel_button=document.createElement("input");

	if(!this.modal)
	{
		this.modal=null;
		return null;
	}

	var myself=this;

	this.modal.set_title("Clone Robot");

	this.clone_target.div.className="form-group has-feedback";
	this.modal.get_content().appendChild(this.clone_target.div);

	this.set_input_normal_m();
	this.clone_target.div.appendChild(this.clone_target.span);

	this.clone_target.input.type="text";
	this.clone_target.input.className="form-control";
	this.clone_target.input.spellcheck=false;
	this.clone_target.input.placeholder="Enter robot clone's name here..."
	this.clone_target.input.onkeydown=
		this.clone_target.input.onkeypress=
		this.clone_target.input.onkeyup=
		function()
		{
			myself.set_input_normal_m();

			if(myself.clone_target.timeout)
				clearTimeout(myself.clone_target.timeout);

			if(myself.valid_robot_name_m(this.value))
			{
				var input=this;
				myself.clone_target.timeout=setTimeout(function(){myself.check_exists_m(input.value);},300);
				myself.clone_button.disabled=false;
			}
			else
			{
				myself.clone_button.disabled=true;
				myself.set_input_error_m();
			}
		};
	this.clone_target.div.appendChild(this.clone_target.input);

	this.clone_button.className="btn btn-primary";
	this.clone_button.disabled=true;
	this.clone_button.type="button";
	this.clone_button.value="Clone";
	this.clone_button.onclick=function()
	{
		if(myself.onclone)
			myself.onclone(myself.clone_target.input.value);

		myself.hide();
	};
	this.modal.get_footer().appendChild(this.clone_button);

	this.cancel_button.className="btn btn-primary";
	this.cancel_button.type="button";
	this.cancel_button.value="Cancel";
	this.cancel_button.onclick=function(){myself.hide();};
	this.modal.get_footer().appendChild(this.cancel_button);
}

modal_clone_t.prototype.show=function()
{
	this.modal.show();
}

modal_clone_t.prototype.hide=function()
{
	this.modal.hide();
}

modal_clone_t.prototype.check_exists_m=function(name)
{
	this.robots=[];
	var myself=this;

	try
	{
		send_request("GET","/superstar/",".","?get",
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
	this.clone_target.div.className="form-group has-feedback";
	this.clone_target.span.className="glyphicon form-control-feedback";
	this.clone_target.span.style.visibility="hidden";
}

modal_clone_t.prototype.set_input_warning_m=function()
{
	this.clone_target.div.className="form-group has-feedback has-warning";
	this.clone_target.span.className="glyphicon form-control-feedback glyphicon-warning-sign";
	this.clone_target.span.style.visibility="visible";
}

modal_clone_t.prototype.set_input_error_m=function()
{
	this.clone_target.div.className="form-group has-feedback has-error";
	this.clone_target.span.className="glyphicon form-control-feedback glyphicon glyphicon-remove";
	this.clone_target.span.style.visibility="visible";
}