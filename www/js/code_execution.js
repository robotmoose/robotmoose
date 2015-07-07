/* code_execution.js
 Run robot control code written in the Web-browser 
 Dr. Orion Lawlor & Arsh Chauhan, 2015-07-06 (Public Domain) 
*/

/**
 Take an array of state objects:
     statesArray:[ { "name": "state1name", "code":"// Javascript code for state 1" } , ...]
*/
function code_execution_t(states,robot)
{
	this.states = states;
	this.robot=robot;
	this.compiled=false;
	this.compile();
}

code_execution_t.prototype.compile = function()
{
	var states_array=this.states;
	var robot=this.robot;
	
	// Each compiled state looks like this sort of object:
	robot.state={
		index:-1,
		name:"invalid_initial_state",
		run:function() { console.log("Robot.state default: Eval failed (syntax error?)."); }
	};
	
	this.compiled_states = [];
	var compiled_states=this.compiled_states;
	this.code="";
	for (var i = 0; i < this.states.length; i++)
	{
		var s=this.states[i];
		this.compiled_states[i]={index:i, name:s.name, code:s.code};
		this.code += "///////// state "+s.name+" ///////////\n";
		this.code += "var " + s.name + " = compiled_states["+i+"];\n";
		this.code += s.name+".run= function() { // user code:\n\n "+ s.code + ";\n\n};\n";
		if (i == 0)
			this.code += "robot.state = " + s.name + "; // set initial state\n";
	}
	try {
		if (this.verbose) console.log("Compiling code: "+this.code);
		eval(this.code);
		this.compiled=true;
	} 
	catch (err) {
		console.log("Compile error "+err+" on user code "+this.code);
	}
}

code_execution_t.prototype.run = function()
{
	if (!this.compiled) return;
	try {
		if (this.verbose) console.log("State: "+this.robot.state.name);
		this.robot.state.run();
	} 
	catch (err) {
		console.log("Run error "+err+" on user code "+this.code);
	}
}










