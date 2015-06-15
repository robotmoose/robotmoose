function load_js(js)
{
	var scr=document.createElement("script");
	scr.src=js;
	document.head.appendChild(scr);
};

function load_dependencies()
{
	load_js("/js/xmlhttp.js");
};

(function(){load_dependencies()})();

function config_editor_t(div)
{
	if(!div)
		return null;

	this.div=div;
	this.div.innerHTML="";

	this.title=document.createElement("text");
	this.title.innerHTML="Firmware Configuration";
	this.div.appendChild(this.title);

	this.break0=document.createElement("br");
	this.div.appendChild(this.break0);

	this.textarea=document.createElement("textarea");
	this.div.appendChild(this.textarea);
	console.log(this.textarea);

	this.break1=document.createElement("br");
	this.div.appendChild(this.break1);

	this.button=document.createElement("input");
	this.button.type="button";
	this.button.value="Configure";

	var myself=this;
	this.button.onclick=function(){myself.configure();};

	this.div.appendChild(this.button);
}

config_editor_t.prototype.configure=function()
{
	console.log("Configure!");
	var config_text=this.textarea.value;

	try
	{
		var configs=this.lex(config_text);
		this.validate(configs);
		console.log(JSON.stringify(configs));
		//send_request("POST","path","request","uri",this.onreply,this.onerror,data,content_type);
	}
	catch(e)
	{
		console.log("Error! - "+e);
	}
}

function parse_identifier(str)
{
	var identifier="";

	if(str.length>0&&!((str[0]>='a'&&str[0]<='z')||str[0]=='_'))
		return "";

	for(var ii=0;ii<str.length;++ii)
	{
		if((str[ii]>='a'&&str[ii]<='z')||(str[ii]>='0'&&str[ii]<='9')||str[ii]=='_')
			identifier+=str[ii];
		else
			break;
	}

	return identifier;
}

function parse_argument(str)
{
	var argument="";

	for(var ii=0;ii<str.length;++ii)
	{
		if((str[ii]>='a'&&str[ii]<='z')||(str[ii]>='0'&&str[ii]<='9'))
			argument+=str[ii];
		else
			break;
	}

	return argument;
}

function parse_symbol(str,symbol)
{
	return (str.length>0&&str[0]==symbol);
}

function skip_whitespace(str,col,line)
{
	while(str.length>0&&(str[0]==' '||str[0]=='\t'||str[0]=='\n'||str[0]=='\r'))
	{
		if(str[0]=='\n'||str[0]=='\r')
		{
			col=0;
			++line;
		}
		else
		{
			++col;
		}

		str=str.substring(1,str.length);
	}

	return {str:str,col:col,line:line};
}

config_editor_t.prototype.lex=function(config)
{
	var col=0;
	var line=0;
	var temp;
	var copy=config.toLowerCase();
	var configs=new Array();

	while(copy.length>0)
	{
		temp=skip_whitespace(copy,col,line);
		copy=temp.str;
		col=temp.col;
		line=temp.line;

		var name=parse_identifier(copy);
		var args=new Array();

		if(name.length==0)
			throw "Line: "+line+" Col: "+col+" - Invalid device name!";

		copy=copy.substring(name.length,copy.length);
		col+=name.length;

		temp=skip_whitespace(copy,col,line);
		copy=temp.str;
		col=temp.col;
		line=temp.line;

		if(!parse_symbol(copy,'('))
			throw "Line: "+line+" Col: "+col+" - Expected '('!";

		copy=copy.substring(1,copy.length);
		++col;

		var arg="";

		while(true)
		{
			temp=skip_whitespace(copy,col,line);
			copy=temp.str;
			col=temp.col;
			line=temp.line;

			arg=parse_argument(copy);
			args.push(arg);
			copy=copy.substring(arg.length,copy.length);
			col+=arg.length;

			temp=skip_whitespace(copy,col,line);
			copy=temp.str;
			col=temp.col;
			line=temp.line;

			if(parse_symbol(copy,')'))
				break;

			temp=skip_whitespace(copy,col,line);
			copy=temp.str;
			col=temp.col;
			line=temp.line;

			if(arg.length=="")
			{
				if(parse_identifier(copy).length>0)
					throw "Line: "+line+" Col: "+col+" - Invalid argument!";

				throw "Line: "+line+" Col: "+col+" - Expected argument!";
			}

			if(!parse_symbol(copy,','))
				throw "Line: "+line+" Col: "+col+" - Expected ',' or ')'!";

			copy=copy.substring(1,copy.length);
			++col;
		}

		if(!parse_symbol(copy,')'))
			throw "Line: "+line+" Col: "+col+" - Expected ')'!";

		copy=copy.substring(1,copy.length);
		++col;

		temp=skip_whitespace(copy,col,line);
		copy=temp.str;
		col=temp.col;
		line=temp.line;

		if(!parse_symbol(copy,';'))
			throw "Line: "+line+" Col: "+col+" - Expected ';'!";

		copy=copy.substring(1,copy.length);
		++col;

		temp=skip_whitespace(copy,col,line);
		copy=temp.str;
		col=temp.col;
		line=temp.line;

		configs.push({"name":name,"args":args});
	}

	return configs;
}

config_editor_t.prototype.validate=function(config)
{
	//actually check args...
}

config_editor_t.prototype.onreply=function(response)
{
	console.log("Success! - "+response);
}

config_editor_t.prototype.onerror=function(error)
{
	console.log("Failure! - "+error);
}


