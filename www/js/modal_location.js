function modal_location_t(div, robot, map_json, onreset){

	var myself = this;
	this.div = div;
	this.robot = robot;
	this.map_json = map_json;
	this.onreset=onreset;
	
	this.posX_set=0;
	this.posY_set=0;

	this.modal_location= new modal_t(div);
	this.modal_location.set_title("Reset Location");
	this.modal_location.dialog.className="modal-dialog modal-lg"
	
	//this.minimap_img.style["max-width"]=myself.modal_location.get_content().offsetWidth - 30;

	this.modal_loc_row=document.createElement("div");
	this.modal_loc_row.className="row";
	this.modal_img_col=document.createElement("div");
	this.modal_img_col.className="col-md-6";
	this.modal_ui_col=document.createElement("div");
	this.modal_ui_col.className="col-md-6";
	
	this.modal_loc_row.appendChild(myself.modal_img_col);
	this.modal_loc_row.appendChild(myself.modal_ui_col);
	this.modal_location.get_content().appendChild(myself.modal_loc_row);
	//this.modal_location.get_content().appendChild(myself.minimap_img);
	
	
	this.minimap_img = document.createElement("img");
	
	if (map_json.path)
		this.minimap_img.src=map_json.path;
	else
		this.minimap_img.src="maps/nomap.jpg";
	

	this.minimap_img.style["max-width"]="100%";
	this.minimap_img.style["max-height"]="100%";
	this.modal_img_col.appendChild(myself.minimap_img);
	
	

	
	this.minimap_roomba=document.createElement("img");
	this.minimap_roomba.src="maps/minimap_roomba.png";

	this.modal_img_col.appendChild(myself.minimap_roomba);



	this.posX = myself.minimap_img.offsetLeft;
	this.posY = myself.minimap_img.offsetTop;
	var coord_text = "X= " + myself.posX + ", Y= " + myself.posY;
	this.pos_text_el = document.createTextNode(coord_text);		
	
	this.pos_row=document.createElement("div");
	this.pos_row.className="row";
	this.pos_colX = document.createElement("div");
	this.pos_colX.className="col-md-4"
	this.pos_colY = document.createElement("div");
	this.pos_colY.className="col-md-4 col-md-offset-2"
	
	this.posX_input = document.createElement("input");
	this.posX_input.type="number";
	this.posX_input.value=0;
	this.posX_input.id="posX_input";
	this.posX_input.onchange=function(){ myself.pos_input_onchange()};
	this.posX_label=document.createElement("label");
	this.posX_label.for="posX_input";
	this.posX_label.innerHTML="X coordinate";
	
	this.posY_input=document.createElement("input");
	this.posY_input.type="number";
	this.posY_input.value=0;
	this.posY_input.id="posY_input"
	this.posY_input.onchange=function(){ myself.pos_input_onchange()};
	this.posY_label=document.createElement("label");
	this.posY_label.for="posY_input";
	this.posY_label.innerHTML="Y coordinate";
	
	
	this.set_loc_button=document.createElement("input");
	this.set_loc_button.className="btn btn-primary";
	this.set_loc_button.style.marginBottom="10px";
	this.set_loc_button.style.width = "50%";
	this.set_loc_button.disabled=false;
	this.set_loc_button.type="button";
	this.set_loc_button.value="Set Location";
	this.set_loc_button.title="Click here to set robot location";
	this.set_loc_button.addEventListener("click",function(event)
	{
		myself.set_loc_button_pressed_m();
	});
	
	this.pos_colX.appendChild(this.posX_input);
	this.pos_colX.appendChild(this.posX_label);
	this.pos_colY.appendChild(this.posY_input);
	this.pos_colY.appendChild(this.posY_label);
	this.pos_row.appendChild(this.pos_colX);
	this.pos_row.appendChild(this.pos_colY);
	this.modal_ui_col.appendChild(this.pos_text_el);
	this.modal_ui_col.appendChild(document.createElement("br"));
	this.modal_ui_col.appendChild(document.createElement("br"));
	this.modal_ui_col.appendChild(this.pos_row);
	this.modal_ui_col.appendChild(this.set_loc_button);

	
	this.modal_img_col.addEventListener("click", function(event){myself.minimap_onclick(event)});
	this.modal_img_col.addEventListener("mousemove", function(event){myself.get_coords(event)});
	
	
	window.setTimeout(function(){myself.set_minimap_roomba();}, 1);
	
}

