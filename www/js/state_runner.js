/*
 Execute student code from the "Code" tab,
 of the main robot pilot interface

 Mike Moss & Orion Lawlor, 2015-08 (Public Domain)
*/

function state_runner_t()
{

	var _this=this;
	this.execution_interval=30; // milliseconds between runs

	this.state=null;
	this.continue_state=null;
	this.continue_timeout=null;
	this.state_list=[];
	this.kill=true;
	this.state_start_time_ms=this.get_time_ms();

	var myself=this;
	this.VM={};
	this.VM_pilot=null; // e.g., power commands, shared with manual drive
	this.VM_pilot_last="???";
	this.VM_sensors={};
	this.VM_store={};
	this.VM_UI=null;

	// Send off the pilot data, if it has changed
	myself.pilot_flush=function() {
		//console.log("Pilot flush: "+myself.VM_pilot.power.L);
		var pilot_cur=JSON.stringify(myself.VM_pilot);
		if (pilot_cur!=myself.VM_pilot_last)
		{ // Send off autopilot's driving commands
			myself.VM_pilot_last=pilot_cur;
			if (myself.onpilot) myself.onpilot(myself.VM_pilot);
		}
	}

	// Commit all changed data
	myself.do_writes=function(VM) {
		myself.pilot_flush();

		VM.state_written=VM.state;
	}

	/**
	 seq is used to sequence commands, including delays.
	 The trick is executing code before and after the delay,
	 but we disable side effects on the part of the code (the "phase")
	 that is not currently active.
	*/
	this.VM_seq={};
	var seq=this.VM_seq;

	// Storage for blocking functions.  Index is by code_count.
	seq.store=[];

	// Restart the sequence for a new run
	seq.reset=function() {
		seq.code_count=0; // source code phase (counts up every time through code)
		seq.exec_count=0; // active runtime phase (counts up only after delays)
		seq.exec_start_count=-1; // trails exec_count by 1
	};
	seq.reset();

	// Check if we are we currently active (running)
	seq.current=function() { return seq.code_count==seq.exec_count; };

	// Mark start of a potentially blocking phase.
	//  Returns state storage for this phase.
	seq.block_start=function(VM) {
		if (seq.current() && seq.exec_start_count<seq.exec_count)
		{ // This is the first run of the new phase
			seq.exec_start_count=seq.exec_count;
			seq.store[seq.code_count]={}; // new empty object

			// Commit user's actions before blocking
			myself.do_writes(VM);
		}
		return seq.store[seq.code_count];
	}

	// Advance to the next piece of code
	seq.advance=function() {
		seq.exec_count++;
	}

	// Mark end of blocking section
	seq.block_end=function() {
		seq.code_count++; // increment count for each phase
	}


	// VM navigator: object with functions used by VM for path planning
	this.VM_nav= {};
	var nav = this.VM_nav;

	nav.getTheta=function(x_target, y_target, VM) // get angle between +x axis and target point
	{
		if (!VM||!VM.sensors||!VM.sensors.location)
		{
			console.log("Error: nav.getTheta called without VM sensors");
			return;
		}

		var x_curr = VM.sensors.location.x;
		var y_curr = VM.sensors.location.y;

		var data = {};
		data.x_dist = x_target - x_curr;
		data.y_dist = y_target - y_curr;

		var ratio_y_x = data.y_dist / data.x_dist;

		var atan_deg = Math.atan(ratio_y_x)*180/Math.PI;

		if (x_curr < x_target) data.theta = atan_deg;
		else if (atan_deg >= 0) data.theta = - 180 + atan_deg;
		else data.theta = 180 + atan_deg;



		return data;

	}

	nav.getPhi=function(theta, VM) // get angle and direction (right/left) between current direction and theta
	{
		if (!VM||!VM.sensors||!VM.sensors.location)
		{
			console.log("Error: nav.getPhi called without VM sensors");
			return;
		}

		var data = {};

		var curr = VM.sensors.location.angle;
		if (theta*curr >= 0) // same sign
		{
			if (curr - theta > 0)
			{
				data.dir = "R";
				data.phi = curr - theta;
			}
			else
			{
				data.dir = "L";
				data.phi = theta - curr;
			}
		}
		else if (curr > 0) // curr up, theta down
		{
			if (curr - theta < 180)
			{
				data.dir = "R";
				data.phi = (curr - theta);
			}
			else
			{
				data.dir = "L";
				data.phi = (360 - curr + theta);
			}
		}
		else // theta up, curr down
		{
			if (theta - curr < 180)
			{
				data.dir = "L";
				data.phi = (theta - curr);
			}
			else
			{
				data.dir = "R";
				data.phi = (360 - theta + curr);
			}
		}
		return data;
	}

	nav.turn=function(data, VM) // turn until reach target angle
	{
		var done = false;
		var speed = 25;
		var curr_angle=VM.sensors.location.angle;

		var dist=curr_angle-data.theta;
		while (dist>+180.0) dist-=360.0; // reduce mod 360
		while (dist<-180.0) dist+=360.0;
		if (data.dir == "L")
		{
			speed = -speed;
			dist = -dist;
		}
		var p_constant=0.05;
		var p_multiplier=0.006;
		speed*=dist*p_multiplier+p_constant;
		VM.power.L=+speed; VM.power.R=-speed;
		if (dist <= 0.0)
		{ // done with move
			VM.power.L=VM.power.R=0.0;
			done = true;
		}
		// Commit these new power values:
		myself.do_writes(VM);


		return done;
	}

	nav.forward=function(data, VM)
	{
		var done = false

		var speed = 40;

		var p=new vec3(VM.sensors.location.x,VM.sensors.location.y,0.0); // vector of current position

		var dist=data.dist - 100.0*p.distanceTo(data.start); // distance remaining = starting distance - distance traveled
		var slow_dist=10.0; // scale back on approach
		if (dist<slow_dist) speed*=0.1+0.9*dist/slow_dist;
		//console.log("Forward: distance: "+dist+" -> speed "+speed);
		VM.power.L=VM.power.R=speed;

		if (dist <= 0.0) // done with move
		{
			VM.power.L=VM.power.R=0.0;
			done = true;
		}
		// Commit these new power values:
		myself.do_writes(VM);

		return done;
	}

	nav.checkCell=function(i, j, VM, swap, pad) // returns 0 if obstacle is found in this cell; returns 1 otherwise
	{

		if (swap)
		{
			var t = i;
			i = j;
			j = t;
		}

		var cell_size = myself.grid_data.cell_size;
		var w = myself.grid_data.width;

		var k = j*w + i;

		// check for obstacle
		if (pad) // use padded grid
		{
			if(VM.nav.padded_grid[k])
				return 0;
		}
		else if (myself.grid_data.array[k]) // use actual grid
		{
			return 0;
		}

		if (!myself.grid_data.test_cell) myself.grid_data.test_cell = []; // TESTING - send data back to navigation.js for display

		//myself.grid_data.test_cell[k] = 1; // TESTING

		return 1;
	}

	nav.checkPath=function(x_target, y_target, VM, x_curr0, y_curr0, pad) // returns 0 if obstacle is found in path, or returns 1 if path is clear
	{

		var cell_size = myself.grid_data.cell_size;


		var x_curr = VM.sensors.location.x;
		var y_curr = VM.sensors.location.y;
		if (x_curr0 && y_curr0)
		{
			x_curr = x_curr0;
			y_curr = y_curr0;
		}


		// convert to coordinates where origin is at top left of map

		var rw = myself.grid_data.map_width/2;
		var rh = myself.grid_data.map_height/2;
		var x_curr = x_curr + rw;
		var y_curr= (y_curr - rh)*-1;
		var x_target = x_target + rw;
		var y_target = (y_target - rh)*-1;


		var cell_size = myself.grid_data.cell_size;
		var w = myself.grid_data.width;



		var dx = x_target - x_curr;
		var dy = y_target - y_curr;
		var m = dy/dx;
		var m_abs = Math.abs(m);


		if (m_abs < 1)
		{
		 	var u = x_curr;
		 	var sign_u = Math.sign(dx);
		 	var v = y_curr;
		 	var sign_v = Math.sign(dy);
		 	var u_target = x_target;
		 	var swap = "";
		}
		else
		{
			var u = y_curr;
			var sign_u = Math.sign(dy);
			var v = x_curr;
			var sign_v = Math.sign(dx);
			var u_target = y_target;
			var swap = 1;
			m_abs = 1/m_abs;
		}


	 	var cell_dist_u = (Math.floor(u/cell_size)+1)*cell_size;

	 	if (sign_v>0)
	 		var cell_dist_v = (Math.floor(v/cell_size)+1)*cell_size;
	 	else
	 		var cell_dist_v = Math.floor(v/cell_size)*cell_size;

	 	var e = v*sign_v - cell_dist_v*sign_v + m_abs*(cell_dist_u - u);

		var i = Math.floor(u/cell_size);
		var j = Math.floor(v/cell_size);

		var padding = 0;

		if (!pad) // if no padded grid, add padding
		{
			var wheelbase = myself.grid_data.wheelbase;
			if (!VM.nav.pad)
				VM.nav.pad = Math.ceil(0.5*wheelbase/myself.grid_data.cell_size);
			padding = VM.nav.pad;
		}


		VM.nav.checkCell(i, j, VM, swap, pad); // remove? Should it check current cell?

		// Modified bresenham algorithm
		var i_limit = Math.floor(u_target/cell_size);
		var done = false;
		while (!done)
		{
			i = i + 1*sign_u;
			e = e + m_abs*cell_size;
			if (e > 0)
			{
				//if (!VM.nav.checkCell(i, j, VM, swap, pad)) // extra width
				//	return 0;
				j = j + 1*sign_v;
				for (var j1 = j+padding*sign_v; j1<j; j1-=sign_v)
				{
					if (!VM.nav.checkCell(i-1*sign_u, j1, VM, swap, pad)) // mod to accomodate width
						return 0;
				}

				e = e - cell_size;
			}
			for (var j1 = j-padding; j1 < j+padding+1; j1++)
			{
				if (!VM.nav.checkCell(i, j1, VM, swap, pad))
					return 0;
			}


			if ((sign_u > 0 && i < i_limit) || (sign_u < 0 && i > i_limit))
				done = false;
			else
				done = true;
		}


		return 1;

	}

	nav.padObstacles=function(k_curr, VM) // enlarge obstacles to ensure the robot will not collide with one after A* algorithm
	{
		var k_target = VM.nav.data.k_target;
		var w = myself.grid_data.width;
		var h = myself.grid_data.height;
		var grid_copy = myself.grid_data.array.slice();

		var wheelbase = myself.grid_data.wheelbase;
		if (!VM.nav.pad) VM.nav.pad = Math.ceil(0.5*wheelbase/myself.grid_data.cell_size);
		var pad = VM.nav.pad;
		//if (pad == 1) pad="";
		//console.log("Padding obstacles, grid length: " + grid_copy.length);

		for (var k = 0; k < grid_copy.length; k++)
		{
			if (grid_copy[k]==1)
			{
				var j0 = Math.floor(k/w);
				var i0 = k-j0*w;
				for (var i = i0-pad; i< i0+pad+1; i++)
				{
					for (var j = j0-pad; j< j0+pad+1; j++)
					{
						if (j >= 0 && j < h && i>=0 && i < w)
						{
							var k1 = j*w + i;
							if (grid_copy[k1]==0 && k1!=k_target && k1!=k_curr)
							{
								grid_copy[k1] = 2;
								myself.grid_data.test_cell[k1] = 5;
							}
						}
					}
				}

			}
		}
		for (var k = 0; k < grid_copy.length; k++)
		{
			if (grid_copy[k]==2)
				grid_copy[k] = 1;
		}

		VM.nav.padded_grid=grid_copy;

	}

	nav.aStarDist=function(i,j,x2,y2)
	{
		var cell_size = myself.grid_data.cell_size;
		var x = i*cell_size+cell_size/2;
		var dx = Math.abs(x - x2);
		var y = j*cell_size+cell_size/2;
		var dy = Math.abs(y - y2);
		//console.log("aStarDist - cell_size: " + cell_size + ", x_curr: " + x2 + ", y_curr " + y2 + ", x: " + x + ", y: " + y);
		return Math.sqrt(dx*dx + dy*dy);
	}

	nav.aStarEval=function(dist, i1, j1, diag, VM)
	{

		var result = {};
		var i_target=VM.nav.data.i_target;
		var j_target=VM.nav.data.j_target;
		var x_target=VM.nav.data.x_target;
		var y_target=VM.nav.data.y_target;
		var x_curr=VM.nav.data.x_curr;
		var y_curr=VM.nav.data.y_curr;
		var cell_size = myself.grid_data.cell_size;

		if(i1==i_target && j1 == j_target)
		{
			var result = {}
			result.val = -1;
			result.d = 0;
			return result;
		}
		var d = 0;

		if (dist==0)
			d = VM.nav.aStarDist(i1,j1,x_curr,y_curr);
		else if (diag)
			d = dist + cell_size*Math.sqrt(2);
		else
			d = dist + cell_size;

		var hf = VM.nav.aStarDist(i1,j1,x_target,y_target);
		var val = d + hf;
		result.val = val;
		result.d = d;
		return result;

	}

	nav.aStarVisit=function(obj0, obj, diag, VM)
	{
		var i1 = obj.i;
		var j1 = obj.j;
		var w = myself.grid_data.width;
		var k = j1*w+i1

		if (k == VM.nav.data.k_target) // goal reached
		{
			obj.k0=obj0.k;
			VM.nav.visited[k]=obj;
			return "done";
		}
		if (!VM.nav.expanded[k])  // if not yet expanded
		{
			if (VM.nav.checkCell(i1, j1, VM, 0, 1)) // if cell clear
			{
				if (!VM.nav.visited[k]) // if not yet visited
				{

					var result = VM.nav.aStarEval(obj0.d, i1,j1,diag,VM);
					obj.v = result.val;
					obj.d = result.d;
					obj.k=k;
					obj.k0=obj0.k;
					VM.nav.p_queue.add(obj);
					VM.nav.visited[k]=obj;
					myself.grid_data.test_cell[k]=1; // TESTING
				}
				else // if already visited
				{
					var result = VM.nav.aStarEval(obj0.d, i1,j1,diag,VM);
					if (result.d < VM.nav.visited[k].d) // better path found
					{
						VM.nav.visited[k].v = result.val;
						VM.nav.visited[k].d = result.d;
						VM.nav.visited[k].dir = obj.dir;
						VM.nav.visited[k].k0 = obj0.k;
					}
				}
			}
			else
			{
				VM.nav.expanded[k]=1;
				myself.grid_data.test_cell[k]=3; // TESTING
			}

		}

		return 0;

	}

	nav.aStar=function(obj0, VM, onfinished, counter)
	{
		if(!counter)
			counter=0;

		var w = myself.grid_data.width;
		var h = myself.grid_data.height;
		var cell_size = myself.grid_data.cell_size;
		var i = obj0.i;
		var j = obj0.j;
		var k = obj0.k;
		VM.nav.expanded[k]=1;

		var dist = obj0.d;

		var e_bound = i+1 < w;
		var w_bound = i - 1 >= 0;
		var s_bound = j + 1 < h;
		var n_bound = j - 1 >= 0;
		//console.log("e_bound: " + e_bound);
		//console.log("w_bound: " + w_bound);
		//console.log("s_bound: " + s_bound);
		//console.log("n_bound: " + n_bound);


		if (e_bound) // east
		{
			var obj = {i : i+1, j : j, dir : 0};
			var diag = "";
			if(VM.nav.aStarVisit(obj0, obj, diag, VM) == "done")
			{
				VM.nav.astar_result = 1;
				if (onfinished) onfinished();
				return;
			}
		}
		if (s_bound&&e_bound) // southeast
		{
			var obj = {i : i+1, j : j+1, dir : 1};
			var diag = true;
			if(VM.nav.aStarVisit(obj0, obj, diag, VM) == "done")
			{
				VM.nav.astar_result = 1;
				if (onfinished) onfinished();
				return;
			}
		}
		if (s_bound) // south
		{
			var obj = {i : i, j : j+1, dir : 2};
			var diag = "";
			if(VM.nav.aStarVisit(obj0, obj, diag, VM) == "done")
			{
				VM.nav.astar_result = 1;
				if (onfinished) onfinished();
				return;
			}
		}
		if (s_bound&&w_bound) // southwest
		{
			var obj = {i : i-1, j : j+1, dir : 3};
			var diag = true;
			if(VM.nav.aStarVisit(obj0, obj, diag, VM) == "done")
			{
				VM.nav.astar_result = 1;
				if (onfinished) onfinished();
				return;
			}
		}
		if (w_bound) // west
		{
			var obj = {i : i-1, j : j, dir : 4};
			var diag = "";
			if(VM.nav.aStarVisit(obj0, obj, diag, VM) == "done")
			{
				VM.nav.astar_result = 1;
				if (onfinished) onfinished();
				return;
			}
		}
		if (n_bound&&w_bound) // northwest
		{
			var diag = true;
			var obj = {i : i-1, j : j-1, dir : 5};
			if(VM.nav.aStarVisit(obj0, obj, diag, VM) == "done")
			{
				VM.nav.astar_result = 1;
				if (onfinished) onfinished();
				return;
			}
		}
		if (n_bound) // north
		{
			var obj = {i : i, j : j-1, dir : 6};
			var diag = "";
			if(VM.nav.aStarVisit(obj0, obj, diag, VM) == "done")
			{
				VM.nav.astar_result = 1;
				if (onfinished) onfinished();
				return;
			}
		}
		if (n_bound&&e_bound) // northeast
		{

			var obj = {i : i+1, j : j-1, dir : 7};
			var diag = true;
			if(VM.nav.aStarVisit(obj0, obj, diag, VM) == "done")
			{
				VM.nav.astar_result = 1;
				if (onfinished) onfinished();
				return
			}
		}

		if (VM.nav.p_queue.isEmpty())
		{
			console.log("Error: No path to waypoint found");
			VM.nav.astar_result = -1;
			if (onfinished) onfinished();
			return -1;
		}
		//console.log("Finished round " + counter + " of A*. Size of priority queue: " + VM.nav.p_queue.size);
		obj1 = VM.nav.p_queue.poll();
		counter+=1;

		if (counter > 3000)
		{
			counter=0;
			//console.log("astar: recusion limit reached - setting timeout");
			setTimeout(function(){VM.nav.aStar(obj1, VM, onfinished, counter);},0);
		}
		else


		VM.nav.aStar(obj1, VM, onfinished, counter);


	}

	nav.aStarMakeWaypoint=function(i,j)
	{
		var cell_size = myself.grid_data.cell_size;
		var wp = {};
		wp.k = j*myself.grid_data.width + i;
		wp.x = i*cell_size + cell_size/2;
		wp.y = j*cell_size + cell_size/2;
		wp.x = wp.x - myself.grid_data.map_width/2;
		wp.y = -wp.y + myself.grid_data.map_height/2;
		return wp;

	}

	nav.aStarBuildPath=function(k0, x_target, y_target, VM)
	{

    		var waypoints = [];
    		var wp_t = {};
    		wp_t.x = x_target;
    		wp_t.y = y_target;
    		wp_t.k =  VM.nav.data.k_target;
    		//console.log("k_target: " + VM.nav.data.k_target);
    		//console.log("k0_target: " + VM.nav.visited[VM.nav.data.k_target].k0);
    		var wp_counter=0;
    		waypoints[wp_counter] = wp_t;

    		var k = VM.nav.data.k_target
    		var obj_target = VM.nav.visited[k];
    		myself.grid_data.test_cell[k] = 2;
    		while (k!=k0)
    		{
    			var obj = VM.nav.visited[k];
    			var obj0 = VM.nav.visited[obj.k0];
    			if (!obj0)
    			{
    				console.log("*******************************************************")
    				console.log("building path - k: " + k + ", dir: " + obj.dir);
    				console.log("x_curr: " + VM.sensors.location.x + ", y_curr: " + VM.sensors.location.y + ", x_target: " + x_target + ", y_target: " + y_target);
    			}
    			//console.log("building path - k: " + k + ", dir: " + obj.dir);
    			if (obj0.k!=k0 && obj.dir!=obj0.dir)
    			{
    				wp_counter++;
	    			var wp = nav.aStarMakeWaypoint(obj0.i, obj0.j);
	    			waypoints[wp_counter]=wp;
    			}
    			myself.grid_data.test_cell[k] = 2;
    			k = obj.k0;
    		}
    		myself.grid_data.test_cell[k0] = 2;

    		waypoints.reverse();


    		if (waypoints.length>1) // check if the path is clear between the robot and any waypoint (if so, remove all intermediate waypoints)
    		{
    			for (var i=waypoints.length-1; i > 0; i--)
    			{
    				var wp2 = waypoints[i];
    				if(VM.nav.checkPath(wp2.x, wp2.y, VM, 0, 0, 1))
				{
					waypoints.splice(0, i);
					break;
				}
    			}
    		}

		if (waypoints.length>2) // check if the path is clear between any waypoints (if so, remove all intermediate waypoints)
		{
			for (var i = 0; i < waypoints.length-2; i++)
			{
				for (var j = waypoints.length-1; j > i + 1; j--)
				{
					var wp1 = waypoints[i];
					var wp2 = waypoints[j];
					if (!wp2)
					{
						console.log("Error: attempted to access element j")
						console.log("Waypoints length: " + waypoints.length)
					}
					if(VM.nav.checkPath(wp2.x, wp2.y, VM, wp1.x, wp1.y, 1))
					{
						waypoints.splice(i+1, j-i-1);
						break;
					}
				}
			}
		}


		for (var i in waypoints)
		{
			var index = waypoints[i].k;
			//console.log("setting index k: " + index);
			myself.grid_data.test_cell[waypoints[i].k] = 4;
		}

		VM.nav.waypoints_temp=waypoints;
	}

	nav.findPath=function(x_target, y_target, VM, onfinished)
	{
		// Data structures for A* algorithm
		VM.nav.p_queue = new FastPriorityQueue(function(a,b) {return a["v"] < b["v"]});
		VM.nav.visited=[];
		VM.nav.expanded=[];

		if (!myself.grid_data.test_cell) myself.grid_data.test_cell = []; // for testing (to display colors on map)

		VM.nav.data={};

		var cell_size = myself.grid_data.cell_size;
		var x_curr = VM.sensors.location.x;
		var y_curr = VM.sensors.location.y;

		// convert to coordinates where origin is at top left of map

		var rw = myself.grid_data.map_width/2;
		var rh = myself.grid_data.map_height/2;
		VM.nav.data.x_curr = x_curr + rw;
		VM.nav.data.y_curr= (y_curr - rh)*-1;
		VM.nav.data.x_target = x_target + rw;
		VM.nav.data.y_target = (y_target - rh)*-1;
		VM.nav.data.i_target = Math.floor(VM.nav.data.x_target/cell_size);
		VM.nav.data.j_target = Math.floor(VM.nav.data.y_target/cell_size);

		var w= myself.grid_data.width;
		VM.nav.data.k_target=VM.nav.data.j_target*w + VM.nav.data.i_target;

		var obj = {};
		obj.i = Math.floor(VM.nav.data.x_curr/cell_size);
		obj.j = Math.floor(VM.nav.data.y_curr/cell_size);
		obj.k = obj.j*w + obj.i;
		obj.d=0;
		VM.nav.visited[obj.k]=obj;

		VM.nav.padObstacles(obj.k, VM);

		var onfinished_astar=function()
		{
			if (VM.nav.astar_result == -1)
			{
				console.log("findPath Error: No path found");
				VM.nav.findpath_done=true;
				VM.nav.waypoints_temp=-1;
				return;
			}
			VM.nav.aStarBuildPath(obj.k, x_target, y_target, VM);
			VM.nav.findpath_done=true;
			if (onfinished) onfinished();
		}

		VM.nav.aStar(obj, VM, onfinished_astar);

	}
}

