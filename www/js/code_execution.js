
/* code_execution.js
 * Run code written in the Web-browser 
 * Arsh Chauhan 
 * 07/06/15
 * 
 */


function code_execution_t(statesArray)
{
	this.statesArray = statesArray;
	this.states = [];
	this.code =[];
	this.stringCode = "";
	this.currentState;
}

code_execution_t.prototype.collectSubmission = function()
{
	var myself = this;
	for (var i = 0; i < myself.statesArray.length; i++)
	{
		myself.states[i] = myself.statesArray[i].state;
		myself.code[i] = myself.statesArray[i].code;
	}
	console.log( myself.states);
	console.log( myself.code);
	
}

code_execution_t.prototype.createFunction = function()
{
	var pilot={
	L:0.0,
	R:0.0
	};

var robot={
	pilot:pilot,
	location: {
		x:0,
		y:0,
		z:0,
		angle:0
		}, state: null
	};
	var myself = this;
	myself.robot = robot;
	for (var i = 0; i < myself.statesArray.length; i++)
	{
		myself.stringCode += "var " + myself.states[i] + " = function() { "+ myself.code[i] + "};";
		if(i == 0)
			myself.stringCode += "robot.state = " + myself.states[i] + ";";
		
	}
	eval(myself.stringCode);
	console.log(myself.stringCode);
}

code_execution_t.prototype.runCode = function()
{
	var myself = this;
	myself.robot.state();

}










