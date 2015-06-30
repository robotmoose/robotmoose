
/*
 * Dynamically create a HTML table with javascript
 * Users can append new rows to the table, each row has 2 columns
 * Created for the ITEST project at the Univesity of Alaska, Fairbanks (2015)
 * 
 * Goal: Write control code in a web-browser and have it execute on a Create2
 * 
 * Public Domain 
 */


function state_editor_t(div, robot_name)
{
	this.div = div;
	if(!div || ! robot_name)
		return null;
		
	this.states = [];
	this.robotName = robot_name;
	
	this.table = document.createElement("table");
		this.table.className = "table table-bordered";
		this.table.style.padding = "100px;"; // FIXME: Add padding 
		this.table.id = "states_table";
	this.thead = document.createElement("thead");
	this.trHeadings = document.createElement("tr");
	this.trHeadings.state = document.createElement("th");
	this.trHeadings.state.innerHTML = "State Name";
	this.trHeadings.code = document.createElement("th");
	this.trHeadings.code.innerHTML = "Code";
	var myself = this;
	this.add_row = document.createElement("input");
		this.add_row.type = "button";
		this.add_row.value = "Add State";
		this.add_row.className = "btn btn-primary";
		this.add_row.onclick = function(){myself.addRow();};
	
	
	this.submit_code = document.createElement("input");
	this.submit_code.type = "button";
		this.submit_code.value = "Submit Code";
		this.submit_code.className = "btn btn-primary";
	    this.submit_code.onclick = function(){console.log("Submit button pressed");myself.collectData("send");}; 
		this.submit_code.style.marginLeft = "1%";
		
		
	this.refresh = document.createElement("input");
	this.refresh.type = "button";
		this.refresh.value = "Refresh";
		this.refresh.className = "btn btn-primary";
	    this.refresh.onclick = function(){console.log("Refresh button pressed");myself.refreshData();}; 
		this.refresh.style.marginLeft = "1%";
	
	
	
	// Uncomment for loading and storing to disk functionality 
	
	/*this.save_data = document.createElement("input");
	this.save_data.type = "button";
		this.save_data.value = "Save to disk";
		this.save_data.className = "btn btn-primary";
	    this.save_data.onclick = function(){console.log("Save button pressed");myself.collectData("save");};
		this.save_data.style.marginLeft = "1%";
	this.tbody = document.createElement("tbody");
	
	this.load_data = document.createElement("input");
	this.load_data.type = "button";
		this.load_data.value = "Load from disk";
		this.load_data.className = "btn btn-primary";
		this.load_data.onclick = function(){console.log("Load button pressed");myself.loadData();}; // TODO: Implement loading from disk
		this.load_data.style.marginLeft = "1%";
	*/
	this.tbody = document.createElement("tbody");
	this.div.appendChild(this.table);
	
	this.div.appendChild(this.table);
	this.table.appendChild(this.thead);
	this.thead.appendChild(this.trHeadings)
	this.trHeadings.appendChild(this.trHeadings.state);
	this.trHeadings.appendChild(this.trHeadings.code);
	this.div.appendChild(this.add_row);
	this.div.appendChild(this.submit_code);
	this.div.appendChild(this.refresh);
	
	/*
	this.div.appendChild(this.save_data);
	this.div.appendChild(this.load_data);
	*/
}

state_editor_t.prototype.addRow = function()
{
	var myself = this;
	console.log(this.table.children);
	var rows = document.getElementById("states_table").rows.length;
	console.log("# of rows :"+rows);
	this.row = this.table.insertRow(-1);
	this.cell1 = this.row.insertCell(0);
	this.cell1.id = "state_cell";
	document.getElementById("state_cell").style.width = '1%';
	this.state = document.createElement('input');
	this.state.setAttribute('type','text');
	this.state.setAttribute('value',"");
	this.cell2 = this.row.insertCell(1);
	this.cell2.id = "code_cell";
	document.getElementById("code_cell").style.width = '1%';
	this.code = document.createElement('textarea');
	
	this.cell1.appendChild(this.state);
	this.cell2.appendChild(this.code);

}

/*Walk the table DOM to collect all the data.
 * Check the passed parameter to see if the data has to be sent to superstar or saved to disk 
*/

