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