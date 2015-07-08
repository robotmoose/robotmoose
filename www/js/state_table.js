/*
 Draggable state table editor class

 By Mike Moss, 2015-06 (Public Domain)
*/

function make_button(name,onclick,margin) {
	var b=document.createElement("input");
	b.type="button";
	b.value=name;
	b.className="btn btn-primary";
	b.style.marginRight=(margin===undefined)?10:margin;
	b.onclick=onclick;
	return b;
}

function state_table_t(div,robot_name)
{
	if(!div||!robot_name)
		return null;

	var myself=this;

	this.div=div;
	this.robot_name=robot_name;

	this.element=document.createElement("div");
	this.div.appendChild(this.element);

	this.element.style.width=640;

	this.state_list_prettifier=document.createElement("div");
	this.state_list_prettifier.className="form-inline";

	this.state_list=document.createElement("ul");
	this.state_list.className="sortable_handle";
	this.state_list.style.paddingLeft=0;

	this.action_buttons=document.createElement("div");
	this.action_buttons.className="form-inline";
	this.action_buttons.appendChild(make_button("Run",
		function(){if (myself.run) myself.run();}
	));
	/*this.action_buttons.appendChild(make_button("Upload",
		function(){myself.upload();}
	,0));
	this.action_buttons.appendChild(make_button("Download",
		function(){myself.download();}
	));*/
	this.action_buttons.appendChild(make_button("Add State",
		function(){myself.create_row("newState","// JavaScript code\n\n");}
	));

	this.state_list_prettifier.appendChild(this.state_list);
	this.element.appendChild(this.state_list_prettifier);

	//this.break0=document.createElement("br");
	//this.element.appendChild(this.break0);
	//this.file_manager=new file_manager_t(this.element);
	//this.file_manager.onsave=function(filename){return myself.get_value_string();};
	//this.file_manager.onload=function(filename,data){myself.set_value(data);};
	this.break1=document.createElement("br");
	this.element.appendChild(this.break1);
	this.element.appendChild(this.action_buttons);

	this.row_data=[];

	$("ul.sortable_handle").sortable({handle:'.glyphicon'});

	// this.download();
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



state_table_t.prototype.get_value_array=function()
{
	var states_array=[];

	try
	{
		for(var ii=0;ii<this.state_list.children.length;++ii)
		{
			var child=this.state_list.children[ii];

			if(!child.state_name)
				throw "Could not find state name of li.";

			if(!child.state_code)
				throw "Could not find state code of li.";

			 states_array.push({name:child.state_name.value,code:child.state_code.getValue()});
		}
	}
	catch(error)
	{
		console.log("state_table_t::get_value_array() - "+error);
		states_array=[];
	}

	return states_array;
}

state_table_t.prototype.get_value_string=function()
{
	return JSON.stringify(this.get_value_array());
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
				{ // clean out our current state, replace with server version
					while(myself.state_list.firstChild)
						myself.state_list.removeChild(myself.state_list.firstChild);

					myself.set_value(response);
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
		console.log("state_table_t::download() - "+error);
	}
}

state_table_t.prototype.upload=function()
{
	try
	{
		var states_data=this.get_value_string();

		send_request("GET","/superstar/"+this.robot_name,"states",
			"?set="+encodeURIComponent(states_data),
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
		input.size=10;
		input.style.marginLeft=input.style.marginRight=10;
		td0.appendChild(input);

		var textarea=document.createElement("textarea");
		textarea.innerHTML=code;
		textarea.width="100%";
		td1.appendChild(textarea);

		var code_editor=CodeMirror.fromTextArea(textarea,
			{indentUnit:4,indentWithTabs:true,lineNumbers:true,
				matchBrackets:true,mode:"text/x-javascript"});
		code_editor.setSize(320,100);

		var button=document.createElement("span");
		button.className="close";
		button.innerHTML="x";
		button.li=li;
		button.style.marginLeft=10;
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

		// this.download();
	}
	catch(error)
	{
		console.log(error);
	}
}