state_editor_t.prototype.collectData = function(action)
{

	 console.log("Action Requested: "+ action);
	//var table = document.getElementById("states_table");
	var myself = this;
	
	myself.states =[]; //Clear out the array so it doesnt make duplicate data on every update
	
	for(var i = 1; i<this.table.rows.length; i++)
	{
		
		var row = this.table.rows[i];
		
		myself.states.push({name:row.cells[0].children[0].value,code:row.cells[1].children[0].value});
		
	}
	console.log(myself.states);
	console.log("In collectData()");
	
	// If action == save, then save to disk else send off to server 
	if(action =="save")
		myself.saveData();
	else
		myself.sendData();
	
}

state_editor_t.prototype.rebuildTable = function()
{
	console.log("In rebuildTable()");
	var myself = this;
	for (var i =0 ; i< myself.states.length ; i++)
	{
		myself.addRow()
		myself.state.setAttribute('value',myself.states[i].name);
		myself.code.setAttribute('value',myself.states[i].code);
	}
}


state_editor_t.prototype.sendData = function()
{
	console.log("In sendData()");
	var myself = this;
	try
	{
		send_request("GET","/superstar/"+myself.robotName,"states","?set="+JSON.stringify(myself.states),
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

//Rebuild the state table based on values received from superstar 

state_editor_t.prototype.refreshData = function()
{
	var myself=this;
	myself.states = [];

	try
	{
		send_request("GET","/superstar/"+myself.robotName,"states","?get",
			function(response)
			{
				if(response)
				{
					var states_json=JSON.parse(response);
					myself.states.length=0;

					for(var ii=0;ii<states_json.length;++ii)
					{
						if(!states_json[ii].name)
							throw "Could not find state name of json object.";

						if(!states_json[ii].code)
							throw "Could not find state code of json object.";

						myself.states.push(states_json[ii].name,states_json[ii].code);
					}
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
	console.log(myself.states);
	myself.rebuildTable();
}

/*
 * Uncommment for Load and save to disk functionality 
 * 
 * 

state_editor_t.prototype.saveData = function()
{
	var myself = this;
	console.log("In saveData()");
	var blobData = new Blob([JSON.stringify(myself.states)], {type: "application/json"});
	var downloadURL = window.URL.createObjectURL(blobData);
	console.log(downloadURL);
	
	var a = window.document.createElement('a');
	a.href = downloadURL;
	a.download = 'states.json';
	
	document.body.appendChild(a);
	a.click();
	
	document.body.removeChild(a);
	
	this.div.addEventListener('focus',divFocus,false);
	
	// Destroy URL object once download is complete 
	function divFocus()
	{	
		this.div.addEventListener('focus',divFocus,false);
		window.URL.revokeObjectURL(downloadURL);
	}
	
	
}

state_editor_t.prototype.loadData = function()
{
	var myself = this;
	myself.states =[];
	var validFile = false;
	this.file = document.createElement('input');
		this.file.type = "file";
		this.file.className = "btn btn-primary";
		this.file.id = "states_file";
	this.div.appendChild(this.file);
	
	
	this.file.addEventListener('change',updateFile,false);
	
	function updateFile()
	{
		
		var fileName = myself.file.value;
		//for(var i in fileName)
		console.log(fileName);
		var extension = fileName.split(".")[1];
		console.log(extension);
		if(extension != "json")
			alert("Please select a json file");
		else
			validFile = true;
	
	
		if(validFile) // Read file only if its a JSON file 
		{
			this.loadedData = new FileReader();
			//for(i in myself.file)
			//console.log(i);
			
			this.loadedData.onerror = function (){ console.log("Error loading file" + loadData.error.name);}
			this.loadedData.onload = function ()
			{
		
				console.log("load successfull");
				//for(i in this.loadedData)
					console.log(myself.loadedData.result);
				//myself.states = myself.loadedData.result;
				//console.log(myself.loadedData.result);
			}
			this.loadedData.readAsText(myself.file.files[0]);
			console.log(this.loadedData.result);
		}
   }
	
	//statesFile.onchange = function (){ statesFile = document.getElementById("states_file").value;console.log(statesFile);}
	
		
}
*/


