function load_js(js)
{
	var scr=document.createElement("script");
	scr.src=js;
	document.head.appendChild(scr);
};

function load_link(link)
{
	var ln=document.createElement("link");
	ln.rel="stylesheet";
	ln.href=link;
	document.head.appendChild(ln);
};

function load_style(style)
{
	var sty=document.createElement("style");
	sty.appendChild(document.createTextNode(style));
	document.head.appendChild(sty);
};

function load_dependencies()
{
	load_js("/js/xmlhttp.js");
	load_js("/js/jquery/jquery.min.js");
	load_js("/js/codemirror/codemirror.js");
	load_js("/js/bootstrap/bootstrap.min.js");
	load_link("/css/bootstrap.min.css");
	load_js("/js/codemirror/clike_arduino_nxt.js");
	load_js("/js/codemirror/addon/edit/matchbrackets.js");
	load_js("/js/codemirror/addon/dialog/dialog.js");
	load_js("/js/codemirror/addon/search/search.js");
	load_js("/js/codemirror/addon/search/searchcursor.js");
	load_link("/js/codemirror/codemirror.css");
	load_link("/js/codemirror/addon/dialog/dialog.css");
	load_style(".CodeMirror{border:1px solid #000000;}");
	load_style(".lint-error{background:#ff8888;color:#a00000;padding:1px}\r\n.lint-error-icon{background:#ff0000;color:#ffffff;border-radius:50%;margin-right:7px;}");
	load_link("/js/jquery/jquery.sortable.css");
	load_js("/js/jquery/jquery.sortable.min.js");
};

(function(){load_dependencies()})();

function state_table_t(div)
{
	var myself=this;
	this.div=div;

	if(!this.div)
		return null;

	this.div.style.width=480;

	this.state_list_prettifier=document.createElement("div");
	this.state_list_prettifier.className="form-inline";

	this.state_list=document.createElement("ul");
	this.state_list.className="sortable";

	this.break0=document.createElement("br");

	this.upload_button=document.createElement("input");
	this.upload_button.type="button";
	this.upload_button.value="Upload";
	this.upload_button.className="btn btn-primary";
	this.upload_button.onclick=function(){myself.upload()};

	this.adder_prettifier=document.createElement("div");
	this.adder_prettifier.className="form-inline";

	this.adder_button=document.createElement("input");
	this.adder_button.type="button";
	this.adder_button.value="Add";
	this.adder_button.disabled=false;
	this.adder_button.className="btn btn-primary";
	this.adder_button.style.marginLeft=10;

	this.adder_button.onclick=function()
	{myself.create_row("New State","//Code");};

	this.state_list_prettifier.appendChild(this.state_list);
	this.adder_prettifier.appendChild(this.upload_button);
	this.adder_prettifier.appendChild(this.adder_button);
	this.div.appendChild(this.state_list_prettifier);
	this.div.appendChild(this.break0);
	this.div.appendChild(this.adder_prettifier);

	$("ul.sortable").sortable();
}

state_table_t.prototype.upload=function()
{
	this.get_value();
}

state_table_t.prototype.get_value=function()
{
	var states=[];

	try
	{
		for(var ii=0;ii<this.state_list.children.length;++ii)
		{
			var child=this.state_list.children[ii];

			if(child.children.length!=3)
				throw "Row has more than 3 things in it!"

			var input=child.children[0];
			var textarea=child.children[1];

			if(input.tagName!="INPUT")
				throw "First element should be an input!"

			if(textarea.tagName!="TEXTAREA")
				throw "Second element should be a textarea!"

			states.push({name:input.value,code:textarea.innerHTML});
		}
	}
	catch(error)
	{
		console.log("state_table_t::get_value() - "+error);
	}

	console.log(states);

	return states;
}

state_table_t.prototype.create_row=function(name,code)
{
	var myself=this;

	(function()
	{
		var li=document.createElement("li");
		li.className="list-group-item";

		var input=document.createElement("input");
		input.value=name;
		li.appendChild(input);

		var textarea=document.createElement("textarea");
		textarea.innerHTML=code;
		li.appendChild(textarea);

		var button=document.createElement("span");
		button.className="close";
		button.innerHTML="x";
		button.li=li;
		button.onclick=function(){myself.state_list.removeChild(li);};
		li.appendChild(button);

		myself.state_list.appendChild(li);
	})();
}