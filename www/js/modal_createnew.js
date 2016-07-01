//Members
//		oncreate() - triggered when ok button is hit
//		oncancel() - triggered when cancel button is hit

function modal_createnew_t(div,title,message,oncheck,oncreate,oncancel)
{
	this.modal=new modal_createcancel_t(div,title,message);

	if(!this.modal)
		return null;

	var _this=this;
	this.oncreate=oncreate;
	this.oncancel=oncancel;
	this.modal.oncreate=function()
	{
		if(_this.oncreate)
			_this.oncreate(_this.name.value);
	};
	this.modal.oncancel=function()
	{
		_this.name.value="";
		_this.hide_error();
		if(_this.oncancel)
			_this.oncancel();
	};
	this.modal.modal.get_content().style.height="80px";
	this.oncheck=oncheck;

	this.el=document.createElement("div");
	this.el.className="form-group";
	this.el.style.float="left";
	this.modal.modal.get_content().appendChild(this.el);

	this.name=document.createElement("input");
	this.name.type="text";
	this.name.placeholder="Experiment Name";
	this.name.className="form-control";
	this.name.style.width="320px";
	this.name.spellcheck=false;
	this.name.onchange=function(){_this.update_m(this.value)};
	this.name.onkeydown=function(){_this.update_m(this.value)};
	this.name.onkeyup=function(){_this.update_m(this.value)};
	this.name.onkeypress=function(){_this.update_m(this.value)};
	this.name.addEventListener('keypress',function(e)
	{
		if((e.which||e.keyCode)===13)
			_this.modal.create_button.click();
	});
	this.el.appendChild(this.name);

	this.glyph=document.createElement("span");
	this.el.appendChild(this.glyph);
	this.glyph.className="glyphicon form-control-feedback glyphicon glyphicon-remove";
	this.glyph.style.visibility="hidden";

	this.label=document.createElement("label");
	this.el.appendChild(this.label);
	this.label.style="col-sm-2 control-label";
	this.label.style.color="#800000";

	this.error_span=document.createElement("span");
	this.el.appendChild(this.error_span);
	this.error_span.style.color="#800000";
	this.error_span.style.background="#ffa0a0";
	this.hide_error();
}

modal_createnew_t.prototype.show=function()
{
	this.name.value="";
	this.modal.show();
	this.name.focus();
}

modal_createnew_t.prototype.hide=function()
{
	this.modal.hide();
	this.name.value="";
	this.hide_error();
}

modal_createnew_t.prototype.show_error=function()
{
	this.label.innerHTML="Experiment with this name already exists.";
	this.el.className="form-group has-feedback has-error";
	this.glyph.style.visibility="visible";
	this.modal.create_button.disabled=true;
}

modal_createnew_t.prototype.hide_error=function()
{
	this.label.innerHTML="";
	this.el.className="form-group";
	this.glyph.style.visibility="hidden";
	this.modal.create_button.disabled=false;
}






modal_createnew_t.prototype.update_m=function(value)
{
	this.oncheck(value);
}