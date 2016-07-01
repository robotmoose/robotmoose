var div=document.getElementById("content");
var uri=parse_uri();
div.innerHTML=uri.message.replace(new RegExp("\n","g"),"<br/>");