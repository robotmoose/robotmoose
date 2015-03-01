function send_request(method,request,uri,on_reply,on_error,data,content_type)
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
					}
				}
			};

			var request_text=encodeURIComponent(request);

			if(uri)
				request_text+=uri;

			xmlhttp.open(method,request_text,true);

			if(content_type)
				xmlhttp.setRequestHeader("Content-Type",content_type);

			xmlhttp.send(data);
		}
	}
	catch(e)
	{
		on_error(e);
	}
}