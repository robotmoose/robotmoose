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

function tree_viewer_t(div,json)
{
	this.div=div;

	if(!div)
		return null;

	this.json=json;

	this.div.style.width=480;
	this.spacing=-10;
	this.paths=[];
	this.views={};

	this.list=document.createElement("div");
	this.div.appendChild(this.list);

	this.build(this.json,this.list);
}

tree_viewer_t.prototype.create_handle=function(li)
{
	var myself=this;
	var handle=document.createElement("span");
	handle.style.cursor="pointer";
	handle.onclick=function()
	{
		if(li.tree_parent.style.display=="none")
		{
			myself.views[li.tree_path]=true;
			li.tree_parent.style.display="";
			li.tree_handle.className="glyphicon glyphicon-minus";
		}
		else
		{
			myself.views[li.tree_path]=false;
			li.tree_parent.style.display="none";
			li.tree_handle.className="glyphicon glyphicon-plus";
		}
	};

	handle.li=li;
	return handle;
}

tree_viewer_t.prototype.refresh=function(json)
{
	this.json=json;
	this.list.innerHTML="";
	this.build(this.json,this.list);
}

tree_viewer_t.prototype.build=function(json,parent)
{
	var myself=this;

	return (function()
	{
		var ul=document.createElement("ul");
		ul.className="list-group";
		ul.style.marginBottom=-1;
		parent.appendChild(ul);
		myself.spacing+=10;

		for(var key in json)
		{
			if(json[key]!=null&&Object.prototype.toString.call(json[key])==="[object Object]")
			{
				var li=document.createElement("li");
				li.className="list-group-item";
				li.style.marginLeft=myself.spacing;

				var handle=myself.create_handle(li);

				var text=document.createElement("span");
				text.innerHTML=key;
				text.style.paddingLeft=10;

				li.tree_handle=handle;

				li.appendChild(handle);
				li.appendChild(text);
				ul.appendChild(li);

				myself.paths.push(key);
				li.tree_path=JSON.stringify(myself.paths);

				if(myself.views[li.tree_path]==null)
					myself.views[li.tree_path]=false;

				li.tree_parent=myself.build(json[key],ul);

				if(!myself.views[li.tree_path])
				{
					li.tree_parent.style.display="none";
					handle.className="glyphicon glyphicon-plus";
				}
				else
				{
					li.tree_parent.style.display="";
					handle.className="glyphicon glyphicon-minus";
				}

				myself.paths.pop();
			}
			else
			{
				var li=document.createElement("li");
				li.className="list-group-item";
				li.style.marginLeft=myself.spacing;

				var text=document.createElement("span");
				text.innerHTML=key+" = "+json[key];
				text.style.paddingLeft=10;

				li.tree_parent=ul;

				li.appendChild(text);
				ul.appendChild(li);
			}
		}

		myself.spacing-=10;
		return ul;
	})();
}