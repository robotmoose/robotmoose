function isalpha(c)
{
	return (((c>='a')&&(c<='z'))||((c>='A')&&(c<='Z')));
}

function isdigit(c)
{
	return ((c>='0')&&(c<='9'));
}

function isalnum(c)
{
	return (isalpha(c)||isdigit(c));
}

function isident(value)
{
	if(value.length<=0)
		return false;

	if(!isalpha(value[0])&&value[0]!='_')
		return false;

	for(var ii=1;ii<value.length;++ii)
		if(!isalnum(value[ii])&&value[ii]!='_')
			return false;

	return true;
}