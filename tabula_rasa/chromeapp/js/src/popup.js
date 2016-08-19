var div=document.getElementById("content");
function gruveo_open()
{
	if(!parse_uri)
	{
		setTimeout(gruveo_open,100);
		return;
	}

	var uri=parse_uri();
	div.innerHTML=uri.message.replace(new RegExp("\n","g"),"<br/>");
}