//Members

function modal_ok_t(div,title,message)
{
	this.modal=new modal_t(div);
	this.message=null;
	this.button=document.createElement("input");

	if(!this.modal)
	{
		this.modal=null;
		return null;
	}

	var myself=this;

	this.modal.set_title(title);

	this.set_message(message);

	this.button.className="btn btn-primary";
	this.button.disabled=false;
	this.button.type="button";
	this.button.value="Ok";
	this.button.onclick=function(){myself.hide();};
	this.modal.get_footer().appendChild(this.button);
}

modal_ok_t.prototype.show=function()
{
	this.modal.show();
}

modal_ok_t.prototype.hide=function()
{
	this.modal.hide();
}

modal_ok_t.prototype.set_message=function(message)
{
	/*
	if(this.message)
		this.modal.get_content().removeChild(this.message);

	this.message=document.createTextNode(message);
	this.modal.get_content().appendChild(this.message); */
	this.modal.get_content().innerHTML = message; 
	// using innerHTML instead of createTextNode to allow for line breaks and hyperlinks
}
