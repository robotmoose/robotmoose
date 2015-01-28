//Tab Key Data Selection Function
//http://www.webdeveloper.com/forum/showthread.php?t=32317
function setSelectionRange(input,selectionStart,selectionEnd)
{
	if(input.setSelectionRange)
	{
		input.focus();
		input.setSelectionRange(selectionStart,selectionEnd);
	}
	else if(input.createTextRange)
	{
		var range=input.createTextRange();
		range.collapse(true);
		range.moveEnd('character',selectionEnd);
		range.moveStart('character',selectionStart);
		range.select();
	}
}

//Tab Key Data Replace Function
//http://www.webdeveloper.com/forum/showthread.php?t=32317
function replaceSelection(input,replaceString)
{
	if(input.setSelectionRange)
	{
		var selectionStart=input.selectionStart;
		var selectionEnd=input.selectionEnd;
		input.value=input.value.substring(0,selectionStart)+replaceString+input.value.substring(selectionEnd);

		if(selectionStart!=selectionEnd)
		{
			setSelectionRange(input,selectionStart,selectionStart+replaceString.length);
		}
		else
		{
			setSelectionRange(input,selectionStart+replaceString.length,selectionStart+replaceString.length);
		}
	}
	else if(document.selection)
	{
		var range=document.selection.createRange();

		if(range.parentElement()==input)
		{
			var isCollapsed=(range.text=='');
			range.text=replaceString;

			if(!isCollapsed)
			{
				range.moveStart('character',-replaceString.length);
				range.select();
			}
		}
	}
}

//Tab Key Catcher Function
//http://www.webdeveloper.com/forum/showthread.php?t=32317
function catchTab(item,e)
{
	if(navigator.userAgent.match("Gecko"))
	{
		c=e.which;
	}
	else
	{
		c=e.keyCode;
	}

	if(c==9)
	{
		replaceSelection(item,String.fromCharCode(9));
		setTimeout("document.getElementById('"+item.id+"').focus();",0);
		return false;
	}
}