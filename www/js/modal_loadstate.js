//Members
//		onloadfile() - triggered when load button is hit

function modal_loadstate_t(div, state_table)
{
	this.modal=new modal_t(div);
	this.browse_input=document.createElement("input");
	this.name_input=document.createElement("input");
	this.name_used_error=document.createElement("div");
	this.overwrite_label=document.createElement("label");
	this.overwrite_check=document.createElement("input");
	this.load_button=document.createElement("input");
	this.cancel_button=document.createElement("input");

	if(!this.modal)
	{
		this.modal=null;
		return null;
	}

	var myself=this;

	this.modal.set_title("Select file to load");

	this.browse_input.type="file";
	this.browse_input.onchange=function() {
		myself.load_button.disabled=false;
	}
	this.modal.get_content().appendChild(this.browse_input);

	this.name_input.type="text";
	this.name_input.placeholder="Experiment name";
	this.name_input.style.marginTop="5px";
	this.name_input.style.marginBottom="5px";
	this.modal.get_content().appendChild(this.name_input);

	this.name_used_error.className="alert alert-danger";
	this.name_used_error.role="alert";
	this.name_used_error.innerText="An experiment with this name already exists.";
	this.name_used_error.style.display="none";
	this.modal.get_content().appendChild(this.name_used_error);

	this.overwrite_label.innerText="Overwrite experiment if it already exists? ";
	this.overwrite_label.style.display="block";
	this.overwrite_label.appendChild(this.overwrite_check);
	this.modal.get_content().appendChild(this.overwrite_label);
	
	this.overwrite_check.type="checkbox";

	this.load_button.className="btn btn-primary";
	this.load_button.disabled=true;
	this.load_button.type="button";
	this.load_button.value="Load";
	this.load_button.onclick=function()
	{
		if(myself.oncreate)
			myself.oncreate();

		var file = myself.browse_input.files[0];
		var reader = new FileReader();
		reader.readAsText(file);
		reader.onloadend = function()
		{
			try
			{
				var json=JSON.parse(reader.result);
				state_table.experiment_exists(myself.name_input.value, function(doesnt_exist) {
					if (doesnt_exist || myself.overwrite_check.checked) {
						state_table.create_new_experiment(myself.name_input.value);
						state_table.active_experiment=myself.name_input.value;
						state_table.build(json);
						state_table.upload_active_experiment(state_table.active_experiment);
						state_table.upload(state_table.robot);
						state_table.update_states_m();
						myself.modal.hide();
					} else {
						myself.name_used_error.style.display="block";
					}
				});
			}
			catch(error)
			{
				//FIXME: MAKE THIS SHOW AN ERROR
				console.log("LOAD ERROR! - "+error);
			}
		}
	};

	this.modal.get_footer().appendChild(this.load_button);

	this.cancel_button.className="btn btn-primary";
	this.cancel_button.type="button";

	this.cancel_button.value="Cancel";
	this.cancel_button.onclick=function()
	{
		if(myself.oncancel)
			myself.oncancel();

		myself.modal.hide();
	};

	this.modal.close_button.onclick=function()
	{
		if(myself.oncancel)
			myself.oncancel();

		myself.modal.hide();
	}
	this.modal.get_footer().appendChild(this.cancel_button);
}

modal_createcancel_t.prototype.show=function()
{
	this.modal.show();
}

modal_createcancel_t.prototype.hide=function()
{
	this.modal.hide();
}

