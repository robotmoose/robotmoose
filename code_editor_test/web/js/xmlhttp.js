function send_request(method,request,uri,on_reply,data,content_type)
{
	if(method&&request)
	{
		var xmlhttp=new XMLHttpRequest();

		xmlhttp.onreadystatechange=function()
		{
			if(xmlhttp.readyState==4&&xmlhttp.status==200&&on_reply)
				on_reply(xmlhttp.responseText);
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