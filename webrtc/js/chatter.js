function chatter_t(div,webrtc)
{
	if(!div||!webrtc)
		return null;

	this.div=div;
	this.webrtc=webrtc;

	var myself=this;

	this.received_box=document.createElement("div");
	this.received_box.style.height="100px";
	this.received_box.style.overflow="scroll";
	this.div.appendChild(this.received_box);

	this.name_box=document.createElement("input");
	this.name_box.type="text";
	this.name_box.placeholder="Enter name here.";
	this.name_box.onchange=function(){myself.validate();};
	this.name_box.onkeypress=function(){myself.validate();};
	this.name_box.onkeyup=function(){myself.validate();};
	this.name_box.onkeydown=function(){myself.validate();};
	this.name_box.style.marginRight="10px";
	this.div.appendChild(this.name_box);

	this.send_box=document.createElement("input");
	this.send_box.type="text";
	this.send_box.placeholder="Type message here.";
	this.send_box.style.width="75%";
	this.send_box.onchange=function(){myself.validate();};
	this.send_box.onkeypress=function(){myself.validate();};
	this.send_box.onkeyup=function(){myself.validate();};
	this.send_box.onkeydown=function(){myself.validate();};
	this.send_box.style.marginRight="10px";
	this.div.appendChild(this.send_box);

	this.send_button=document.createElement("input");
	this.send_button.type="button";
	this.send_button.value="Send";
	this.send_button.onclick=function(){myself.send();};
	this.div.appendChild(this.send_button);

	this.validate();
}

chatter_t.prototype.validate=function()
{
	this.send_button.disabled=(this.name_box.value.length<=0||this.send_box.value.length<=0);
}

chatter_t.prototype.send=function()
{
	if(!this.send_button.disabled)
	{
		this.webrtc.sendToAll("chat",{name:this.name_box.value,message:this.send_box.value});
		this.send_box.value="";
	}

	this.validate();
}

chatter_t.prototype.receive=function(message)
{
	if(message.type=="chat")
	{
		var bolder=document.createElement("b");

		var name=document.createTextNode(message.payload.name+":  ");
		bolder.appendChild(name);

		var message=document.createTextNode(message.payload.message);

		this.received_box.appendChild(bolder);
		this.received_box.appendChild(message);
		this.received_box.appendChild(document.createElement("br"));
		this.received_box.scrollTop=this.received_box.scrollHeight;
	}
}