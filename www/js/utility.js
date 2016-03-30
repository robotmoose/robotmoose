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

	for(var key in arr)
		u_arr[arr[key]]=true;

	arr.length=0;

	for(var key in u_arr)
		arr.push(key);

	return arr;
}

function get_time()
{
	return (new Date).getTime();
}