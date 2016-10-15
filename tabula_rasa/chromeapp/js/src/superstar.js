//Mike Moss
//10/14/2016
//Contains client code to get requests from a superstar server.

//Superstar object.
//  Variable this.queue to store requests in until .flush is called.
function superstar_t(superstar)
{
	this.superstar=superstar;
	this.superstar=this.fix_url(this.superstar);
	this.queue=[];
	this.comets=[];
}

//Cleans up connections.
superstar_t.prototype.destroy=function()
{
	this.kill_comets();
}

superstar_t.prototype.fix_url=function(url)
{
	if(!url)
		url=window.location.origin;
	if(url.substr(0,9)=="chrome://")
		url="";
	else if(url.substr(0,9)=="127.0.0.1"||url.substr(0,9)=="localhost"
		|| url.substr(0,3)=="10." || url.substr(0,4)=="192.")
			url="http://"+url;
	else if(url.substr(0,7)!="http://"&&url.substr(0,8)!="https://")
		url="https://"+url;
	return url;
}

//Gets the value of path.
//  Calls success_cb on success with the server response.
//  Calls error_cb on error with the server error object (as per spec).
superstar_t.prototype.get=function(path,success_cb,error_cb)
{
	path=this.pathify(path);
	var request=this.build_skeleton_request("get",path);
	this.add_request(request,success_cb,error_cb);
}

//Sets path to the value using the given auth.
//  Calls success_cb on success with the server response.
//  Calls error_cb on error with the server error object (as per spec).
superstar_t.prototype.set=function(path,value,auth,success_cb,error_cb)
{
	path=this.pathify(path);
	var opts=JSON.stringify({value:value});
	var request=this.build_skeleton_request("set",path,opts);
	this.build_auth(path,request,auth);
	this.add_request(request,success_cb,error_cb);
}

//Gets sub keys of the given path.
//  Calls success_cb on success with the server response.
//  Calls error_cb on error with the server error object (as per spec).
superstar_t.prototype.sub=function(path,success_cb,error_cb)
{
	path=this.pathify(path);
	var request=this.build_skeleton_request("sub",path);
	this.add_request(request,success_cb,error_cb);
}

//Pushes the given value onto path using the given auth.
//  Calls success_cb on success with the server response.
//  Calls error_cb on error with the server error object (as per spec).
//  Note, if the path is not an array, it will be after this.
superstar_t.prototype.push=function(path,value,len,auth,success_cb,error_cb)
{
	path=this.pathify(path);
	var opts=JSON.stringify({value:value,length:len});
	var request=this.build_skeleton_request("push",path,opts);
	this.build_auth(path,request,auth)
	this.add_request(request,success_cb,error_cb);
}

//Gets the value of path when it changes.
//  Calls success_cb on success with the server response.
//  Calls error_cb on error with the server error object (as per spec).
superstar_t.prototype.get_next=function(path,last_hash,success_cb,error_cb)
{
	//Build request.
	path=this.pathify(path);
	var request=this.build_skeleton_request("get_next",path);
	request.id=0;
	if(last_hash)
		request.params.last_hash=last_hash;

	//Function to handle errors...
	var handle_error=function(error)
	{
		if(error_cb)
			error_cb(error);
		else
			console.log("Superstar error ("+error.code+") "+error.message);
	}

	//Make the request.
	var _this=this;
	var xmlhttp=new XMLHttpRequest();
	this.comets.push(xmlhttp);
	xmlhttp.onreadystatechange=function()
	{
		if(xmlhttp.readyState==4)
		{
			if(xmlhttp.status==200)
			{
				//Cleanup
				xmlhttp.already_dead=true;
				_this.cleanup_comets();

				try
				{
					//Parse response, call responses.
					var response=JSON.parse(xmlhttp.responseText);

					//Got an array, must be batch data...
					if(response.constructor===Object)
					{
						//Error callback...
						if(response.error)
							handle_error(response.error);

						//Success callback...
						else if(success_cb)
							success_cb(response.result);
					}

					//Server error...
					else
						handle_error(response.error);
				}

				//Handle bad parse or throw...
				catch(error)
				{
					var error_obj=
					{
						code:0,
						message:error
					};
					handle_error(error_obj);
				}
			}

			//Handle bad connection...
			else
			{
				var error_obj=
				{
					code:0,
					message:"HTTP returned "+xmlhttp.status+"."
				};
				handle_error(error_obj);
			}
		}
	};
	xmlhttp.open("POST",this.superstar+"/superstar/",true);
	xmlhttp.send(JSON.stringify(request));

	//Limit to 5 comet connections...
	//  Note, more is likely to stop outstanding comet connections
	//        from getting serviced...
	for(let ii=0;ii<this.comets.length-5;++ii)
		this.comets[ii].needs_to_die=true;
	this.cleanup_comets();

	//Return connection (so user can abort).
	xmlhttp.destroy=function()
	{
		xmlhttp.needs_to_die=true;
	}
	return xmlhttp;
}

//Changes auth for the given path and auth to the given value.
//  Calls success_cb on success with the server response.
//  Calls error_cb on error with the server error object (as per spec).
superstar_t.prototype.change_auth=function(path,value,auth,success_cb,error_cb)
{
	path=this.pathify(path);
	var opts=JSON.stringify({value:value});
	var request=this.build_skeleton_request("change_auth",path,opts);
	this.build_auth(path,request,auth);
	this.add_request(request,success_cb,error_cb);
}