state_runner_t.prototype.set_UI=function (UI_builder) {
	this.VM_UI=UI_builder;
};

state_runner_t.prototype.run=function(robot,state_table)
{
	if(!state_table)
		return;

	var myself=this;

	//state_table.upload(robot);

	state_table.run();
	myself.robot = robot;
	myself.robot.target_wp = 0;

	// Clear out old state
	myself.state=null;
	myself.continue_state=null;
	myself.continue_timeout=null;
	myself.state_list=[];
	myself.kill=false;
	state_table.clear_prints();
	myself.VM_store={};

	myself.run_m(state_table);
}

state_runner_t.prototype.stop=function(state_table)
{
	this.kill=true;

	// Make sure continue doesn't fire after a stop
	this.clear_continue_m();

	state_table.set_active(); // no section is active

	this.VM_seq.reset(); // reset sequencer

	if (this.VM_pilot) { // stop the robot when the code stops running
		this.VM_pilot.power.L=this.VM_pilot.power.R=0.0; // stop drive
		this.pilot_flush();
		this.VM_pilot.cmd=undefined; // stop scripts
		for (var idx in this.VM_pilot.power.pwm) {
			this.VM_pilot.power.pwm[idx]=0; // stop PWM
		}
		if (this.onpilot) this.onpilot(this.VM_pilot);
	}
}

