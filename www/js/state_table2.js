function state_table2_t(div)
{
	if(!div)
		return null;

	this.div=div;
	this.element=document.createElement("div");
	this.drag_list=new drag_list_t(this.element);
	this.entries=[];

	if(!this.drag_list)
	{
		this.div=null;
		this.element=null;
	}

	this.element.style.width=640;
	this.div.appendChild(this.element);
	var myself=this;
	setInterval(function(){myself.upload("work123");},500);
}

state_table2_t.prototype.get_entries=function()
{
	this.entries.sort(function(lhs,rhs)
	{
		if(lhs&&rhs)
			return lhs.drag_list.li.offsetTop-rhs.drag_list.li.offsetTop;
	});

	return this.entries;
}

state_table2_t.prototype.create_entry=function(state,code)
{
	if(!state||!code)
		return null;

	var entry={};
	entry.drag_list=this.drag_list.create_entry();
	this.create_entry_m(entry,state,code);
	this.entries.push(entry);
	return entry;
}

state_table2_t.prototype.remove_entry=function(entry)
{
	if(!entry)
		return;

	for(var key in this.entries)
	{
		if(this.entries[key]&&this.entries[key].drag_list===entry)
		{
			this.entries[key]=null;
			break;
		}
	}
}

state_table2_t.prototype.create_entry_m=function(entry,state,code)
{
	if(!entry||!entry.drag_list||!state||!code)
		return;

	entry.table={};
	entry.table.table=document.createElement("table");
	entry.table.row=entry.table.table.insertRow(0);
	entry.table.left=entry.table.row.insertCell(0);
	entry.table.right=entry.table.row.insertCell(1);
	entry.input=document.createElement("input");
	entry.textarea=document.createElement("textarea");

	entry.table.row.style.verticalAlign="top";
	entry.table.left.style.paddingRight=10;
	entry.drag_list.content.appendChild(entry.table.table);

	entry.input.className="form-control";
	entry.input.size=10;
	entry.input.value=state;
	entry.table.left.appendChild(entry.input);

	entry.textarea.innerHTML=code;
	entry.table.right.appendChild(entry.textarea);

	entry.code_editor=CodeMirror.fromTextArea(entry.textarea,
		{indentUnit:4,indentWithTabs:true,lineNumbers:true,
			matchBrackets:true,mode:"text/x-javascript"});
	entry.code_editor.setSize(320,100);

	var myself=this;
	entry.drag_list.onremove=function(entry){myself.remove_entry(entry);};
}

state_table2_t.prototype.download=function(robot_name)
{
	if(!robot_name)
		return;

	var myself=this;

	try
	{
		send_request("GET","/superstar/"+robot_name,"states","?get",
			function(response)
			{
				if(response)
				{
					for(var key in myself.entries)
						myself.remove_entry(myself.entries[key]);

					var obj=JSON.parse(response);

					for(var key in obj)
						this.create_entry(obj.state,obj.code);
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
		console.log("state_table2_t::download() - "+error);
	}
}

state_table2_t.prototype.upload=function(robot_name)
{
	if(!robot_name)
		return;

	var data=[];

	this.get_entries();

	try
	{
		var data=[];

		this.get_entries();

		for(var key in this.entries)
		{
			if(this.entries[key])
			{
				var obj={};
				obj.state=this.entries[key].input.value;
				obj.code=this.entries[key].code_editor.getValue();
				data.push(obj);
			}
		}

		send_request("GET","/superstar/"+robot_name,"states",
			"?set="+encodeURIComponent(JSON.stringify(data)),
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