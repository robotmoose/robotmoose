//callbacks
//		onclose() -- called when modal is hidden

function modal_change_auth_t(div, robot, onclose)
{
	var _this = this;

	this.div = div;
	this.robot = robot;
	this.onclose = onclose;

	this.modal=new modal_t(div);
	if(!this.modal)
	{
		this.modal=null;
		return null;
	}
	this.modal.set_title("Change Authentication");


	// form elements

	this.robot_auth_group=document.createElement("div");
	this.robot_auth_group.className="form-group has-feedback";
	this.robot_auth=document.createElement("input");
	this.robot_auth_span=document.createElement("span");

	this.robot_newauth_group=document.createElement("div");
	this.robot_newauth_group.className="form-group has-feedback";
	this.robot_newauth=document.createElement("input");
	this.robot_newauth_span=document.createElement("span");
	this.robot_newauth_span2=document.createElement("span");
	this.robot_newauth_span_text=document.createElement("span");

	this.robot_repeatauth_group=document.createElement("div");
	this.robot_repeatauth_group.className="form-group has-feedback";
	this.robot_repeatauth=document.createElement("input");
	this.robot_repeatauth_span=document.createElement("span");



	// button elements
	this.confirm_button=document.createElement("input");
	this.cancel_button=document.createElement("input");

	// header
	var header_msg = "Robot: " + robot.year + "/" + robot.school + "/" + robot.name + "<br><br>";
	this.header_div = document.createElement("div");
	this.header_div.innerHTML = header_msg;

	this.modal.get_content().appendChild(this.header_div);


	// old auth input
	this.robot_auth.className="form-control";
	this.robot_auth.type="password";
	this.robot_auth.placeholder="Old robot authentication";
	this.robot_auth.onkeydown=function(event)
	{
		if(event.keyCode==13)
			_this.confirm_button.click();
	};
	this.robot_auth_group.appendChild(this.robot_auth);

	this.robot_auth_span.innerHTML="";
	this.robot_auth_span.style.color="#800000";
	this.robot_auth_span.style.background="#ffa0a0";
	this.robot_auth_span.style.visibility="hidden";
	this.robot_auth_group.appendChild(this.robot_auth_span);


	this.modal.get_content().appendChild(this.robot_auth_group);


	// new auth input
	this.robot_newauth.className="form-control";
	this.robot_newauth.type="password";
	this.robot_newauth.placeholder="New authentication";
	this.robot_newauth.id="newauth";
	this.robot_newauth["aria-describedby"]="newauth_span help_block";
	var new_auth_check=function(event)
	{
		var reg = /[^!-~]/;

		if(_this.robot_newauth.value!=_this.robot_repeatauth.value)
		{
			_this.confirm_button.disabled=true;
		}
		else if(_this.robot_newauth.value.match(reg))
		{
			_this.robot_newauth_group.className="form-group has-warning has-feedback";
			_this.robot_newauth_span.className="glyphicon glyphicon-warning-sign form-control-feedback";
			_this.robot_newauth_span2.innerHTML="(warning)";
			_this.robot_newauth_span_text.innerHTML="Invalid character. Authentication may not contain spaces or non-printable characters.";
			_this.confirm_button.disabled=true;
		}
		else if(_this.robot_newauth.value.length<4&&_this.robot_newauth.value!="!"&&_this.robot_newauth.value!="-")
		{
			_this.robot_newauth_group.className="form-group has-warning has-feedback";
			_this.robot_newauth_span.className="glyphicon glyphicon-warning-sign form-control-feedback";
			_this.robot_newauth_span2.innerHTML="(warning)";
			_this.robot_newauth_span_text.innerHTML="Authentication must be at least 4 characters long";
			_this.confirm_button.disabled=true;
		}
		else
		{
			_this.robot_newauth_group.className="form-group has-success has-feedback";
			_this.robot_newauth_span.className="glyphicon glyphicon-ok form-control-feedback";
			_this.robot_newauth_span2.innerHTML="(success)";
			_this.robot_newauth_span_text.innerHTML="";
			_this.confirm_button.disabled=false;
		}
	};
	this.robot_repeatauth.addEventListener("keyup",function(event)
	{
		if(event.keyCode==13)
			_this.confirm_button.click();
		else
			new_auth_check(event);
	});

	var check_auth_confirm=function(event)
	{
		//new_auth_check(event);
		if(_this.robot_repeatauth.value!=_this.robot_newauth.value)
			_this.robot_repeatauth_span.style.visibility="visible";
		else
			_this.robot_repeatauth_span.style.visibility="hidden";
	};
	this.robot_newauth.addEventListener("change",function(event)
	{
		new_auth_check(event);
		check_auth_confirm(event);
	});
	this.robot_newauth.addEventListener("keyup",function(event)
	{
		new_auth_check(event);
		check_auth_confirm(event);
	});

	this.robot_repeatauth.addEventListener("change",check_auth_confirm);
	this.robot_repeatauth.addEventListener("keyup",check_auth_confirm);
	this.robot_newauth_group.appendChild(this.robot_newauth);


		// checkmark/warning glyphs
	this.robot_newauth_span.className="";
	this.robot_newauth_span["aria-hidden"]=true;

	this.robot_newauth_span2.className="sr-only";
	this.robot_newauth_span2.innerHTML="(success)";
	this.robot_newauth_span2.id="newauth_span";

	this.robot_newauth_span_text.className="help-block";
	this.robot_newauth_span_text.id="help_block";
	this.robot_newauth_span_text.innerHTML="";

	this.robot_newauth_group.appendChild(this.robot_newauth_span);
	this.robot_newauth_group.appendChild(this.robot_newauth_span2);
	this.robot_newauth_group.appendChild(this.robot_newauth_span_text);

	this.modal.get_content().appendChild(this.robot_newauth_group);


	// repeat auth input
	this.robot_repeatauth.className="form-control";
	this.robot_repeatauth.type="password";
	this.robot_repeatauth.placeholder="Repeat new authentication";
	this.robot_repeatauth.onkeydown=function(event)
	{
		if(event.keyCode==13)
			_this.confirm_button.click();
	};
	this.robot_repeatauth_group.appendChild(this.robot_repeatauth);

	this.repeatauth_error_str="Repeat authentication does not match.";
	this.robot_repeatauth_span.innerHTML=this.repeatauth_error_str;
	this.robot_repeatauth_span.style.color="#800000";
	this.robot_repeatauth_span.style.background="#ffa0a0";
	this.robot_repeatauth_span.style.visibility="hidden";
	this.robot_repeatauth_group.appendChild(this.robot_repeatauth_span);

	this.modal.get_content().appendChild(this.robot_repeatauth_group);


	// confirm button
	this.confirm_button.className="btn btn-primary";
	//this.confirm_button.disabled=true;
	this.confirm_button.type="button";
	this.confirm_button.value="Confirm";
	this.confirm_button.onclick=function()
	{
		// store auth values and clear input fields

		var robot = _this.robot;

		robot.auth=CryptoJS.SHA256(_this.robot_auth.value).
			toString(CryptoJS.enc.Hex);
		newauth_val=CryptoJS.SHA256(_this.robot_newauth.value).
			toString(CryptoJS.enc.Hex);
		if(_this.robot_newauth.value=="!"||_this.robot_newauth.value=="-")
			newauth_val=_this.robot_newauth.value;
		repeatauth_val=CryptoJS.SHA256(_this.robot_repeatauth.value).toString(CryptoJS.enc.Hex);
		if(_this.robot_repeatauth.value=="!"||_this.robot_repeatauth.value=="-")
			repeatauth_val=_this.robot_repeatauth.value;

		// remove error messages and warning/success labels
		_this.robot_auth_span.style.visibility="hidden";
		_this.robot_repeatauth_span.style.visibility="hidden";
		_this.robot_newauth_group.className="form-group has-feedback";
		_this.robot_newauth_span.className="";
		_this.robot_newauth_span_text.innerHTML="";
		//_this.confirm_button.disabled=true;

		// error function passed to superstar funcs
		_this.onerror = function(err)
		{
			if(err.code==-32000||err.code==-32001) // incorrect authentication
			{
				_this.robot_auth_span.style.visibility="visible"; // display message for incorrect authentication
				if(err.code==-32000)
					this.robot_auth_span.innerHTML="Authentication error connecting to Superstar, make sure your password is correct.";
				else
					this.robot_auth_span.innerHTML="This authentication code cannot be changed.";
				_this.robot_auth.focus();
			}
			else
			{
				$.notify({message:"Superstar error("+err.code+") - "+err.message},
					{type:'danger',z_index:1050});
			}
		}

		// Check connection validity
		superstar_set(robot, 'authtest', 'authtest', function()
		{
			if(newauth_val==repeatauth_val) // check if auth fields match
			{
				var robot_path = "/robots/" + robot.year + "/" + robot.school + "/" + robot.name;
				superstar.change_auth(robot_path, newauth_val, robot.auth, function()
				{
					_this.robot_auth.value="";
					_this.robot_newauth.value="";
					_this.robot_repeatauth.value="";
					_this.onconfirm();
				},
				function(err)
				{
					_this.onerror(err);
				});

			}
		},
		function(err)
		{
			_this.onerror(err);

		});


	};
	this.modal.get_footer().appendChild(this.confirm_button);

	// cancel button
	this.cancel_button.className="btn btn-primary";
	this.cancel_button.disabled=false;
	this.cancel_button.type="button";
	this.cancel_button.value="Cancel";
	this.cancel_button.onclick=function()
	{
		_this.hide();
	};
	this.modal.get_footer().appendChild(this.cancel_button);


}