// Utility function: return time in milliseconds
state_runner_t.prototype.get_time_ms=function() {
	return (new Date()).getTime();
}

// Look up this state in our state list, or return null if it's not listed
state_runner_t.prototype.find_state=function(state_name)
{
	for(let key in this.state_list)
	{
		var s=this.state_list[key];
		if(s && s.name==state_name) {
			return s;
		}
	}
	// else not found
	return null;
}

state_runner_t.prototype.run_m=function(state_table)
{
	this.run_start_time_ms=this.get_time_ms();
	this.state_list=state_table.get_states();

	if(this.state_list.length<=0)
	{
		//console.log("no state_list");
		this.stop_m(state_table);
		return;
	}

	if(this.state==null)
		this.state=this.state_list[0].name;

	this.start_state(this.state);

	var myself=this;
	setTimeout(function(){myself.execute_m(state_table);},this.execution_interval);
}

// Request a stop (put actual functionality into stop, above)
state_runner_t.prototype.stop_m=function(state_table)
{
	state_table.onstop_m();
}


// Called when beginning to execute a state (either first time, or when switching states)
state_runner_t.prototype.start_state=function(state_name)
{
	this.VM_UI.start_state(state_name);
	this.VM_seq.reset();
	//console.log("Entering VM state "+state_name);
	this.state_start_time_ms=this.get_time_ms();
}


