/**
  Network communication with generic web server.

  (Superstar-specific communication is now in superstar.js, so this file is unused)
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
							on_reply(decodeURIComponent(xmlhttp.responseText));
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