//Replaces multiple slashes in path with a single slash.
//  Removes all leading and trailing slashes.
superstar_t.prototype.pathify=function(path)
{
	if(!path)
		path=""
	path=path.replace(/\/+/g,"\/");
	while(path.length>0&&path[0]=='/')
		path=path.substring(1,path.length);
	while(path.length>0&&path[path.length-1]=='/')
		path=path.substring(0,path.length-1);
	return path;
}

//Builds a basic jsonrpc request with given method.
//  Adds path as path and opts as opts to the params object.
//  Note, opts is optional.
superstar_t.prototype.build_skeleton_request=function(method,path,opts)
{
	path=this.pathify(path);
	var request=
	{
		jsonrpc:"2.0",
		method:method,
		params:
		{
			path:path
		},
		id:null
	};
	if(opts)
		request.params.opts=opts;
	return request;
}

//Builds HMACSHA256 for given path and request object with given auth.
superstar_t.prototype.build_auth=function(path,request,auth)
{
	path=this.pathify(path);
	request.params.auth=auth;
}

//Adds a build request into the batch queue.
//  Note, won't be sent until .flush() is called.
superstar_t.prototype.add_request=function(request,success_cb,error_cb)
{
	this.queue.push
	({
		request:request,
		success_cb:success_cb,
		error_cb:error_cb
	});
}

//Kills comet connections that need to die and removes them from comet array.
superstar_t.prototype.cleanup_comets=function()
{
	var new_comets=[];
	for(let ii in this.comets)
		if(this.comets[ii]&&this.comets[ii].needs_to_die)
			this.comets[ii].abort();
		else if(this.comets[ii]&&!this.comets[ii].already_dead)
			new_comets.push(this.comets[ii]);
	this.comets=new_comets;
}

superstar_t.prototype.kill_comets=function()
{
	for(let ii in this.comets)
		this.comets[ii].abort();
	this.comets=[];
}

//Builds the batch request object and clears out the current queue.
superstar_t.prototype.flush=function()
{
	//No requests, return.
	if(this.queue.length==0)
		return;

	//Build batch of current requests.
	var batch=[];
	for(let ii=0;ii<this.queue.length;++ii)
	{
		var request=this.queue[ii].request;
		var path=request.params.path;
		var opts=request.params.opts;
		var auth=request.params.auth;
		request.id=ii;
		if(auth)
			request.params.auth=CryptoJS.HmacSHA256(path+opts,auth).
				toString(CryptoJS.enc.Hex);
		batch.push(this.queue[ii].request);
	}
	var old_queue=this.queue;
	this.queue=[];

	//Function to handle errors...
	var handle_error=function(request,error)
	{
		if(request.error_cb)
			request.error_cb(error);
		else
			console.log("Superstar error ("+error.code+") "+error.message);
	}

	//Make the request.
	var xmlhttp=new XMLHttpRequest();
	xmlhttp.onreadystatechange=function()
	{
		if(xmlhttp.readyState==4)
		{
			if(xmlhttp.status==200)
			{
				try
				{
					//Parse response, call responses.
					var response=JSON.parse(xmlhttp.responseText);

					//Got an array, must be batch data...
					if(response.constructor===Array)
						for(let key in response)
						{
							if(response[key].id==null)
								continue;

							//Error callback...
							var response_obj=response[key].error;
							if(response_obj)
							{
								handle_error(old_queue[response[key].id],response_obj);
								continue;
							}

							//Success callback...
							response_obj=response[key].result;
							if(old_queue[response[key].id].success_cb)
								old_queue[response[key].id].success_cb(response_obj);
						}

					//Server error...
					else
						for(let key in old_queue)
							handle_error(old_queue[key],response.error);
				}

				//Handle bad parse or throw...
				catch(error)
				{
					var error_obj=
					{
						code:0,
						message:error
					};
					for(let key in old_queue)
						handle_error(old_queue[key],error_obj);
				}
			}

			//Handle bad connection...
			else
			{
				var error_obj=
				{
					code:0,
					message:"HTTP returned "+xmlhttp.status+"."
				};
				for(let key in old_queue)
					handle_error(old_queue[key],error_obj);
			}
		}
	};
	xmlhttp.open("POST",this.superstar+"/superstar/",true);
	xmlhttp.send(JSON.stringify(batch));
}

//Singleton superstar...
var superstar=new superstar_t();








//Hacky wrapper around v1 interface...
setInterval(function()
{
	superstar.flush();
},300);

function robot_to_starpath(robot)
{
	if(!robot)
		robot={};
	var path="/robots/";
	path+=robot.year+"/"+robot.school+"/"+robot.name+"/";
	return path;
}

function robot_set_superstar(url)
{
	if(superstar.fix_url(url)!=superstar.superstar)
	{
		superstar.destroy();
		superstar=new superstar_t(url);
	}
}

function superstar_get(robot,path,on_success,on_error)
{
	superstar.get(robot_to_starpath(robot)+path,on_success,on_error);
}

function superstar_set(robot,path,json,on_success,on_error)
{
	superstar.set(robot_to_starpath(robot)+path,json,robot.auth,on_success,on_error);
}

function superstar_sub(robot,path,on_success,on_error)
{
	superstar.sub(robot_to_starpath(robot)+path,on_success,on_error);
}

function superstar_append(robot,path,json,length,on_success,on_error)
{
	superstar.push(robot_to_starpath(robot)+path,json,length,robot.auth,on_success,on_error);
}
