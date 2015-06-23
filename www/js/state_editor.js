


function state_editor_t(div)
{
	this.div = div;
	if(!div)
		return null;
	
	this.table = document.createElement("table");
		this.table.className = "table table-bordered";
		this.table.style.padding = "100px;"; // FIXME: Add padding 
	this.thead = document.createElement("thead");
	this.trHeadings = document.createElement("tr");
	this.trHeadings.state = document.createElement("th");
	this.trHeadings.state.innerHTML = "State Name";
	this.trHeadings.code = document.createElement("th");
	this.trHeadings.code.innerHTML = "Code";
	this.add_row = document.createElement("input");
		this.add_row.type = "button";
		this.add_row.value = "Add State";
		this.add_row.className = "btn btn-primary";
		this.add_row.onclick = function(){this.addRow();}; // FIXME: Throws this.addRow is not a function 
	
	this.tbody = document.createElement("tbody");
	this.div.appendChild(this.table);
	
	this.div.appendChild(this.table);
	this.table.appendChild(this.thead);
	this.thead.appendChild(this.trHeadings)
	this.trHeadings.appendChild(this.trHeadings.state);
	this.trHeadings.appendChild(this.trHeadings.code);
	this.div.appendChild(this.add_row);
	
	
	
	
}

state_editor_t.prototype.addRow = function()
{
	var myself = this;
	var rows = myself.row.length;
	var table = myself.table;
	
	var row = table.insertRow(rows+1);
	var state_name = row.insertCell(0);
	state_name.type = "text";
	var code = row.insertCell(1);
	code.type = "textarea";
}


