function ss_get(superstar,path,on_success,on_error)
{
	var xhr=new XMLHttpRequest();
	xhr.open("GET","http://"+superstar+"/superstar/"+path+"?get",true);
	xhr.onreadystatechange=function()
	{
		if(xhr.readyState==4)
		{
			if(xhr.status==200)
			{
				try
				{
					var str=xhr.responseText;

					if(str=="")
						str="{}";

					var json=JSON.parse(str);

					if(on_success)
						on_success(json);
				}
				catch(error)
				{
					if(on_error)
						on_error("Error parsing response \""+xhr.responseText+"\" ("+error+").");
				}
			}
			else
			{
				if(on_error)
					on_error("Error receiving from server (status "+xhr.status+").");
			}
		}
	}

	xhr.send();
}

function ss_set(superstar,path,json,on_success,on_error)
{
	try
	{
		var json_str=encodeURIComponent(JSON.stringify(json));
		var xhr=new XMLHttpRequest();

		xhr.open("GET","http://"+superstar+"/superstar/"+path+"?set="+json_str,true);
		xhr.onreadystatechange=function()
		{
			if(xhr.readyState==4)
			{
				if(xhr.status==200)
				{
					if(on_success)
						on_success(json);
				}
				else
				{
					if(on_error)
						on_error("Error receiving from server (status "+xhr.status+").");
				}
			}
		}

		xhr.send();
	}
	catch(error)
	{
		if(on_error)
			on_error("Error parsing json to send \""+json+"\".");
	}
}