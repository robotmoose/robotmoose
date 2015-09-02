function chatter_t(div,webrtc)
{
	if(!div||!webrtc)
		return null;

	this.div=div;
	this.webrtc=webrtc;

	var myself=this;

	this.ui_table=document.createElement("table");
	this.ui_table.style.margin="0px";
	this.ui_table.style.padding="0px";
	this.ui_table.style.width="100%";
	this.div.appendChild(this.ui_table);

	var row=this.ui_table.insertRow(0);
	row.style.margin="0px";
	row.style.padding="0px";
	row.style.width="100%";

	var left=row.insertCell(0);
	left.style.margin="10px";
	left.style.padding="0px";

	var center=row.insertCell(1);
	center.style.margin="0px";
	center.style.padding="0px";
	center.style.width="100%";

	var right=row.insertCell(2);
	right.style.margin="0px";
	right.style.padding="0px";

	this.name_box=document.createElement("input");
	this.name_box.type="text";
	this.name_box.placeholder="Enter name here.";
	this.name_box.onchange=function(){myself.validate_m();};
	this.name_box.onkeypress=function(){myself.validate_m();};
	this.name_box.onkeyup=function(){myself.validate_m();};
	this.name_box.onkeydown=function(){myself.validate_m();};
	this.name_box.style.width="200px";
	this.name_box.style.marginLeft="-2px";
	this.name_box.style.marginRight="-2px";
	left.appendChild(this.name_box);

	this.send_box=document.createElement("input");
	this.send_box.type="text";
	this.send_box.placeholder="Type message here.";
	this.send_box.onchange=function(){myself.validate_m();};
	this.send_box.onkeypress=function(){myself.validate_m();};
	this.send_box.onkeyup=function(){myself.validate_m();};
	this.send_box.onkeydown=function(event){myself.validate_m();if(event.keyCode==13)myself.send_m();};
	this.send_box.style.width="100%";
	center.appendChild(this.send_box);

	this.send_button=document.createElement("input");
	this.send_button.type="button";
	this.send_button.value="Chat";
	this.send_button.onclick=function(){myself.send_m();};
	this.send_button.style.width="60px";
	this.send_button.style.marginLeft="2px";
	this.send_button.style.marginRight="-2px";
	right.appendChild(this.send_button);

	this.received_box=document.createElement("div");
	this.received_box.style.height="80%";
	this.received_box.style.overflow="scroll";
	this.div.appendChild(this.received_box);

	this.validate_m();
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












chatter_t.prototype.validate_m=function()
{
	this.send_button.disabled=(this.name_box.value.length<=0||this.send_box.value.length<=0);
}

chatter_t.prototype.send_m=function()
{
	if(!this.send_button.disabled)
	{
		var message=
		{
			type:"chat",
			payload:
			{
				name:this.name_box.value,
				message:this.send_box.value
			}
		};

		this.webrtc.sendToAll("chat",message.payload);
		this.send_box.value="";

		this.receive(message);
	}

	this.validate_m();
}