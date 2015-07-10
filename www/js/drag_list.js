function drag_list_t(div)
{
	if(!div)
		return null;

	this.div=div;
	this.element=document.createElement("div");
	this.ul=document.createElement("ul");
	this.entries=[];

	this.div.appendChild(this.element);

	this.ul.className="sortable_handle";
	this.element.appendChild(this.ul);

	this.refresh_entries_m();
}

drag_list_t.prototype.get_entries=function()
{
	this.entries.sort(function(lhs,rhs)
	{
		if(lhs&&rhs)
			return lhs.li.offsetTop-rhs.li.offsetTop;
	});

	return this.entries;
}

drag_list_t.prototype.create_entry=function()
{
	var myself=this;
	var entry={};
	entry.li=document.createElement("li");
	entry.table={};
	entry.table.table=document.createElement("table");
	entry.table.row=entry.table.table.insertRow(0);
	entry.table.left=entry.table.row.insertCell(0);
	entry.table.center=entry.table.row.insertCell(1);
	entry.table.right=entry.table.row.insertCell(2);
	entry.move_handle=document.createElement("span");
	entry.content=document.createElement("span");
	entry.close_button=document.createElement("span");

	entry.li.className="list-group-item";

	entry.table.row.style.verticalAlign="top";
	entry.table.center.style.width="100%";
	entry.li.appendChild(entry.table.table);

	entry.move_handle.className="glyphicon glyphicon-move";
	entry.move_handle.style.marginRight=10;
	entry.move_handle.style.cursor="move";
	entry.table.left.appendChild(entry.move_handle);

	entry.table.center.appendChild(entry.content);

	entry.close_button.className="close";
	entry.close_button.style.marginLeft=10;
	entry.close_button.innerHTML="&times;";
	entry.close_button.drag_list_t=entry;
	entry.close_button.onclick=function(event){myself.remove_entry(this.drag_list_t);};

	entry.table.right.appendChild(entry.close_button);

	this.ul.appendChild(entry.li);
	this.entries.push(entry);
	this.refresh_entries_m();

	return entry;
}

drag_list_t.prototype.remove_entry=function(entry)
{
	if(entry)
	{
		if(entry.onremove)
			entry.onremove(entry);

		for(var key in this.entries)
		{
			if(this.entries[key]&&this.entries[key]===entry)
			{
				this.ul.removeChild(this.entries[key].li);
				this.entries[key]=null;
				break;
			}
		}
	}
}

drag_list_t.prototype.refresh_entries_m=function()
{
	$("ul.sortable_handle").sortable({handle:'.glyphicon'});
}