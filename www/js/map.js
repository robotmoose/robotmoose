/**
  Draw an onscreen map of the robot's position, orientation, and sensor data.

  Dr. Orion Lawlor, lawlor@alaska.edu, 2015-07-23 (Public Domain)
*/

function robot_map_t(div)
{
	this.div=div;
	this.div.title="Shows where the robot thinks it is in the world.  The grid lines are 1 meter apart.  The robot's right and left wheels leave red and purple tracks";
	var myself=this;

	this.element=document.createElement("div");
	this.div.appendChild(this.element);
	this.element.title="Select map image overlay";

	this.controls_div=document.createElement("div");
	this.element.appendChild(this.controls_div);
	this.controls_div.className="form-horizontal";

	//Map dropdown...
	this.map_select=document.createElement("select");
	this.controls_div.appendChild(this.map_select);
	this.change_map_image=function()
	{
		myself.last_map_select=myself.map_select.value;
		myself.load_button_pressed_m();
	};
	this.map_select.addEventListener("change",this.change_map_image);
	this.map_select.className="form-control";
	this.map_select.style.marginBottom="10px";

	// ********** Add maps here:
		var option=document.createElement("option");
		option.text="No Map Image";
		var opt0 = {};
		opt0.path="";
		opt0.width=10;
		opt0.height=10;
		option.value=JSON.stringify(opt0);
		//option.value="";
		this.map_select.appendChild(option);

		var option1=document.createElement("option");
		option1.text="Map 1";
		var opt1 = {};
		opt1.path="maps/map1.jpg";
		opt1.width=3;
		opt1.height=10;
		option1.value=JSON.stringify(opt1);
		//option1.value="maps/map1.jpg";
		this.map_select.appendChild(option1);

		var option2=document.createElement("option");
		option2.text="Map 2";
		var opt2 = {};
		opt2.path="maps/map2.jpg";
		opt2.width=10;
		opt2.height=10;
		option2.value=JSON.stringify(opt2);
		//option2.value="maps/map2.jpg"
		this.map_select.appendChild(option2);

		var option3=document.createElement("option");
		option3.text="Map 3";
		var opt3 = {};
		opt3.path="maps/map3.jpg";
		opt3.width=10;
		opt3.height=10;
		option3.value=JSON.stringify(opt3);
		//option3.value="maps/map3.jpg";
		this.map_select.appendChild(option3);

	//Upload button...
	this.upload_map_button=document.createElement("input");
	this.controls_div.appendChild(this.upload_map_button);
	this.upload_map_button.className="btn btn-primary form-control";
	this.upload_map_button.style.marginBottom="10px";
	this.upload_map_button.disabled=false;
	this.upload_map_button.type="button";
	this.upload_map_button.value="Upload Map Image";
	this.upload_map_button.title="Click here to upload a map image";
	this.upload_map_button.addEventListener("click",function(event)
	{
		myself.upload_map_button_pressed_m();
	});




	this.map_display = document.createElement("div");
	this.map_display.title="Shows where the robot thinks it is in the world.  The grid lines are 1 meter apart.  The robot's right and left wheels leave red and purple tracks";

	this.need_redraw=true;
	this.renderer=new renderer_t(myself.map_display,function() {myself.setup();}, function() {return myself.loop();} );
	if(!this.renderer.setup()) {
		var p=document.createElement("p");
		p.innerHTML="<p>WebGl seems to be disabled: <a target=_blank href=https://get.webgl.org>Click here to test</a><br> <u>If disabled, Try the following steps:</u></p> "
		p.innerHTML+="<p><b>Firefox:</b> Go to about:config in your address bar,search for webgl and check if webgl.disabled is true <br> No Luck? <a target=_blank href=https://support.mozilla.com/en-US/kb/how-do-i-upgrade-my-graphics-drivers>Help Page</a></p>";
		p.innerHTML+="<p><b>Chrome:</b> <ol><li><b>Is hardware acceleration enabled?</b> Type chrome://settings, show advanced settings -> under system > check Use hardware acceleration when available</li><b><li>Check WebGl:</b> Type chrome://flags into the address bar and confirm that Disable WebGl is gray</li></ol></p>";
		p.innerHTML+= "<p><b>Safari:</b> <ol><li>Go to Safari's Preferences</li><li>Select the Advanced tab</li><li>Check the Show Develop menu in menu bar checkbox</li><li>In the Develop menu, check Enable WebGl</li><li>Confused?<a target=_blank href=http://voicesofaliveness.net/webgl>Pictures</a></li></ol></p>";
		div.appendChild(p);
		this.renderer=null;
	}

	this.div.appendChild(myself.map_display);

}