// Inner code execution driver: prepare student-visible UI, and eval
//  Returns the virtual machine object used to wrap user code
state_runner_t.prototype.make_user_VM=function(code,states)
{
	var myself=this;
	var VM={}; // virtual machine with everything the user can access

// Block access to all parent-created members (e.g., inherited locals)
	for(let key in this)
		VM[key]=undefined;

// Import each of their state names (e.g., "start" state)
	for(let key in states)
		if(states[key])
			VM[states[key].name]=states[key].name;

// Sequencer
	VM.sequencer=this.VM_seq;
	VM.sequencer.code_count=0;

// Navigator
	VM.nav = this.VM_nav;


// Import all needed I/O functionality
	VM.console=console;
	VM.printed_text="";
	VM.print=function(value) {
		if (VM.sequencer.current()) {
			VM.printed_text+=value+"\n";
			// console.log(value+"\n");
		}
	};
	VM.stop=function() {
		if (VM.sequencer.current()) {
			VM.state=null;
		}
	}

	var time_ms=this.get_time_ms();
	VM.time=time_ms - this.state_start_time_ms; // time in state (ms)
	VM.time_run=time_ms - this.run_start_time_ms; // time since "Run" (ms)

	VM.delay=function(ms) {
		var t=VM.sequencer.block_start(VM);

		if (VM.sequencer.current()) {
            for(let key in VM.UI.elements){
                var ele = VM.UI.elements[key] || undefined;
                if(ele.type == "button" && VM.button(ele.name)){
                    VM.sequencer.advance();
                }
            }
			if (!t.time)
			{ // starting a delay
				t.time=VM.time+ms;
			}
			if (VM.time >= t.time)
			{ // done with delay
				VM.sequencer.advance();
			}
		}
		VM.sequencer.block_end();
	}

	VM.pilot=JSON.parse(JSON.stringify(this.VM_pilot));
	VM.pilot.cmd=undefined; // don't re-send scripts
	VM.script=function(cmd,arg) {
		if (VM.sequencer.current()) {
			VM.pilot.cmd={"run":cmd, "arg":arg};
		}
	};

	if (this.VM_sensors) VM.sensors=this.VM_sensors; else VM.sensors={};
	if (this.VM_pilot.power) VM.power=this.VM_pilot.power; else VM.power={};
	VM.store=this.VM_store;
	VM.robot={sensors:VM.sensors, power:VM.power};

	if (VM.sensors.power && VM.sensors.power.neopixel && !VM.power.neopixel) {
		VM.power.neopixel=JSON.parse(JSON.stringify(VM.sensors.power.neopixel));
	}

	// Simple instantanious drive:
	VM.drive=function(speedL,speedR) {
		if (VM.sequencer.current()) {

			// cap speed at 100 percent
			if (speedL > 100.0) speedL = 100.0;
			else if (speedL < -100.0) speedL = -100.0;
			if (speedR > 100.0) speedR = 100.0;
			else if (speedR < -100.0) speedR = -100.0;

			VM.power.L=speedL; VM.power.R=speedR;
		}
	}

	// Drive forward specified distance (cm)
	VM.forward=function(target,speed) {
		if (!target) target=10; // centimeters
		if (!speed) speed=0.4*100; // moderate speed -- scaled for percentage
		else if (speed > 100.0) speed = 100.0; // cap speed at 100 percent
		else if (speed < -100.0) speed = -100.0;

		var t=VM.sequencer.block_start(VM);
		if (VM.sequencer.current()) {
			var p=new vec3(VM.sensors.location.x,VM.sensors.location.y,0.0);
			if (!t.start)
			{ // starting a move
				t.start=p;
			}
			if (target<0) { // drive backwards
				target=-target;
				speed=-speed;
			}
			var dist=target - 100.0*p.distanceTo(t.start);
			var slow_dist=10.0; // scale back on approach
			if (dist<slow_dist) speed*=0.1+0.9*dist/slow_dist;
			//console.log("Forward: distance: "+dist+" -> speed "+speed);
			var trim=myself.VM_pilot.trim/100*speed;
			VM.power.L=VM.power.R=speed;
			if(trim<0)
				VM.power.L+=trim;
			else
				VM.power.R-=trim;

			if (dist <= 0.0)
			{ // done with move
				VM.power.L=VM.power.R=0.0;
				VM.sequencer.advance();
			}
			// Commit these new power values:
			myself.do_writes(VM);
		}
		VM.sequencer.block_end();
	}
	VM.backward=function(target,speed) {
		if (!target) target=10; // centimeters
		VM.forward(-target,speed);
	}

	// Turn right (clockwise) specified distance (deg)
	VM.right=function(target,speed) {
		if (!target) target=90; // degrees

		//Control undershooting
		var min_target=10;
		if (target>0&&target<min_target) target=min_target;
		if (target<0&&target>-min_target) target=-min_target;

		//Hack to make 180 work (since you can only go <180)
		if (target>179&&target<=181)
			target=179;
		if (target<-179&&target>=-181)
			target=-179;

		if (!speed) speed=0.25*100; // --- scaled for percentage
		else if (speed > 100.0) speed = 100.0; // cap speed at 100 percent
		else if (speed < -100.0) speed = -100.0;

		var t=VM.sequencer.block_start(VM);
		if (VM.sequencer.current()) {
			var a=VM.sensors.location.angle;
			if (!t.target)
			{ // starting turn: compute target angle
				t.target=a-target;
			}
			var dist=a-t.target;
			while (dist>+180.0) dist-=360.0; // reduce mod 360
			while (dist<-180.0) dist+=360.0;
			if (target<0) { // drive backwards to turn other way
				speed=-speed;
				dist=-dist;
			}

			var p_constant=0.05;
			var p_multiplier=0.006;
			speed*=dist*p_multiplier+p_constant;
			//console.log("PMult: "+p_multiplier+" Turn distance: "+dist+" -> speed: "+speed);
			VM.power.L=+speed; VM.power.R=-speed;
			if (dist <= 0.0)
			{ // done with move
				VM.power.L=VM.power.R=0.0;
				VM.sequencer.advance();
			}
			// Commit these new power values:
			myself.do_writes(VM);
		}
		VM.sequencer.block_end();
	}
	VM.left=function(target,speed) {
		if (!target) target=90; // degrees
		VM.right(-target,speed);
	}

	// Drive straight to point (cartesian coordinate)
	VM.driveToPoint=function(x_target, y_target)
	{
		var t=VM.sequencer.block_start(VM);
		if (VM.sequencer.current()) {
			if (!t.data) // calculate angle and distance
			{
				t.data = VM.nav.getTheta(x_target, y_target, VM); // get target angle (theta), x y distances
				t.data.x_target = x_target;
				t.data.y_target = y_target;
				var temp = VM.nav.getPhi(t.data.theta, VM); // get angle to turn (phi), direction
				t.data.dir = temp.dir;
				t.data.phi = temp.phi;
				var dist_m = Math.sqrt(t.data.x_dist*t.data.x_dist + t.data.y_dist*t.data.y_dist)
				t.data.dist = dist_m*100;
				t.data.start=new vec3(VM.sensors.location.x,VM.sensors.location.y,0.0); // vector of starting position

			}
			if (!t.done_turn) // turn until
				t.done_turn = VM.nav.turn(t.data, VM);
			else if (!t.done_forward) // move until
				t.done_forward = VM.nav.forward(t.data, VM);
			else // waypoint reached
				VM.sequencer.advance();
		}
		VM.sequencer.block_end();


	};

	// Drive to point (cartesian coordinate) with path finding
	// INCOMPLETE (currently detects if path is clear, but does not use this information)
	VM.driveToPointSmart=function(x_target, y_target)
	{
		var t=VM.sequencer.block_start(VM);

		// check that current and target locations are on map
		if (Math.abs(x_target)*2 > myself.grid_data.map_width || Math.abs(y_target)*2 > myself.grid_data.map_height)
		{
			VM.label("ERROR: Target out of map range for driveToPointSmart() function");
		}
		else if (Math.abs(VM.sensors.location.x)*2 > myself.grid_data.map_width || Math.abs(VM.sensors.location.y)*2 > myself.grid_data.map_height)
		{
			VM.label("ERROR: Robot out of map range for driveToPointSmart() function");
		}
		else{
			var x_target_f = x_target;
			var y_target_f = y_target;


			if (VM.sequencer.current()) {
				if (!t.path_blocked && t.waypoints)
				{
					var wp = t.waypoints[t.waypoint_counter];
					x_target = wp.x;
					y_target = wp.y;
				}
				if(!t.grid_copy || t.waiting || t.grid_copy.toString()!=myself.grid_data.array.toString()) // if first time or obstacles changed
				{
					if (t.waiting || !VM.nav.checkPath(x_target_f, y_target_f, VM)) // path to goal is blocked
					{
						if(!t.waiting)
						{
							VM.nav.waypoints_temp="";
							VM.nav.findpath_done="";
							VM.nav.findPath(x_target_f, y_target_f, VM);
						}
						if(!VM.nav.findpath_done)// findpath not done
						{
							t.waiting=true;
						}
						else // findPath done
						{
							t.waiting="";
							t.waypoints = VM.nav.waypoints_temp;
							if (t.waypoints== -1) // no path found
							{
								t.path_blocked = true;
								VM.label("ERROR in driveToPointSmart(): Path to goal blocked");
								VM.power.L=VM.power.R=0.0;

							}
							else
							{
								t.path_blocked="";
								t.waypoint_counter = 0;

								t.grid_copy = myself.grid_data.array.slice();


								var wp = t.waypoints[t.waypoint_counter];
								x_target = wp.x;
								y_target = wp.y;
								t.data = "";
								t.done_turn = "";
								t.done_forward = "";
							}
						}
					}
					else if (t.waypoints)// path to goal is clear
					{
						x_target = x_target_f;
						y_target = y_target_f;
						t.grid_copy = "";
						t.waypoints = "";
						t.waypoint_counter = "";
						t.data = "";
						t.done_turn = "";
						t.done_forward = "";
					}



				}
				if(!t.path_blocked && !t.waiting)
				{
					if (!t.data) // calculate angle and distance
					{
						t.data = VM.nav.getTheta(x_target, y_target, VM); // get target angle (theta), x y distances
						t.data.x_target = x_target;
						t.data.y_target = y_target;
						console.log("Calculating t.data for new waypoint.")
						console.log("x_target: " + t.data.x_target)
						console.log("y_target: " + t.data.y_target)
						var temp = VM.nav.getPhi(t.data.theta, VM); // get angle to turn (phi), direction
						t.data.dir = temp.dir;
						t.data.phi = temp.phi;
						//console.log("Theta: " + t.data.theta)
						var dist_m = Math.sqrt(t.data.x_dist*t.data.x_dist + t.data.y_dist*t.data.y_dist)
						t.data.dist = dist_m*100;
						t.data.start=new vec3(VM.sensors.location.x,VM.sensors.location.y,0.0); // vector of starting position

					}
					if (!t.done_turn) // turn until
						t.done_turn = VM.nav.turn(t.data, VM);
					else if (!t.done_forward) // move until
						t.done_forward = VM.nav.forward(t.data, VM);
					else // goal reached
					{
						if (t.waypoints)
						{
							if (t.waypoint_counter == t.waypoints.length-1)
							{
								t.waypoints="";
								t.grid_copy="";
								t.waypoint_counter="";
								VM.nav.padded_grid="";
								console.log("driveToSmart(): Final waypoint reached")
								VM.sequencer.advance();
							}
							else
							{
								t.waypoint_counter++;
								t.data="";
								t.done_turn="";
								t.done_forward="";
								console.log("driveToSmart(): Advancing waypoint counter");
							}

						}
						else
							VM.sequencer.advance();
					}
				}

			}
		}
		VM.sequencer.block_end();


	};

	VM.testSuiteSingle=function(x_target, y_target, VM, onfinished)
	{
		//console.log("testing path - x: " + x_target + ", y: " + y_target + ", x_c: " + VM.sensors.location.x + ", y_c: " + VM.sensors.location.y);
		// callback for findpath
		var onfinished_findpath=function(){


			VM.nav.test_t1 = performance.now(); // end timing
			//console.log("ending timer")


			VM.nav.test_time_total+= (VM.nav.test_t1 - VM.nav.test_t0); // record time

				// Advance to next positions
				if(VM.nav.test_x_t >= VM.nav.test_x_lim) // x_target reached limit
				{
					if(VM.nav.test_x_c >= VM.nav.test_x_lim) // x_curr reached limit (finished)
					{
						var t_ave = VM.nav.test_time_total/VM.nav.test_count;

						console.log("Test Suite. Cell size: " + myself.grid_data.cell_size +
						"\nNumber of runs: " + VM.nav.test_count +
						"\nTotal time: " + VM.nav.test_time_total + " ms." +
						"\nAverage time: " + t_ave + " ms.");

						VM.nav.test_suite_done = true;
						if(onfinished) onfinished();
						return;

					}
					else
					{
						VM.nav.test_x_c+=1.0; // advance x_curr
						VM.nav.test_x_t=-VM.nav.test_x_lim; // reset x_target
					}
				}
				else
				{
					VM.nav.test_x_t+=1.0; // advance x_target
				}

				setTimeout(function(){VM.testSuiteSingle(VM.nav.test_x_t, VM.nav.test_y_t, VM, onfinished);},0);
		}
		VM.nav.test_count+=1;
		//console.log("starting timer");
		VM.sensors.location.y = VM.nav.test_y_c;
		VM.sensors.location.x = VM.nav.test_x_c;
		VM.nav.test_t0 = performance.now(); // start timing
		VM.nav.findPath(x_target, y_target, VM, onfinished_findpath);
	}

	VM.testSuite=function() // benchmark A* performance
	{

		if (!VM.nav.test_suite_started) // first time - set parameters
		{
			console.log("Test suite")
			VM.nav.test_t0=0.0; // test start time
			VM.nav.test_t1=0.0; // test end time
			VM.nav.test_time_total=0.0; // total time of all tests
			VM.nav.test_count=0; // number of tests (used for average)
			VM.nav.test_suite_started=true;

			VM.nav.test_x_lim = 8.00;
			VM.nav.test_y_c = 9.00;
			VM.nav.test_y_t = -VM.nav.test_y_c;

			VM.nav.test_x_c = -VM.nav.test_x_lim;
			VM.nav.test_x_t = -VM.nav.test_x_lim;


			//VM.sensors.location.y = VM.nav.test_y_c;
			//VM.sensors.location.x = VM.nav.test_x_c;

			var onfinished_single=function(){
				VM.nav.test_suite_done=true;
				//console.log("test suite finished")
				//VM.stop();
			}

			VM.testSuiteSingle(VM.nav.test_x_t, VM.nav.test_y_t, VM, onfinished_single);

		}


		if (VM.nav.test_suite_done) // finished test suite - display result
		{
			VM.nav.test_suite_started="";
			VM.nav.test_suite_done="";

			VM.stop();
		}


	}




	// UI construction:
	VM.UI=this.VM_UI;
	VM.button=function(name,next_state,opts) {
		var ret=VM.UI.element(name,"button",opts);
		if (next_state && ret.oneshot) {
			//console.log("UI advancing to state "+next_state+" from button "+name);
			VM.state=next_state;
			ret.oneshot=false;
		}
		return ret.value; // mouse up/down boolean
	};
	// Make a checkbox with a label
	VM.checkbox=function(name, bool_v, opts) {
		var ret=VM.UI.element(name,"checkbox",opts);
		if (bool_v)
		{
			var s_arr = bool_v.split(".");
			var v_str;
			if (s_arr[0] === "store") v_str = s_arr[1];
			{
				VM.store[v_str] = ret.dom.checked;

			}
			//else v_str = s_arr[0];
		}
		return ret.dom.checked; // checked/unchecked boolean
	};
	// Make a slider with a label
	VM.slider=function(name,min,start,max,opts){
		opts = opts ||{};
		opts.min = min;
		opts.defaultValue = start;  // starting value
		opts.max = max;
		var numSteps = 100; // number of slider steps
		opts.step = Math.abs(max - min)/numSteps; //slider step size
		var ret=VM.UI.element(name,"slider",opts);
		return parseFloat(ret.dom.value); // returns value of slider
	};
	VM.spinner=function(name, num_v){
		var ret = VM.UI.element(name, "spinner");
		if (num_v)
		{
			var s_arr = num_v.split(".");
			var v_str;
			if (s_arr[0] === "store") v_str = s_arr[1];
			{
				VM.store[v_str] = ret.dom.value;

			}
		}
		return ret.dom.value;
	};
	VM.label=function(name,opts) {
		var ret=VM.UI.element(name,"label",opts);
	};

// Basically eval user's code here
	(new Function(
		"with(this)\n{\n"+  // "with" lets us access VM. stuff directly
			"(function() { \"use strict\";\n"+	// strict mode prevents undefined variables
				code+	// user's code
			"}())\n"+
		"\n}"
	)).call(VM);

	if (VM.sequencer.current()) this.do_writes(VM);

	return VM;
}

