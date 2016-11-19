//Members
//		onconnect(selected_robot,auth) - triggered when connect button is hit

function modal_connect_t(div)
{
	div.style["z-index"]=1000;
	this.superstar_root="/superstar/robots";
	this.schools=[];
	this.robots=[];

	this.modal=new modal_t(div);
	this.year_select=document.createElement("select");
	this.school_select=document.createElement("select");
	this.robot_select=document.createElement("select");
	this.robot_auth_group=document.createElement("div");
	this.robot_auth=document.createElement("input");
	this.robot_auth_span=document.createElement("span");
	this.connect_button=document.createElement("input");
	this.cancel_button=document.createElement("input");
	this.sim_button=document.createElement("input");
	this.change_auth_button=document.createElement("input");

	if(!this.modal)
	{
		this.modal=null;
		return null;
	}

	var _this=this;

	this.modal.set_title("Connect to Robot");

	this.year_select.className="form-control";
	this.year_select.onchange=function(){_this.build_school_list_m();};
	this.modal.get_content().appendChild(this.year_select);

	this.modal.get_content().appendChild(document.createElement("br"));

	this.school_select.className="form-control";
	this.school_select.onchange=function(){_this.build_robot_list_m();};
	this.modal.get_content().appendChild(this.school_select);

	this.modal.get_content().appendChild(document.createElement("br"));

	this.robot_select.className="form-control";
	this.robot_select.onchange=function(){_this.update_disables_m();};
	this.modal.get_content().appendChild(this.robot_select);

	this.modal.get_content().appendChild(document.createElement("br"));

	this.robot_auth_group.className="form-group has-feedback";
	this.modal.get_content().appendChild(this.robot_auth_group);

	this.robot_auth.className="form-control";
	this.robot_auth.type="password";
	this.robot_auth.placeholder="Enter robot authentication";
	this.robot_auth.onkeydown=function(event)
	{
		if(event.keyCode==13)
			_this.connect_button.click();
	};
	this.robot_auth_group.appendChild(this.robot_auth);

	this.auth_error_str="Authentication error connecting to Superstar!\nMake sure your password is correct.";
	this.robot_auth_span.innerHTML=this.auth_error_str;
	this.robot_auth_span.style.color="#800000";
	this.robot_auth_span.style.background="#ffa0a0";
	this.robot_auth_span.style.visibility="hidden";
	this.robot_auth_group.appendChild(this.robot_auth_span);

	this.connect_button.className="btn btn-primary";
	this.connect_button.disabled=true;
	this.connect_button.type="button";
	this.connect_button.value="Connect";
	this.connect_button.onclick=function()
	{
		var robot={};
		robot.superstar=null; // <- means "same server as this page"
		robot.year=get_select_value(_this.year_select);
		robot.school=get_select_value(_this.school_select);
		robot.name=get_select_value(_this.robot_select);
		robot.auth=CryptoJS.SHA256(_this.robot_auth.value).
			toString(CryptoJS.enc.Hex);
		_this.robot_auth.value="";

		// Remember selections for later using localStorage API
		localStorage.previous_year = robot.year;
		localStorage.previous_school = robot.school;
		localStorage.previous_robot = robot.name;

		// Check connection validity
		superstar_set(robot, 'authtest', 'authtest', function()
		{
			if(_this.onconnect)
				_this.onconnect(robot);
			_this.hide();
		},
		function(err)
		{
			if(err.code==-32000)
			{
				_this.show_auth_error();
			}
			else
			{
				$.notify({message:"Superstar error("+err.code+") - "+err.message},
					{type:'danger',z_index:1050});
			}
		})
	};
	this.modal.get_footer().appendChild(this.connect_button);

	/* 
	this.cancel_button.className="btn btn-primary";
	this.cancel_button.type="button";
	this.cancel_button.value="Cancel";
	this.cancel_button.onclick=function(){_this.hide();};
	//this.modal.get_footer().appendChild(this.cancel_button);
	*/

	this.sim_button.className="btn btn-primary";
	this.sim_button.type="button";
	this.sim_button.value="Simulate a Robot";
	this.sim_button.style.left=10;
	this.sim_button.style.float="left";
	this.sim_button.onclick=function()
	{
		var robot= new robot_sim_t();

		robot_network.sim = true;
		if(_this.onconnect)_this.onconnect(robot);
		_this.hide();
	};
	this.modal.get_footer().appendChild(this.sim_button);


	this.change_auth_button.className="btn btn-primary";
	this.change_auth_button.type="button";
	this.change_auth_button.value="Change Authentication";
	this.change_auth_button.style.float="right";

	//this.change_auth_button.overflow="hidden";
	this.change_auth_button.onclick=function()
	{
		var robot={};
		robot.superstar=null; // <- means "same server as this page"
		robot.year=get_select_value(_this.year_select);
		robot.school=get_select_value(_this.school_select);
		robot.name=get_select_value(_this.robot_select);

		_this.modal_change_auth = new modal_change_auth_t(div, robot, function(){_this.show()})
		_this.modal_change_auth.show();
		_this.hide();
		_this.modal_change_auth.modal.close_button.addEventListener("click", function(){_this.show()})
	};

	this.robot_auth_group.style["margin-bottom"]="40px";
	this.robot_auth_group.appendChild(this.change_auth_button);
}

