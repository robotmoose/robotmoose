//Checkmark.js
//	Description:
//		Creates a simply interface that creates two columns.
//		The first column (returned by .getElement()) you can put whatever you want.
//		The second column is a checkbox signifying good or bad (set with .check(true/false)).
//	Created By: Mike Moss

function checkmark_t(el)
{
	if(!el)
		return null;
	this.el=el;
	this.size="24px";
	this.table=new_table(this.el,1,2);
	clear_margin_padding_table(this.table);
	this.table.rows[0].cells[0].style.width="100%";
	this.table.rows[0].cells[1].style.paddingLeft=this.table.rows[0].cells[1].style.paddingRight="5px";
	this.el.appendChild(this.table);
	this.check_img=this.make_img("/images/check.png");
	this.cross_img=this.make_img("/images/exclamation.png");
	this.show_m(this.cross_img);
}

checkmark_t.prototype.appendChild=function(el)
{
	this.table.rows[0].cells[0].appendChild(el);
}

checkmark_t.prototype.removeChild=function(el)
{
	this.table.rows[0].cells[0].removeChild(el);
}

checkmark_t.prototype.getElement=function()
{
	return this.table.rows[0].cells[0];
}

checkmark_t.prototype.check=function(value)
{
	if(value)
	{
		this.show_m(this.check_img);
		this.hide_m(this.cross_img);
	}
	else
	{
		this.hide_m(this.check_img);
		this.show_m(this.cross_img);
	}
}


checkmark_t.prototype.make_img=function(src)
{
	var img=document.createElement("img");
	img.src=src;
	clear_margin_padding(img);
	this.hide_m(img);
	this.table.rows[0].cells[1].appendChild(img);
	return img;
}

checkmark_t.prototype.show_m=function(el)
{
	el.style.visibility="visible";
	el.style.width=el.style.height=this.size;
}

checkmark_t.prototype.hide_m=function(el)
{
	el.style.visibility="hidden";
	el.style.width=el.style.height="0px";
}