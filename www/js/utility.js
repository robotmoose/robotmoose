//find utility for array

function array_find(array, find_me)
{
	for(ii=0; ii<array.length; ii++)
	{
		if(array[ii]==find_me)
		{
			return ii;
		}
	}
	return -1;
}

function remove_duplicates(arr)
{
	u_arr=[];

	for(let key in arr)
		u_arr[arr[key]]=true;

	arr.length=0;

	for(let key in u_arr)
		arr.push(key);

	return arr;
}

function get_time()
{
	return (new Date).getTime();
}

function get_select_value(select)
{
	if(!select||!select.options||select.options.length<=0)
		return "";
	if(select.selectedIndex>select.options.length)
		select.selectedIndex=0;
	if(!select.options[select.selectedIndex])
		return "";
	return select.options[select.selectedIndex].text;
}

function valid_robot(robot)
{
	return (robot&&robot.year&&robot.school&&robot.name);
}

//Returns the global offset for an element via iterative looping of parents...
//  http://stackoverflow.com/questions/442404/retrieve-the-position-x-y-of-an-html-element
function get_absolute_offset(el)
{
	var offset=
	{
		x:0,
		y:0
	};
	while(el)
	{
		offset.x+=parseInt(el.offsetLeft)-parseInt(el.scrollLeft);
		offset.y+=parseInt(el.offsetTop)-parseInt(el.scrollTop);
		el=el.offsetParent;
	}
	return offset;
};

function is_in_select(select,value)
{
	for(var ii in select.options)
		if(select.options[ii].value)
			return true;
	return false;
}