modal_connect_t.prototype.show=function()
{
	var _this=this;
	robot_network.sim=false;

	superstar.sub("/robots/",
		function(year_list)
		{
			_this.years=year_list;
			_this.schools=[];
			_this.robots=[];
			_this.build_year_list_m();
			_this.modal.show();
		},
		function(error)
		{
			throw "Superstar error ("+error.code+") "+error.message;
		});
}

modal_connect_t.prototype.hide=function()
{
	this.modal.hide();
	this.robot_auth_group.className="form-group has-feedback";
	this.robot_auth_span.style.visibility="hidden";
}

modal_connect_t.prototype.build_year_list_m=function()
{
	var _this=this;
	this.year_select.length=0;

	var default_option=document.createElement("option");
	default_option.text="Select a Year";
	this.year_select.appendChild(default_option);
	this.year_select.selectedIndex=0;

	for(let ii=this.years.length-1;ii>=0;--ii)
	{
		var option=document.createElement("option");
		option.text=this.years[ii];
		this.year_select.appendChild(option);
	}

	if(localStorage.previous_year&&is_in_select(this.year_select,localStorage.previous_year))
		this.year_select.value = localStorage.previous_year;
	else if(this.years.length>0)
		this.year_select.selectedIndex=1;

	this.build_school_list_m();
	this.update_disables_m();
}

modal_connect_t.prototype.build_school_list_m=function()
{
	var _this=this;
	this.school_select.length=0;

	var default_option=document.createElement("option");
	default_option.text="Select a School";
	this.school_select.appendChild(default_option);

	this.update_disables_m();

	if(this.year_select.selectedIndex>0)
		superstar.sub("/robots/"+get_select_value(this.year_select),
			function(school_list)
			{
				var select_index;
				_this.schools=school_list;

				for(let key in _this.schools)
				{
					var option=document.createElement("option");
					option.text=_this.schools[key];
					if (_this.schools[key]==localStorage.previous_school && is_in_select(_this.school_select,localStorage.previous_school))
						option.selected = true;
					_this.school_select.appendChild(option);
				}
				_this.build_robot_list_m();
				_this.update_disables_m();
			},
			function(error)
			{
				throw error;
			});
}

modal_connect_t.prototype.build_robot_list_m=function()
{
	var _this=this;
	this.robot_select.length=0;

	var default_option=document.createElement("option");
	default_option.text="Select a Robot";
	this.robot_select.appendChild(default_option);

	this.update_disables_m();

	if(this.school_select.selectedIndex>0)
		superstar.sub("/robots/"+get_select_value(this.year_select)+"/"+get_select_value(this.school_select),
			function(robot_list)
			{
				_this.robots=robot_list;

				for(let key in _this.robots)
				{
					var option=document.createElement("option");
					option.text=_this.robots[key];
					if (_this.robots[key]==localStorage.previous_robot && is_in_select(_this.robot_select,localStorage.previous_robot))
					{
						option.selected = true;
					}

					_this.robot_select.appendChild(option);
				}

				_this.update_disables_m();
			},
			function(error)
			{
				throw error;
			},
			"application/json");
}

modal_connect_t.prototype.update_disables_m=function()
{
	this.school_select.disabled=(this.year_select.selectedIndex==0);
	this.robot_select.disabled=(this.year_select.selectedIndex==0||this.school_select.selectedIndex==0);
	this.connect_button.disabled=(this.year_select.selectedIndex==0||this.school_select.selectedIndex==0||this.robot_select.selectedIndex==0);
	this.change_auth_button.disabled=(this.year_select.selectedIndex==0||this.school_select.selectedIndex==0||this.robot_select.selectedIndex==0);
}

modal_connect_t.prototype.show_auth_error=function()
{
	this.robot_auth_group.className="form-group has-feedback has-error";
	this.robot_auth_span.style.visibility="visible";
	this.robot_auth.focus();
}