modal_location_t.prototype.set_minimap_roomba=function(posX, posY)
{
	var myself = this;
	
	if(!myself.minimap_roomba_scale)
	{
	
	var roomba_width = 0.3;
	var px_real_img_ratio = this.minimap_img.width/this.map_json.width;
	this.minimap_roomba_scale=roomba_width*(px_real_img_ratio);
	
	if (this.minimap_roomba_scale < 30) this.minimap_roomba_scale = 30; 
	console.log("test width: " + this.minimap_roomba_scale);
	
	this.minimap_roomba.style.position="absolute";
	this.minimap_roomba.style.width=this.minimap_roomba_scale;
	this.minimap_roomba.style.height=this.minimap_roomba_scale;
	this.minimap_roomba_width=this.minimap_roomba_scale;
	this.minimap_roomba_height=this.minimap_roomba_scale;
	}
	
	var viewport = myself.minimap_img.getBoundingClientRect();
	

	
	var offset_posX = 0;
	var offset_posY = 0;
	
	
	// FIX:
	
	if(posX&&posY)
	{
		offset_posX = posX*(myself.minimap_img.width/myself.map_json.width);
		offset_posY = -posY*(myself.minimap_img.height/myself.map_json.height);
	}
	
	
	var offset_left = myself.minimap_img.width/2- myself.minimap_roomba_width/2 + 15 + offset_posX;
	var offset_top = myself.minimap_img.height/2 - myself.minimap_roomba_height/2 + offset_posY;
	
	myself.minimap_roomba.style.left=offset_left;
	myself.minimap_roomba.style.top=offset_top;
	
	console.log("left: " + offset_left + " top: " + offset_top)

}

modal_location_t.prototype.pos_input_onchange=function()
{

	
	var myself = this;
	var opt = myself.map_json;
	
	// check range
	var posX_temp = this.posX_input.value;
	var posY_temp = this.posY_input.value;
	
	if (opt.width/2 < posX_temp)
	{
		this.posX_input.value=opt.width/2;
	}
	else if (-opt.width/2 > posX_temp)
	{
		this.posX_input.value = -opt.width/2;
	}
	
	if (opt.height/2 < posY_temp)
	{
		this.posY_input.value=opt.height/2;
	}
	else if (-opt.height/2 > posY_temp)
	{
		this.posY_input.value=-opt.height/2;
	}
	
	// set roomba position
	this.posX_set=this.posX_input.value;
	this.posY_set=this.posY_input.value;
	
	// set minimap roomba image
	this.set_minimap_roomba(myself.posX_set, myself.posY_set);
}

modal_location_t.prototype.reset_location=function(x_cor, y_cor, angle)
{
	if (!x_cor) x_cor = 0;
	if (!y_cor) y_cor = 0;
	if (!angle) angle = 0;
	
	if (this.robot&&this.robot.sim) this.robot.change_location(x_cor, y_cor, 0, angle);
	else
	{
		robot_network.sensors.location.x = x_cor;
		robot_network.sensors.location.y = y_cor;
		robot_network.sensors.angle = angle;
	}
}

modal_location_t.prototype.set_loc_button_pressed_m=function()
{
	var myself = this;
	var new_x = myself.posX_set;
	var new_y = myself.posY_set;
	var new_angle = 0;
	console.log("Resetting location - x: " + new_x + ", y: " + new_y);
	myself.reset_location(new_x, new_y, new_angle);
	if(myself.onreset) myself.onreset();

}

modal_location_t.prototype.get_coords=function(event)
{
	var myself = this;
	var opt = myself.map_json;
	var ratio_width = opt.width/myself.minimap_img.width;
	var ratio_height = opt.height/myself.minimap_img.height;
	
	var viewport = myself.minimap_img.getBoundingClientRect();
	var posX = ratio_width*(event.pageX - viewport.left - myself.minimap_img.width/2);
	var posY = -ratio_height*(event.pageY - viewport.top - myself.minimap_img.height/2);
	
	if (opt.width/2 < Math.abs(posX))
	{
		return "";
	}
	else
	{
		myself.posX = posX.toFixed(2);
		myself.posY = posY.toFixed(2);
		//myself.pos_text_el.nodeValue="X= " + (myself.posX).toFixed(2) + ", Y= " + (myself.posY).toFixed(2);
		myself.pos_text_el.nodeValue="X= " + myself.posX + ", Y= " + myself.posY;
		return true;
	}
}

modal_location_t.prototype.minimap_onclick=function(event)
{

	var myself = this;
		
	if (myself.get_coords(event))
	{
	myself.posX_input.value=myself.posX; //(myself.posX).toFixed(2);
	myself.posY_input.value=myself.posY; //(myself.posY).toFixed(2);
	myself.posX_set=myself.posX;
	myself.posY_set=myself.posY;
	

	myself.set_minimap_roomba(myself.posX, myself.posY);
	}
	
}

modal_location_t.prototype.show=function()
{
	this.modal_location.show();
}

modal_location_t.prototype.hide=function()
{
	this.modal_location.hide();
}