// Outer code execution driver: setup and error reporting
state_runner_t.prototype.execute_m=function(state_table)
{
	var _this=this;
	if(!this.kill)
	{
		state_table.clear_error();
		try
		{
			if(!this.state)
				throw("State is null.");

			var run_state=this.find_state(this.state);
			if(!run_state)
				throw("State \""+this.state+"\" not found!");

			// console.log("running state "+this.state);
			state_table.set_active(this.state);

			this.update_continue_m(state_table,run_state);

			var VM=this.make_user_VM(run_state.code+"\n",this.state_list);

			if (VM.sequencer.exec_count>=VM.sequencer.code_count)
			{ // Restart the state if we're at the end of the sequence:
				//console.log("Resetting sequencer back to start");
				_this.do_writes(_this.VM);
				VM.sequencer.reset();
			}

			state_table.show_prints(VM.printed_text,this.state);

			if(VM.state_written===null)
			{
				//user stopped
				this.stop_m(state_table);
				return;
			}

			if(VM.state_written!==undefined)
			{
				if(!this.find_state(VM.state_written))
					throw("Next state \""+VM.state_written+"\" not found!");

				this.clear_continue_m();
				this.state=VM.state_written;
				this.start_state(VM.state_written);
			}

			var myself=this;
			setTimeout(function(){myself.execute_m(state_table);},this.execution_interval);
		}
		catch(error)
		{
			//stop with error
			state_table.show_error(error,this.state);
			console.log("Error! - "+error);
			this.stop_m(state_table);
		}
	}
}

// Advance forward to the next state
state_runner_t.prototype.continue_m=function(state_table)
{
	// console.log("State advance timer callback");
	var found=false;
	var next_state=null;
	this.continue_timeout=null;

	for(let key in this.state_list)
	{
		if(this.state_list[key])
		{
			if(this.state_list[key].name==this.state)
			{
				found=true;
			}
			else if(found)
			{
				next_state=this.state_list[key].name;
				break;
			}
		}
	}
	//console.log("State advanced from "+this.state+" to "+next_state+" due to timer");

	if(!next_state)
		this.stop_m(state_table);

	this.state=next_state;
}

// State run time limiting
state_runner_t.prototype.update_continue_m=function(state_table,state)
{
	var state_time_int=parseInt(state.time,10);
	if(!this.continue_timeout&&state_time_int>0)
	{
		var myself=this;
		this.continue_timeout=setTimeout(function(){myself.continue_m(state_table);},
			state_time_int);
	}
}

state_runner_t.prototype.clear_continue_m=function()
{
	if(this.continue_timeout)
	{
		clearTimeout(this.continue_timeout);
		this.continue_timeout=null;
	}
}
