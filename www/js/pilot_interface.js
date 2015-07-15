//Members
//		onpilot(data) - callback triggered when pilot data needs to be sent

function pilot_interface_t(div)
{
	if(!div)
		return null;

	this.div=div;
	this.element=document.createElement("div");
}

pilot_interface_t.prototype.download=function(robot_name)
{
	if(!robot_name)
		return;

	//Does pilot need to download???
	/*try
	{
		send_request("GET","/superstar/"+robot_name,"pilot","?get",
			function(response)
			{
				if(response)
				{
				}
			},
			function(error)
			{
				console.log("pilot_interface_t::download() - XMLHTTP returned "+error);
			},
			"application/json");
	}
	catch(error)
	{
		console.log("pilot_interface_t::download() - XMLHTTP returned "+error);
	}*/
}

pilot_interface_t.prototype.upload=function(robot_name)
{
	if(!robot_name)
		return;

	try
	{
		send_request("GET","/superstar/"+robot_name,"pilot","?set",
			function(response)
			{
				if(response)
				{
				}
			},
			function(error)
			{
				console.log("pilot_interface_t::upload() - XMLHTTP returned "+error);
			},
			"application/json");
	}
	catch(error)
	{
		console.log("pilot_interface_t::upload() - XMLHTTP returned "+error);
	}
}