modal_change_auth_t.prototype.onconfirm=function()
{
	var _this = this;

	// clear modal content
	try
	{
		while(this.modal.get_content().lastChild)
			this.modal.get_content().removeChild(this.modal.get_content().lastChild);

		this.modal.get_footer().removeChild(this.confirm_button);
		this.modal.get_footer().removeChild(this.cancel_button);
	}
	catch(error)
	{
		//Ignoring errors here...
	}


	// display confirmation message
	var text_confirm = document.createTextNode("Authentication Change Confirmed");
	this.modal.get_content().appendChild(text_confirm);

	// OK button
	this.ok_button = document.createElement("input");
	this.ok_button.className="btn btn-primary";
	this.ok_button.disabled=false;
	this.ok_button.type="button";
	this.ok_button.value="OK";
	this.ok_button.onclick=function()
	{
		_this.hide();
		window.removeEventListener("keydown", _this.onkeydown);
	};


	_this.onkeydown = function(event) // Enter = OK click
	{
		if(event.keyCode==13) // Enter
			_this.ok_button.click();
	}

	window.addEventListener("keydown", _this.onkeydown);

	this.modal.get_footer().appendChild(_this.ok_button);

}

modal_change_auth_t.prototype.show=function()
{
	this.modal.show();
	this.robot_auth.focus();
}

modal_change_auth_t.prototype.hide=function()
{
	this.modal.hide();
	this.onclose();
	this.robot_auth_group.className="form-group has-feedback";
	this.robot_auth_span.style.visibility="hidden";
	this.robot_newauth_span.style.visibility="hidden";
	this.robot_repeatauth_span.style.visibility="hidden";
}


