function vec3(x,y,z)
{
	this.x=x;
	this.y=y;
	this.z=z;
}

vec3.prototype.normalize=function()
{
	return this.div(this.length());
}

vec3.prototype.dot=function(rhs)
{
	return this.x*rhs.x+this.y*rhs.y+this.z*rhs.z;
}

vec3.prototype.cross=function(rhs)
{
	return new vec3(this.y*rhs.z-this.z*rhs.y,
		this.z*rhs.x-this.x*rhs.z,
		this.x*rhs.y-this.y*rhs.x);
}

vec3.prototype.length=function()
{
	return Math.sqrt(this.x*this.x+this.y*this.y+this.z*this.z);
}

vec3.prototype.add=function(rhs)
{
	return new vec3(this.x+rhs.x,this.y+rhs.y,this.z+rhs.z);
}

vec3.prototype.sub=function(rhs)
{
	return new vec3(this.x-rhs.x,this.y-rhs.y,this.z-rhs.z);
}

vec3.prototype.mul=function(scaler)
{
	return new vec3(this.x*scaler,this.y*scaler,this.z*scaler);
}

vec3.prototype.div=function(scaler)
{
	return new vec3(this.x/scaler,this.y/scaler,this.z/scaler);
}