robot_map_t.prototype.setup=function(texture_file, width, height) {
	if (this.renderer===null) return;

	var myself=this;

	this.resize_map();

	// Add grid
	var grid_cells=10;
	if (!width) width = 10;
	if (!height) height = 10;
	var per_cell=1000; // one meter cells (in mm)
	this.grid=this.renderer.create_grid(per_cell,width,height,20,texture_file);
	this.grid.rotation.x=0;


	// Add light source
	var size=100000;
	var intensity=0.8;
	this.light=this.renderer.create_light(intensity,
		new THREE.Vector3(-size/2,-size/2,+size));

	// FIXME: add 2D room overlay

	// Add a robot
	this.mapRobot=new roomba_t(this.renderer,null);

	// Set initial camera
	this.renderer.controls.center.set(0,0,0); // robot?
	this.renderer.controls.object.position.set(0,-1200,1400);

}

robot_map_t.prototype.resize_map=function()
{
	var edge_offset = 20;
	var rend_width = this.element.offsetWidth;
	var rend_height = this.div.offsetHeight-this.element.offsetHeight;
	this.renderer.set_size(rend_width,rend_height);

}


// Updated sensor data is available:
robot_map_t.prototype.refresh=function(sensors) {
	if (this.renderer===null) return;
	this.sensors=sensors;
	this.need_redraw=true;
}

robot_map_t.prototype.loop=function() {
	if (this.renderer===null) return;

	var sensors=this.sensors;
	if (!sensors || !sensors.location) return;

	// Convert angle from degrees to radians
	var angle_rad=sensors.location.angle*Math.PI/180.0;
	// Convert position from meters to mm (rendering units)
	var P=new vec3(sensors.location.x,sensors.location.y,0.0).te(1000.0);

	// Move onscreen robot there
	this.mapRobot.set_location(P,angle_rad);

	// Place the wheels (so wheel tracks work)
	this.mapRobot.wheel[0]=this.mapRobot.world_from_robot(150,+Math.PI*0.5);
	this.mapRobot.wheel[1]=this.mapRobot.world_from_robot(150,-Math.PI*0.5);

	// Check for obstacle sensors
	if (sensors.lidar) {
		if (sensors.lidar.change!=this.last_lidar_change) {
			this.mapRobot.draw_lidar(this.renderer,sensors.lidar);
			this.last_lidar_change=sensors.lidar.change;
		}
	}

	if (this.reset_tracks)
	{
		this.mapRobot.left_tracker.reset();
		this.mapRobot.right_tracker.reset();
		this.reset_tracks = false;
	}


	var need_redraw=this.need_redraw;
	this.need_redraw=false;
	return need_redraw;
}

robot_map_t.prototype.load_button_pressed_m=function()
{
	var myself=this;

	if (this.mapRobot)
	{
		this.mapRobot.model.destroy();
		this.mapRobot=null;
		this.reset_tracks = true;
	}


	var opt = JSON.parse(myself.last_map_select);

	this.make_new(opt.path, opt.width, opt.height);

}

