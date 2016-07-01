function parse_uri()
{
	var queries={};
	var query=window.location.search.substring(1);
	var vars=query.split("&");

	for(var ii=0;ii<vars.length;++ii)
	{
		var pair=vars[ii].split("=");

		if(typeof(queries[pair[0]])==="undefined")
		{
			queries[pair[0]]=decodeURIComponent(pair[1]);
		}
		else if(typeof(queries[pair[0]])==="string")
		{
			var arr=[queries[pair[0]],decodeURIComponent(pair[1])];
			queries[pair[0]]=arr;
		}
		else
		{
			queries[pair[0]].push(decodeURIComponent(pair[1]));
		}
	}

	return queries;
}