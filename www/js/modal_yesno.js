//Members
//		onok() - triggered when ok button is hit
//		oncancel() - triggered when cancel button is hit

function modal_yesno_t(div,title,message)
{
	this.modal=new modal_t(div);
	this.message=null;
	this.yes_button=document.createElement("input");
	this.no_button=document.createElement("input");

	if(!this.modal)
	{
		this.modal=null;
		return null;
	}

	var myself=this;

	this.modal.set_title(title);

	this.set_message(message);

	this.yes_button.className="btn btn-primary";
	this.yes_button.disabled=false;
	this.yes_button.type="button";
	this.yes_button.value="Yes";
	this.yes_button.onclick=function()
	{
		if(myself.onok)
			myself.onok();

		myself.hide();
	};
	this.modal.get_footer().appendChild(this.yes_button);

	this.no_button.className="btn btn-primary";
	this.no_button.type="button";
	this.no_button.value="No";
	this.no_button.onclick=function()
	{
		if(myself.oncancel)
			myself.oncancel();

		myself.hide();
	};
	this.modal.get_footer().appendChild(this.no_button);
}

modal_yesno_t.prototype.show=function()
{
	this.modal.show();
}

modal_yesno_t.prototype.hide=function()
{
	this.modal.hide();
}

modal_yesno_t.prototype.set_message=function(message)
{
	if(this.message)
		this.modal.get_content().removeChild(this.message);

	this.message=document.createTextNode(message);
	this.modal.get_content().appendChild(this.message);
}