robot_map_t.prototype.upload_map_button_pressed_m=function()
{
	var myself = this;

	// new modal popup
	this.modal=new modal_t(div);
	this.modal.set_title("Upload Map Image");


	// upload button
	this.file_input=document.createElement("input");
	this.file_input.type='file';
	this.file_input.addEventListener("change",function(event)
	{
		if (myself.file_input.files[0].type.split('/')[0]=='image')
		{

				if (myself.upload_error_div&&myself.modal.get_content().contains(myself.upload_error_div))
					myself.modal.get_content().removeChild(myself.upload_error_div); // remove error message

				var reader = new FileReader();
				reader.onload=function()
				{
					// create image object and set source to uploaded image
					myself.uploaded_map=document.createElement("img");
					myself.uploaded_map.src=reader.result;
					myself.uploaded_map.style["max-width"]=myself.modal.get_content().offsetWidth - 30;


					// preview image:
					if (myself.upload_preview_div&&myself.modal.get_content().contains(myself.upload_preview_div))
					{
						myself.modal.get_content().removeChild(myself.upload_preview_div); // remove old preview
					}


					var dim_text = "Image Dimensions: "  + myself.uploaded_map.width + ":" + myself.uploaded_map.height;
					myself.dim_text_node = document.createTextNode(dim_text);

					myself.upload_preview_div=document.createElement("div");


					myself.upload_preview_div.appendChild(document.createElement("br"));
					myself.upload_preview_div.appendChild(myself.uploaded_map);
					myself.upload_preview_div.appendChild(document.createElement("br"));
					myself.upload_preview_div.appendChild(myself.dim_text_node);


					myself.modal.get_content().appendChild(myself.upload_opts_div);
					myself.modal.get_content().appendChild(myself.upload_preview_div);

				}
				reader.readAsDataURL(myself.file_input.files[0]);

		}
		else
		{
			myself.file_input_form.reset();
			myself.display_upload_message("Error: selected file is not an image. Please select an image and try again.");
		}
	});

	this.display_upload_message=function(msg) // display error message to modal
	{
			if (myself.upload_error_div&&myself.modal.get_content().contains(myself.upload_error_div))
				myself.modal.get_content().removeChild(myself.upload_error_div); // remove error message

			if (myself.upload_opts_div&&myself.modal.get_content().contains(myself.upload_opts_div))
				myself.modal.get_content().removeChild(myself.upload_opts_div); // remove height, width, and confirm button

			if (myself.upload_preview_div&&myself.modal.get_content().contains(myself.upload_preview_div))
				myself.modal.get_content().removeChild(myself.upload_preview_div); // remove preview image

			myself.upload_error_div=document.createElement("div")
			myself.upload_error=document.createTextNode(msg);

			myself.upload_error_div.appendChild(document.createElement("br"));
			myself.upload_error_div.appendChild(myself.upload_error);

			myself.modal.get_content().appendChild(myself.upload_error_div);
	}

	this.file_input_form=document.createElement("form");
	this.file_input_form.appendChild(this.file_input);



	// create objects: title input, width input, height input, and confirm upload button

	this.upload_opts_div=document.createElement("div");

	// text prompt
	var scale_prompt="A preview of your map image is shown below. Choose a width and height between 1 and 100 meters and click \"Load Map Image\" to confirm."
	this.upload_text=document.createTextNode(scale_prompt)

	// map title (optional)
	this.upload_title=document.createElement("input");
	this.upload_title.className="form-control";
	this.upload_title.width="100%";
	this.upload_title.placeholder="Map Title (optional)";

	// map scale
	this.scale_div=document.createElement("div"); // create row element to store height and width input boxes
	this.scale_div.className="row";
	this.scale_column_w=document.createElement("div"); // column element to store width
	this.scale_column_w.className="col-xs-4";
	this.scale_column_h=document.createElement("div"); // column element to store height
	this.scale_column_h.className="col-xs-4";
	this.scale_column_b=document.createElement("div"); // column element to store button
	this.scale_column_b.className="col-xs-4";

	this.upload_width=document.createElement("input"); // width input
	this.upload_width.className="form-control";
	this.upload_width.placeholder="width (in meters)";
	this.upload_width.addEventListener("change",function(event)
	{
		myself.scale_onchange_m();
	});

	this.upload_height=document.createElement("input"); // height input
	this.upload_height.className="form-control";
	this.upload_height.placeholder="height (in meters)";
	this.upload_height.addEventListener("change",function(event)
	{
		myself.scale_onchange_m();
	});


	// confirm upload button
	this.confirm_upload_button=document.createElement("input");
	this.confirm_upload_button.className="btn btn-primary";
	this.confirm_upload_button.disabled=true;
	this.confirm_upload_button.type="button";
	this.confirm_upload_button.value="Load Map Image";
	this.confirm_upload_button.title="Click Here to Load the Map Image";
	this.confirm_upload_button.style.float="right";
	this.confirm_upload_button.addEventListener("click",function(event)
	{
		myself.confirm_upload_button_pressed_m();
	});


	// add text prompt to div
	this.upload_opts_div.appendChild(document.createElement("br"));
	this.upload_opts_div.appendChild(this.upload_text);
	this.upload_opts_div.appendChild(document.createElement("br"));
	this.upload_opts_div.appendChild(document.createElement("br"));

	//add title to div
	this.upload_opts_div.appendChild(this.upload_title);
	this.upload_opts_div.appendChild(document.createElement("br"));


	// add objects to columns
	this.scale_column_w.appendChild(this.upload_width);
	this.scale_column_h.appendChild(this.upload_height);
	this.scale_column_b.appendChild(this.confirm_upload_button);

	// add columns to row
	this.scale_div.appendChild(this.scale_column_w);
	this.scale_div.appendChild(this.scale_column_h);
	this.scale_div.appendChild(this.scale_column_b);

	// add row to div
	this.upload_opts_div.appendChild(this.scale_div);


	// append divs to modal
	this.modal.get_content().appendChild(document.createElement("br"));
	this.modal.get_content().appendChild(this.file_input_form);
	this.modal.show();

}

