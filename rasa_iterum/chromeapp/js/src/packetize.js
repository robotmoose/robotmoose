function ab2str(buffer)
{
	return String.fromCharCode.apply(null,new Uint8Array(buffer));
}

function str2ab(str)
{
	var buffer=new ArrayBuffer(str.length);
	var buffer_viewer=new Uint8Array(buffer);

	for(var ii=0;ii<str.length;++ii)
		buffer_viewer[ii]=str.charCodeAt(ii);

	return buffer;
}

function build_packet(str)
{
	var buffer=new ArrayBuffer(1+2+str.length+1);
	var buffer_viewer=new Uint8Array(buffer);

	buffer_viewer[0]=0x5f;
	buffer_viewer[1]=str.length&0x00ff;
	buffer_viewer[2]=(str.length&0xff00)>>8;
	buffer_viewer[3+str.length]=0x00;

	for (var ii=0;ii<str.length;++ii)
	{
		buffer_viewer[3+ii]=str.charCodeAt(ii);
		buffer_viewer[3+str.length]^=buffer_viewer[3+ii];
	}

	return buffer;
}

//on_receive(str) - str of received packet payload
//state {header=0,size_0,size_1,data,crc}
function parser_t(on_receive)
{
	this.reset();
	this.on_receive=on_receive;
}

parser_t.prototype.parse=function(buffer)
{
	var buffer_viewer=new Uint8Array(buffer);

	for(var ii=0;ii<buffer_viewer.length;++ii)
	{
		if(this.state==0&&buffer_viewer[ii]==0x5f)
		{
			this.state=1;
		}
		else if(this.state==1)
		{
			this.len+=buffer_viewer[ii];
			this.state=2;
		}
		else if(this.state==2)
		{
			this.len+=(buffer_viewer[ii]<<8);
			this.state=3;
		}
		else if(this.state==3)
		{
			this.str+=String.fromCharCode(buffer_viewer[ii]);

			if(this.str.length>=this.len)
				this.state=4;
		}
		else if(this.state==4)
		{
			crc=0x00;

			for(var jj=0;jj<this.str.length;++jj)
				crc^=this.str.charCodeAt(jj);

			if(crc==buffer_viewer[ii]&&this.on_receive)
				this.on_receive(this.str);

			this.state=0;
			this.str="";
			this.len=0;
		}
	}
}

parser_t.prototype.reset=function()
{
	this.state=0;
	this.str="";
	this.len=0;
}
