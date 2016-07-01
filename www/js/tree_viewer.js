//Variable special_types is an object that contains special printing options.
//  For example, to print a key named "bumpers" as binary, you would pass:
//    [{key:"bumper",type:"binary"}]
function tree_viewer_t(div,json,special_types)
{
	this.div=div;

	if(!div)
		return null;

	this.json=json;
	this.special_types=special_types;

	this.div.style.width=480;
	this.indent=50; // pixels of indenting per level
	this.spacing=-this.indent;
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
	handle.onmousedown=function()
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
	if(JSON.stringify(this.json)!=JSON.stringify(json))
	{
		this.json=json;
		while(this.list.firstChild)
			this.list.removeChild(this.list.firstChild);
		this.build(this.json,this.list);
	}
}

tree_viewer_t.prototype.build=function(json,parent)
{
	var myself=this;

	var ul=document.createElement("ul");
	ul.className="list-group";
	ul.style.marginBottom=-1;
	myself.spacing+=myself.indent;

	for(var key in json)
	{
		if(json[key]!=null && typeof json[key]==='object')
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
			text.innerHTML=key+" = ";

			var printed=false;
			for(var ii in this.special_types)
			{
				if(this.special_types[ii].key==key)
				{
					printed=true;
					if(this.special_types[ii].type=="binary")
					{
						var bin_str=json[key].toString(2);
						while(bin_str.length<8)
							bin_str="0"+bin_str;
						text.innerHTML+=bin_str;
					}
					break;
				}
			}

			if(!printed)
				text.innerHTML+=json[key];

			text.style.paddingLeft=10;

			li.tree_parent=ul;

			li.appendChild(text);
			ul.appendChild(li);
		}
	}

	myself.spacing-=myself.indent;
	parent.appendChild(ul);

	return ul;
}
