function modal_t(div)
{
	if(!div)
		return null;

	var myself=this;
	this.div=div;
	this.element=document.createElement("div");
	this.modal=document.createElement("div");
	this.dialog=document.createElement("div");
	this.content=document.createElement("div");
	this.header=document.createElement("div");
	this.body=document.createElement("div");
	this.footer=document.createElement("div");
	this.title_area=document.createElement("h4");
	this.title=document.createTextNode("");
	this.close_button=document.createElement("span");

	this.div.appendChild(this.element);

	this.modal.className="modal fade";
	this.modal.style.verticalAlign="middle";
	this.element.appendChild(this.modal);

	this.dialog.className="modal-dialog";
	this.modal.appendChild(this.dialog);

	this.content.className="modal-content";
	this.dialog.appendChild(this.content);

	this.header.className="modal-header";
	this.content.appendChild(this.header);

	this.body.className="modal-body";
	this.content.appendChild(this.body);

	this.footer.className="modal-footer";
	this.content.appendChild(this.footer);

	this.title_area.className="modal-title";
	this.header.appendChild(this.title_area);

	this.title_area.appendChild(this.title);

	this.close_button.className="close";
	this.close_button.innerHTML="&times;";
	this.close_button.style.float="right";
	this.close_button.onclick=function(event){myself.hide();};
	this.title_area.appendChild(this.close_button);
}

modal_t.prototype.set_title=function(title)
{
	this.title.nodeValue=title;
}

modal_t.prototype.get_content=function()
{
	return this.body;
}

modal_t.prototype.get_footer=function()
{
	return this.footer;
}

modal_t.prototype.show=function()
{
	$(this.modal).find(".modal-dialog").modal('show');
	this.modal.className="modal show";
}

modal_t.prototype.hide=function()
{
	this.modal.className="modal hide";
	$(this.modal).find(".modal-dialog").modal('hide');
}
