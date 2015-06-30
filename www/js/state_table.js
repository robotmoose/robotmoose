function state_table_t(div,robot_name)
{
	var myself=this;

	this.div=div;
	this.robot_name=robot_name;

	if(!this.div||!this.robot_name)
		return null;

	this.div.style.width=640;

	this.state_list_prettifier=document.createElement("div");
	this.state_list_prettifier.className="form-inline";

	this.state_list=document.createElement("ul");
	this.state_list.className="sortable_handle";
	this.state_list.style.paddingLeft=0;

	this.break0=document.createElement("br");

	this.upload_button=document.createElement("input");
	this.upload_button.type="button";
	this.upload_button.value="Upload";
	this.upload_button.className="btn btn-primary";
	this.upload_button.onclick=function(){myself.upload()};

	this.adder_prettifier=document.createElement("div");
	this.adder_prettifier.className="form-inline";

	this.adder_button=document.createElement("input");
	this.adder_button.type="button";
	this.adder_button.value="Add";
	this.adder_button.disabled=false;
	this.adder_button.className="btn btn-primary";
	this.adder_button.style.marginLeft=10;
	this.adder_button.onclick=function(){myself.create_row("New State","//Code");};

	this.break1=document.createElement("br");

	this.state_list_prettifier.appendChild(this.state_list);
	this.adder_prettifier.appendChild(this.upload_button);
	this.adder_prettifier.appendChild(this.adder_button);
	this.div.appendChild(this.state_list_prettifier);
	this.div.appendChild(this.break0);
	//this.file_manager=new file_manager_t(this.div);
	//this.file_manager.onsave=function(filename){return myself.get_value();};
	//this.file_manager.onload=function(filename,data){myself.set_value(data);};
	this.div.appendChild(this.break1);
	this.div.appendChild(this.adder_prettifier);

	this.row_data=[];

	$("ul.sortable_handle").sortable({handle:'.glyphicon'});

	this.download();
}

state_table_t.prototype.set_value=function(value)
{
	try
	{
		if(!value)
			throw "Value is null.";

		var states_json=JSON.parse(value);

		while(this.state_list.firstChild)
			this.state_list.removeChild(this.state_list.firstChild);

		for(var ii=0;ii<states_json.length;++ii)
		{
			if(!states_json[ii].name)
				throw "Could not find state name of json object.";

			if(!states_json[ii].code)
				throw "Could not find state code of json object.";

			this.create_row(states_json[ii].name,states_json[ii].code);
		}
	}
	catch(error)
	{
		console.log("state_table_t::set_value() - "+error);
		states_json=[];
	}
}

state_table_t.prototype.get_value=function()
{
	var states_json=[];

	try
	{
		for(var ii=0;ii<this.state_list.children.length;++ii)
		{
			var child=this.state_list.children[ii];

			if(!child.state_name)
				throw "Could not find state name of li.";

			if(!child.state_code)
				throw "Could not find state code of li.";

			states_json.push({name:child.state_name.value,code:child.state_code.getValue()});
		}
	}
	catch(error)
	{
		console.log("state_table_t::get_value() - "+error);
		states_json=[];
	}

	return JSON.stringify(states_json);
}

state_table_t.prototype.download=function()
{
	var myself=this;

	try
	{
		send_request("GET","/superstar/"+this.robot_name,"states","?get",
			function(response)
			{
				if(response)
					myself.set_value(response);
			},
			function(error)
			{
				throw error;
			},
			"application/json");
	}
	catch(error)
	{
		console.log("state_table_t::download() - "+error);
	}
}

state_table_t.prototype.upload=function()
{
	try
	{
		var states_data=this.get_value();

		send_request("GET","/superstar/"+this.robot_name,"states","?set="+states_data,
			function(response)
			{
			},
			function(error)
			{
				throw error;
			},
			"application/json");
	}
	catch(error)
	{
		console.log("state_table_t::upload() - "+error);
	}
}

state_table_t.prototype.create_row=function(name,code)
{
	var myself=this;

	(function()
	{
		var li=document.createElement("li");
		li.className="list-group-item";

		var table=document.createElement("table");
		var tr=document.createElement("tr");
		var td0=document.createElement("td");
		var td1=document.createElement("td");
		var td2=document.createElement("td");

		tr.style.verticalAlign="top";

		var handle=document.createElement("span");
		handle.className="glyphicon glyphicon-move";
		td0.appendChild(handle);

		var input=document.createElement("input");
		input.className="form-control";
		input.value=name;
		input.style.marginLeft=20;
		input.style.marginRight=20;
		td0.appendChild(input);

		var textarea=document.createElement("textarea");
		textarea.innerHTML=code;
		td1.appendChild(textarea);

		var code_editor=CodeMirror.fromTextArea(textarea,
			{indentUnit:4,indentWithTabs:true,lineNumbers:true,
				matchBrackets:true,mode:"text/x-javascript"});
		code_editor.setSize(320,240);

		var button=document.createElement("span");
		button.className="close";
		button.innerHTML="x";
		button.li=li;
		button.onclick=function(){myself.state_list.removeChild(li);};
		li.appendChild(button);

		tr.appendChild(td0);
		tr.appendChild(td1);
		tr.appendChild(td2);
		table.appendChild(tr);
		li.appendChild(table);

		li.state_name=input;
		li.state_code=code_editor;

		myself.state_list.appendChild(li);
		code_editor.refresh();
	})();
}

state_table_t.prototype.set_robot_name=function(robot_name)
{
	try
	{
		if(!robot_name)
			throw "config_gui_t::set_robot_name - Invalid robot name."

		this.robot_name=robot_name;

		while(this.state_list.firstChild)
			this.state_list.removeChild(this.state_list.firstChild);

		this.download();
	}
	catch(error)
	{
		console.log(error);
	}
}