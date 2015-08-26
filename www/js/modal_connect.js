//Members
//		onconnect(selected_robot,auth) - triggered when connect button is hit

function modal_connect_t(div)
{
	this.schools=[];
	this.robots=[];

	this.modal=new modal_t(div);
	this.school_select=document.createElement("select");
	this.robot_select=document.createElement("select");
	this.robot_auth=document.createElement("input");
	this.connect_button=document.createElement("input");
	this.cancel_button=document.createElement("input");

	if(!this.modal)
	{
		this.modal=null;
		return null;
	}

	var myself=this;

	this.modal.set_title("Connect to Robot");

	this.school_select.className="form-control";
	this.school_select.onchange=function(){myself.build_robot_list_m()};
	this.modal.get_content().appendChild(this.school_select);

	this.modal.get_content().appendChild(document.createElement("br"));

	this.robot_select.className="form-control";
	this.robot_select.onchange=function(){myself.update_disables_m();};
	this.modal.get_content().appendChild(this.robot_select);

	this.modal.get_content().appendChild(document.createElement("br"));

	this.robot_auth.className="form-control";
	this.robot_auth.type="text";
	this.robot_auth.placeholder="Enter robot authentication";
	this.modal.get_content().appendChild(this.robot_auth);

	this.modal.get_content().appendChild(document.createElement("br"));

	this.connect_button.className="btn btn-primary";
	this.connect_button.disabled=true;
	this.connect_button.type="button";
	this.connect_button.value="Connect";
	this.connect_button.onclick=function()
	{
		if(myself.onconnect)
			myself.onconnect(myself.school_select.options[myself.school_select.selectedIndex].text+
				"/"+myself.robot_select.options[myself.robot_select.selectedIndex].text,myself.robot_auth.value);

		myself.hide();
	};
	this.modal.get_footer().appendChild(this.connect_button);

	this.cancel_button.className="btn btn-primary";
	this.cancel_button.type="button";
	this.cancel_button.value="Cancel";
	this.cancel_button.onclick=function(){myself.hide();};
	this.modal.get_footer().appendChild(this.cancel_button);
}

modal_connect_t.prototype.show=function()
{
	var myself=this;

	try
	{
		send_request("GET","/superstar/",".","?sub",
			function(response)
			{
				myself.schools=[];
				myself.schools=JSON.parse(response);
				myself.robots=[];
				myself.build_school_list_m();
				myself.build_robot_list_m();
				myself.modal.show();
			},
			function(error)
			{
				throw error;
			},
			"application/json");
	}
	catch(error)
	{
		console.log("modal_connect_t::show() - "+error);
	}
}

modal_connect_t.prototype.hide=function()
{
	this.modal.hide();
}

modal_connect_t.prototype.build_school_list_m=function()
{
	while(this.school_select.firstChild)
		this.school_select.removeChild(this.school_select.firstChild);

	var default_option=document.createElement("option");
	default_option.text="Select a School";
	this.school_select.appendChild(default_option);
	this.school_select.selectedIndex=0;

	for(var key in this.schools)
	{
		var option=document.createElement("option");
		option.text=this.schools[key];
		this.school_select.appendChild(option);
	}

	this.update_disables_m();
}

modal_connect_t.prototype.build_robot_list_m=function()
{
	var myself=this;

	try
	{
		while(this.robot_select.firstChild)
			this.robot_select.removeChild(this.robot_select.firstChild);

		var default_option=document.createElement("option");
		default_option.text="Select a Robot";
		this.robot_select.appendChild(default_option);
		this.robot_select.selectedIndex=0;

		this.update_disables_m();

		if(this.school_select.selectedIndex!=0)
			send_request("GET","/superstar/",this.school_select.options[this.school_select.selectedIndex].text,"?sub",
				function(response)
				{
					myself.robots=JSON.parse(response);

					for(var key in myself.robots)
					{
						var option=document.createElement("option");
						option.text=myself.robots[key];
						myself.robot_select.appendChild(option);
					}

					myself.update_disables_m();
				},
				function(error)
				{
					throw error;
				},
				"application/json");
	}
	catch(error)
	{
		console.log("modal_connect_t::show() - "+error);
	}
}

modal_connect_t.prototype.update_disables_m=function()
{
	this.robot_select.disabled=(this.school_select.selectedIndex==0);
	this.connect_button.disabled=(this.school_select.selectedIndex==0||this.robot_select.selectedIndex==0);
}
