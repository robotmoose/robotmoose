function new_div(el,id)
{
	var div=document.createElement("div");
	if(el) div.id=id;
	el.appendChild(div);
	clear_margin_padding(div);
	return div;
}

function new_table(el,rows,cells_per_row)
{
	var table=document.createElement("table");
	el.appendChild(table);
	table.style.borderSpacing="0px";

	for(let rr=0;rr<rows;++rr)
	{
		var row=document.createElement("tr");
		clear_margin_padding(row);
		table.appendChild(row);

		for(let cc=0;cc<cells_per_row;++cc)
		{
			var cell=document.createElement("td");
			clear_margin_padding(cell);
			row.appendChild(cell);
		}
	}

	clear_margin_padding(table);
	return table;
}

function new_textarea(el)
{
	var textarea=document.createElement("textarea");
	el.appendChild(textarea);
	textarea.wrap="off";
	textarea.readOnly=true;
	textarea.style.resize="none";
	textarea.style.boxSizing="border-box";
	textarea.style["-webkit-box-sizing"]="border-box";
	textarea.style["-moz-box-sizing"]="border-box";

	clear_margin_padding(textarea);
	maximize(textarea);

	return textarea;
}

function new_status_div(el, id, status_height)
{
	var div=new_div(el, id);
	div.style.width="100%";
	div.style.height=status_height;
	div.style.boxSizing="border-box";
	div.style["-webkit-box-sizing"]="border-box";
	div.style["-moz-box-sizing"]="border-box";
	div.style.border="1px solid black";
}

function clear_margin_padding(el)
{
	el.style.margin="0px";
	el.style.padding="0px";
}

function clear_margin_padding_table(table)
{
	clear_margin_padding(table);

	for(let rr=0;rr<table.rows.length;++rr)
	{
		clear_margin_padding(table.rows[rr]);

		for(let cc=0;cc<table.rows[rr].cells.length;++cc)
			clear_margin_padding(table.rows[rr].cells[cc]);
	}
}

function maximize(el)
{
	el.style.width="100%";
	el.style.height="100%";
}
