// Arsh Chauhan
// 06/17/2016
// Last Edited 06/17/2016
// sound_player.js: Interface to request sound to be played on backend
// Public Domain

function sound_player_t(div,robot)
{
	if (!div)
		return null;

	var _this=this;

	this.div=div;
	this.robot=robot;
	this.path="run";
	this.sounds=[];
	this.sound_requested="";
	this.play_sound=false;


	this.sound_selector=document.createElement("select");
	this.sound_selector.title="Pick a sound to play on the backend";
	this.sound_selector.className="form-control";
	this.default_sound=document.createElement("option");
	this.default_sound.text="Choose a sound";
	this.sound_selector.appendChild(this.default_sound);
	this.sound_selector.onchange=function()
	{
		_this.sound_requested=get_select_value(_this.sound_selector);
	};


	this.play_sound_button=document.createElement("input");
	this.play_sound_button.type="button";
	this.play_sound_button.className="btn btn-primary";
	this.play_sound_button.value="Start Sound";
	this.play_sound_button.onclick=function(){_this.request_sound();};

	this.stop_sound_button=document.createElement("input");
	this.stop_sound_button.type="button";
	this.stop_sound_button.className="btn btn-primary";
	this.stop_sound_button.value="Stop Sound";
	this.stop_sound_button.disabled=true;
	this.stop_sound_button.style.marginLeft="2px";
	this.stop_sound_button.onclick=function(){_this.stop_sound();};

	this.div.appendChild(this.sound_selector);
	this.div.appendChild(document.createElement("br"));
	this.div.appendChild(this.play_sound_button);
	this.div.appendChild(this.stop_sound_button);


}

sound_player_t.prototype.get_sounds=function(json)
{
	if(json&&json.options)
	{
		for (index in json.options)
			this.sounds[index]=json.options[index];
		this.build_sound_list();
	}
}
sound_player_t.prototype.download=function()
{
	if (this.robot.sim)
		return;
	
	var _this=this;

	superstar_get(this.robot,this.path,function(json){_this.get_sounds(json);});
}

sound_player_t.prototype.build_sound_list=function()
{
	this.sound_selector.selectedIndex=0;

	for (var ii=0; ii<this.sounds.length; ii++)
	{
		var option=document.createElement("option");
		option.text=this.sounds[ii];
		this.sound_selector.appendChild(option);
	}
}

sound_player_t.prototype.request_sound=function()
{
	if (this.robot.sim)
		return;

		
	if(this.sound_requested!=="") //Do not execute when no sound selected
	{
		this.play_sound=!this.play_sound;
		this.play_sound_button.disabled=true;
		this.stop_sound_button.disabled=false;
		superstar_set(this.robot,this.path,{"options":this.sounds,"sound":this.sound_requested,"play":this.play_sound});
	}
}

sound_player_t.prototype.stop_sound=function()
{
	if (this.robot.sim)
		return;
		
	this.play_sound=!this.play_sound;
	superstar_set(this.robot,this.path,{"options":this.sounds,"sound":this.sound_requested,"play":this.play_sound});
	this.play_sound_button.disabled=false;
	this.stop_sound_button.disabled=true;
}
