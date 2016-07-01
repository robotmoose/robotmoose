//Members
//		oncreate() - triggered when ok button is hit
//		oncancel() - triggered when cancel button is hit

function modal_createcancel_t(div,title,message)
{
	this.modal=new modal_t(div);
	this.message=null;
	this.create_button=document.createElement("input");
	this.cancel_button=document.createElement("input");

	if(!this.modal)
	{
		this.modal=null;
		return null;
	}

	var myself=this;

	this.modal.set_title(title);

	this.set_message(message);

	this.create_button.className="btn btn-primary";
	this.create_button.disabled=false;
	this.create_button.type="button";
	this.create_button.value="Create";
	this.create_button.onclick=function()
	{
		if(myself.oncreate)
			myself.oncreate();

		myself.hide();
	};
	this.modal.get_footer().appendChild(this.create_button);

	this.cancel_button.className="btn btn-primary";
	this.cancel_button.type="button";
	this.cancel_button.value="Cancel";
	this.cancel_button.onclick=function()
	{
		if(myself.oncancel)
			myself.oncancel();

		myself.hide();
	};
	this.modal.close_button.onclick=function()
	{
		if(myself.oncancel)
			myself.oncancel();

		myself.modal.hide();
	}
	this.modal.get_footer().appendChild(this.cancel_button);
}

modal_createcancel_t.prototype.show=function()
{
	this.modal.show();
}

modal_createcancel_t.prototype.hide=function()
{
	this.modal.hide();
}

modal_createcancel_t.prototype.set_message=function(message)
{
	if(this.message)
		this.modal.get_content().removeChild(this.message);

	this.message=document.createTextNode(message);
	this.modal.get_content().appendChild(this.message);
}