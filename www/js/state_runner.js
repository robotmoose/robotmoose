function state_runner_t()
{
}

state_runner_t.prototype.run=function(state_table)
{
	if(!state_table)
		return;

	var states=state_table.get_states();

	console.log("running");

	if(states.length>0)
	{
		console.log("Executing state "+states[0].name);
		state_table.set_active(states[0].name);
	}
}

state_runner_t.prototype.stop=function(state_table)
{
	console.log("stopping");
	state_table.set_active();
}