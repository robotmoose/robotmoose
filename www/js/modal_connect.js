//Members
//		onconnect(selected_robot) - triggered when connect button is hit

function modal_connect_t(div)
{
	this.modal=new modal_t(div);
	this.select=document.createElement("select");
	this.connect_button=document.createElement("input");
	this.cancel_button=document.createElement("input");

	if(!this.modal)
	{
		this.modal=null;
		return null;
	}

	var myself=this;

	this.modal.set_title("Connect to Robot");

	this.select.className="form-control";
	this.select.onchange=function(){myself.update_disables_m();};
	this.modal.get_content().appendChild(this.select);

	this.connect_button.className="btn btn-primary";
	this.connect_button.disabled=true;
	this.connect_button.type="button";
	this.connect_button.value="Connect";
	this.connect_button.onclick=function()
	{
		if(myself.onconnect)
			myself.onconnect(myself.select.options[myself.select.selectedIndex].text);

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
	this.robots=[];
	var myself=this;

	try
	{
		send_request("GET","/superstar/",".","?sub",
			function(response)
			{
				myself.robots=[];
				myself.robots=JSON.parse(response);
				myself.build_robots_list_m();
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

modal_connect_t.prototype.build_robots_list_m=function()
{
	while(this.select.firstChild)
		this.select.removeChild(this.select.firstChild);

	var default_option=document.createElement("option");
	default_option.text="Select a Robot";
	this.select.appendChild(default_option);
	this.select.selectedIndex=0;

	for(var key in this.robots)
	{
		var option=document.createElement("option");
		option.text=this.robots[key];
		this.select.appendChild(option);
	}
}

modal_connect_t.prototype.update_disables_m=function()
{
	this.connect_button.disabled=(this.select.selectedIndex==0);
}