// Arsh Chauhan
// 06/15/2016
// Last Edited 09/28/2016
// sound_player.js: Play sounds on robot via chromeapp backend
// Public Domain

// DO NOT REMOVE THIS COPYRIGHT NOTICE
// This code uses these sounds from freesound:
// 	1) Bicycle horn (https://www.freesound.org/people/AntumDeluge/sounds/188039/)
// 		by AntumDeluge (https://www.freesound.org/people/AntumDeluge/)

function sound_player_t(name)
{
	if(!name)
		return null;

	var _this=this;
	this.name=name;
	this.robot=name.get_robot();
	this.last_robot=this.robot;
	this.sounds=
	[
		{name:"Squeak",audio:new Audio("sounds/DWY_SqueakHi.wav")},
		{name:"eagle",audio:new Audio("sounds/eagle.mp3")},
		{name:"bike horn",audio:new Audio("https://robotmoose.com/downloads/files/bike_horn.wav")},
	];

	this.sound_requested="";
	this.path="run";
	this.connected=false;

	this.update_interval=setInterval(function(){_this.update();},5000);
	this.play=false;

}

sound_player_t.prototype.load=function(robot)
{
	this.robot=robot;
	this.send_sounds();
}

sound_player_t.prototype.send_sounds=function()
{
	if(this.connected)
	{
		var sound_list=[];
		for(index in this.sounds)
			sound_list.push(this.sounds[index].name);

		if(this.robot&&this.robot.name&&this.robot.superstar&&this.robot.year&&this.robot.school)
			superstar_set(this.robot,this.path,{"options":sound_list,"sound":this.sound_requested,"play":this.play});
	}
}


sound_player_t.prototype.play_sound=function(sound)
{
	try
	{
		if (sound.readyState==="HAVE_ENOUGH_DATA" || sound.readyState==4)
		{
			sound.play();
		}
	}
	catch(e)
	{
		console.log("Could not play sound "+ sound + " " + e);
	}
}

sound_player_t.prototype.handle_update=function(json)
{

	if(json&&json.sound)
	{
		this.sound_requested=json.sound;
		this.play=json.play;

		if(this.play) //We received a request to play sound
		{
			for (index in this.sounds)
			{
				if(this.sounds[index].name===this.sound_requested)
					this.play_sound(this.sounds[index].audio);
			}
		}
	}
}

sound_player_t.prototype.update=function()
{
	if(this.connected)
	{
		var _this=this;
		this.last_robot=this.robot;

		if(this.name&&this.name.get_robot().name !== this.last_robot.name)
			this.load(this.name.get_robot());

		if(this.robot&&this.robot.name&&this.robot.superstar&&this.robot.year&&this.robot.school)
			superstar_get(this.robot,this.path,function(json){_this.handle_update(json);});
		}
}

sound_player_t.prototype.destroy=function()
{
	clearInterval(this.update_interval);
}

sound_player_t.prototype.disconnect=function()
{
	this.connected=false;
}

sound_player_t.prototype.connect=function()
{
	this.connected=true;
}
