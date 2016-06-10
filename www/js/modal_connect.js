//Members
//		onconnect(selected_robot,auth) - triggered when connect button is hit

function modal_connect_t(div)
{
	this.superstar_root="/superstar/robots";
	this.schools=[];
	this.robots=[];

	this.modal=new modal_t(div);
	this.year_select=document.createElement("select");
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

	this.year_select.className="form-control";
	this.year_select.onchange=function(){myself.build_school_list_m()};
	this.modal.get_content().appendChild(this.year_select);

	this.modal.get_content().appendChild(document.createElement("br"));

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
	if (localStorage.previous_auth) {
		this.robot_auth.value = localStorage.previous_auth;
	}
	this.modal.get_content().appendChild(this.robot_auth);

	this.modal.get_content().appendChild(document.createElement("br"));

	
	this.connect_button.className="btn btn-primary";
	this.connect_button.disabled=true;
	this.connect_button.type="button";
	this.connect_button.value="Connect";
	this.connect_button.onclick=function()
	{
		var robot={};
		robot.superstar=null; // <- means "same server as this page"
		robot.year=get_select_value(myself.year_select);
		robot.school=get_select_value(myself.school_select);
		robot.name=get_select_value(myself.robot_select);
		robot.auth=myself.robot_auth.value;

		// Remember selections for later using localStorage API
		localStorage.previous_year = robot.year;
		localStorage.previous_school = robot.school;
		localStorage.previous_robot = robot.name;
		localStorage.previous_auth = robot.auth;

		if(myself.onconnect)
			myself.onconnect(robot);
		
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
		superstar_sub(null,"",
			function(year_list)
			{
				myself.years=year_list;
				myself.schools=[];
				myself.robots=[];
				myself.build_year_list_m();
				myself.build_school_list_m();
				myself.build_robot_list_m();
				myself.modal.show();
			},
			function(error)
			{
				throw error;
			});
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

modal_connect_t.prototype.build_year_list_m=function()
{
	while(this.year_select.firstChild)
		this.year_select.removeChild(this.year_select.firstChild);

	var default_option=document.createElement("option");
	default_option.text="Select a Year";
	this.year_select.appendChild(default_option);
	this.year_select.selectedIndex=0;

	for(var ii=this.years.length-1;ii>=0;--ii)
	{
		var option=document.createElement("option");
		option.text=this.years[ii];
		this.year_select.appendChild(option);
	}

	if (localStorage.previous_year) {
		this.year_select.value = localStorage.previous_year;
	} else if(this.years.length>0) {
		this.year_select.selectedIndex=1;
	}

	this.update_disables_m();
}

modal_connect_t.prototype.build_school_list_m=function()
{
	var myself=this;

	try
	{
		while(this.school_select.firstChild)
			this.school_select.removeChild(this.school_select.firstChild);

		var default_option=document.createElement("option");
		default_option.text="Select a School";
		this.school_select.appendChild(default_option);

		this.update_disables_m();

		if(this.year_select.selectedIndex!=0)	
			superstar_sub(null,get_select_value(this.year_select),
				function(school_list)
				{
					var select_index;
					myself.schools=school_list;

					for(var key in myself.schools)
					{
						var option=document.createElement("option");
						option.text=myself.schools[key];
						if (localStorage.previous_school == option.text) {
							option.selected = true;
						}
						myself.school_select.appendChild(option);
					}

					myself.update_disables_m();
				},
				function(error)
				{
					throw error;
				});
	}
	catch(error)
	{
		console.log("modal_connect_t::build_school_list_m() - "+error);
	}
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

		this.update_disables_m();
		var school = "";

		if (this.school_select.selectedIndex == 0 && localStorage.previous_school) {
			school = localStorage.previous_school;
		} else if (this.school_select.selectedIndex!=0) {
			school = get_select_value(this.school_select);
		}

		if(school != "")
			superstar_sub(null,get_select_value(this.year_select)+"/"+school,
				function(robot_list)
				{
					myself.robots=robot_list;

					for(var key in myself.robots)
					{
						var option=document.createElement("option");
						option.text=myself.robots[key];
						if (localStorage.previous_robot == option.text) {
							option.selected = true;
						}

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
		console.log("modal_connect_t::build_robot_list_m() - "+error);
	}
}

modal_connect_t.prototype.update_disables_m=function()
{
	this.school_select.disabled=(this.year_select.selectedIndex==0);
	this.robot_select.disabled=(this.year_select.selectedIndex==0||this.school_select.selectedIndex==0);
	this.connect_button.disabled=(this.year_select.selectedIndex==0||this.school_select.selectedIndex==0||this.robot_select.selectedIndex==0);
}
