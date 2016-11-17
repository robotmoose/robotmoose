/*
 * Accepts a password for robots which require it
 *
 * Callbacks:
 * 	on_change(auth) - Called when the password is changed.
*/

function auth_input_t(div, on_change)
{
	var _this=this;
	this.div=div;
	this.on_change=on_change
	this.input=document.createElement("input");
	this.input.style.width="100%";
	this.input.placeholder="Password";
	this.input.type="password";

	var change=function()
	{
		on_change(CryptoJS.SHA256(this.value).toString(CryptoJS.enc.Hex));
	};
	this.input.addEventListener("change",change);
	change();
	div.appendChild(this.input);
}

auth_input_t.prototype.disable=function()
{
	this.input.disabled=true;
}

auth_input_t.prototype.enable=function()
{
	this.input.disabled=false;
}