(function(){var imported=document.createElement("script");imported.src="js/xmlhttp.js";document.head.appendChild(imported);})();
(function(){var imported=document.createElement("script");imported.src="js/codemirror/clike_arduino_nxt.js";document.head.appendChild(imported);})();
(function(){var imported=document.createElement("script");imported.src="js/codemirror/addon/edit/matchbrackets.js";document.head.appendChild(imported);})();
(function(){var imported=document.createElement("script");imported.src="js/codemirror/addon/dialog/dialog.js";document.head.appendChild(imported);})();
(function(){var imported=document.createElement("script");imported.src="js/codemirror/addon/search/search.js";document.head.appendChild(imported);})();
(function(){var imported=document.createElement("script");imported.src="js/codemirror/addon/search/searchcursor.js";document.head.appendChild(imported);})();
(function(){var imported=document.createElement("link");imported.rel="stylesheet";imported.href="js/codemirror/codemirror.css";document.head.appendChild(imported);})();
(function(){var imported=document.createElement("link");imported.rel="stylesheet";imported.href="js/codemirror/addon/dialog/dialog.css";document.head.appendChild(imported);})();
(function(){var imported=document.createElement("style");imported.appendChild(document.createTextNode(".CodeMirror{border:1px solid #000000;}"));document.head.appendChild(imported);})();
(function(){var imported=document.createElement("style");imported.appendChild(document.createTextNode(".lint-error{background:#ff8888;color:#a00000;padding:1px}\r\n.lint-error-icon{background:#ff0000;color:#ffffff;border-radius:50%;margin-right:7px;}"));document.head.appendChild(imported);})();

function editor_t()
{
	var myself=this;
	myself.div=null;
	myself.textarea=null;
	myself.widgets=[];
	myself.editor=null;
	myself.timeout=null;

	myself.create=function(div)
	{
		if(div)
		{
			myself.div=div;
			myself.div.innerHTML="";

			myself.textarea=document.createElement("textarea");
			myself.textarea.value="";
			myself.div.appendChild(myself.textarea);

			myself.editor=CodeMirror.fromTextArea(myself.textarea,
				{indentUnit:4,indentWithTabs:true,lineNumbers:true,matchBrackets:true,mode:"text/x-arduino"});

			myself.editor.on("change",myself.interval_reset);

			if(myself.editor)
				return true;
			else
				myself.destroy();
		}

		return false;
	};

	myself.interval_reset=function()
	{
		if(myself.editor)
		{
			if(myself.timeout)
				window.clearTimeout(myself.timeout);

			myself.timeout=setTimeout(myself.compile,1000);
		}
	};

	myself.destroy=function()
	{
		myself.clear_errors();
		myself.div=null;
		myself.textarea=null;
		myself.editor=null;

		if(myself.timeout)
			window.clearTimeout(myself.timeout);
	};

	myself.set_value=function(value)
	{
		if(myself.editor&&value)
			myself.editor.setValue(value);
	};

	myself.get_value=function()
	{
		return myself.editor.getValue();
	};

	myself.set_size=function(width,height)
	{
		if(myself.editor)
		{
			if(width)
				myself.editor.setSize(width,null);

			if(height)
				myself.editor.setSize(null,height);
		}
	};

	myself.clear_errors=function()
	{
		if(myself.editor)
		{
			for(var ii=0;ii<myself.widgets.length;++ii)
				myself.editor.removeLineWidget(myself.widgets[ii]);
			myself.widgets.length=0;
		}
	};

	myself.add_error=function(line,text)
	{
		if(myself.editor)
		{
			var msg=document.createElement("div");
			var icon=msg.appendChild(document.createElement("span"));
			icon.innerHTML="!!";
			icon.className="lint-error-icon";
			msg.appendChild(document.createTextNode(text));
			msg.className="lint-error";
			myself.widgets.push(myself.editor.addLineWidget(line-1,msg,{coverGutter:false,noHScroll:true}));
		}
	};

	myself.compile=function()
	{
		if(myself.editor)
			send_request("POST","code","",myself.compile_response,myself.get_value(),"application/octet-stream");
	};

	myself.compile_response=function(response)
	{
		if(myself.editor)
		{
			if(myself.timeout)
				window.clearTimeout(myself.timeout);

			try
			{
				var json=JSON.parse(response);

				if(json)
				{
					myself.clear_errors();

					for(var ii=0;ii<json.errors.length;++ii)
						myself.add_error(json.errors[ii].line,json.errors[ii].text);
				}
			}
			catch(e)
			{
				console.log(e);
			}
		}
	};
}