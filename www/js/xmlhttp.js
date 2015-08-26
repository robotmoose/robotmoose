/**
  Network communication with superstar server.

  FIXME: Raise level of abstraction here, by adding something like superstar_get and/or superstar_set
*/

/// Bare XMLHttpRequest wrapper
function send_request(method,path,request,uri,on_reply,on_error,data,content_type)
{
	try
	{
		if(method&&request)
		{
			var xmlhttp=new XMLHttpRequest();

			xmlhttp.onreadystatechange=function()
			{
				if(xmlhttp.readyState==4)
				{
					if(xmlhttp.status==200)
					{
						if(on_reply)
							on_reply(xmlhttp.responseText);
					}
					else if(on_error)
					{
						on_error(xmlhttp.status);
					} else {
						throw "Network error while accessing "+path+"/"+request;
					}
				}
			};

			var request_text=encodeURIComponent(request);

			if(uri)
				request_text+=uri;

			if(path)
				request_text=path+"/"+request_text;

			xmlhttp.open(method,request_text,true);

			if(content_type)
				xmlhttp.setRequestHeader("Content-Type",content_type);

			xmlhttp.send(data);
		}
	}
	catch(e)
	{
		if (on_error) {
			on_error(e);
		}
		else {
			throw e;
		}
	}
}

/**
 Use superstar to set path/element=newObject
 and then call onFinished.
*/
function superstar_set(path,element,newObject,onFinished,auth)
{
	send_request("GET", "/superstar/"+path, element,
		"?set=" + encodeURIComponent(JSON.stringify(newObject)),
		function(replyData) { // reply OK
			if (onFinished) onFinished(replyData);
		},
		undefined, // error function
		undefined, // post data
		"application/json"
	);
}


/**
 Use superstar to get path/element,
 and pass the returned object to onFinished.
*/
function superstar_get(path,element,onFinished)
{
	send_request("GET", "/superstar/"+path, element,
		"?get",
		function(replyData) { // reply OK

			var replyObj=null;
			if(replyData)
				replyObj=JSON.parse(replyData); // fixme: try/catch here
			onFinished(replyObj);
		},
		undefined, // error function
		undefined, // post data
		"application/json"
	);
}

/**
 Use superstar to get sub elements of path,
 and pass the returned object to onFinished.
*/
function superstar_sub(path,onFinished)
{
	send_request("GET", "/superstar/"+path, ".",
		"?sub",
		function(replyData) { // reply OK
			var replyObj=null;
			if(replyData)
				replyObj=JSON.parse(replyData); // fixme: try/catch here
			onFinished(replyObj);
		},
		undefined, // error function
		undefined, // post data
		"application/json"
	);
}