robot_map_t.prototype.scale_onchange_m=function() // when height or width are changed
{
	var myself = this;
	if (myself.upload_width && myself.upload_height)
	{
		console.log("scales exist!")
		if (myself.upload_width.value >= 1 && myself.upload_width.value <= 100 && myself.upload_height.value >= 1 && myself.upload_height.value <= 100)
			myself.confirm_upload_button.disabled=false;
		else
			myself.confirm_upload_button.disabled=true;
	}
}


robot_map_t.prototype.confirm_upload_button_pressed_m=function()
{
	var myself = this;
	if (myself.uploaded_map)
	{
		// add image to map options

		myself.uploaded_option=document.createElement("option");
		if (myself.upload_title.value) myself.uploaded_option.text=myself.upload_title.value;
		else myself.uploaded_option.text="Uploaded Map";
		var opt = {};
		opt.path=myself.uploaded_map.src; // image path
		opt.width=myself.upload_width.value; // width
		opt.height=myself.upload_height.value; // height
		myself.uploaded_option.value=JSON.stringify(opt);
		myself.map_select.appendChild(myself.uploaded_option);
		myself.modal.hide();

		myself.map_select.selectedIndex=myself.map_select.length - 1;
		myself.change_map_image();
	}

}

robot_map_t.prototype.clean_up=function()
{
	var myself = this;

	if (this.renderer) this.renderer.destroy();
	this.renderer=null;

	if (this.grid)
	{
		this.grid.geometry.dispose();
		this.grid.material.dispose();
		if (this.grid.texture) this.grid.texture.dispose();
		this.grid = null;
	}

	if (this.map_display)
	{
	this.div.removeChild(myself.map_display);
	this.map_display="";
	}
}

robot_map_t.prototype.make_new=function(filename, width, height)
{
	var myself = this;

	this.clean_up();

	this.map_display = document.createElement("div");
	this.map_display.title="Shows where the robot thinks it is in the world.  The grid lines are 1 meter apart.  The robot's right and left wheels leave red and purple tracks";

	this.need_redraw=true;
	this.renderer=new renderer_t(myself.map_display,function() {myself.setup(filename, width, height);}, function() {return myself.loop();} );
	if(!this.renderer.setup()) {
		var p=document.createElement("p");
		p.innerHTML="<p>WebGl seems to be disabled: <a target=_blank href=https://get.webgl.org>Click here to test</a><br> <u>If disabled, Try the following steps:</u></p> "
		p.innerHTML+="<p><b>Firefox:</b> Go to about:config in your address bar,search for webgl and check if webgl.disabled is true <br> No Luck? <a target=_blank href=https://support.mozilla.com/en-US/kb/how-do-i-upgrade-my-graphics-drivers>Help Page</a></p>";
		p.innerHTML+="<p><b>Chrome:</b> <ol><li><b>Is hardware acceleration enabled?</b> Type chrome://settings, show advanced settings -> under system > check Use hardware acceleration when available</li><b><li>Check WebGl:</b> Type chrome://flags into the address bar and confirm that Disable WebGl is gray</li></ol></p>";
		p.innerHTML+= "<p><b>Safari:</b> <ol><li>Go to Safari's Preferences</li><li>Select the Advanced tab</li><li>Check the Show Develop menu in menu bar checkbox</li><li>In the Develop menu, check Enable WebGl</li><li>Confused?<a target=_blank href=http://voicesofaliveness.net/webgl>Pictures</a></li></ol></p>";
		div.appendChild(p);
		this.renderer=null;
	}

	this.div.appendChild(myself.map_display);

}


