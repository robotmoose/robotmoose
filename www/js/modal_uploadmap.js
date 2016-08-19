/* modal_uploadmap.js

	Upload map image
	onupload(src, width, height, title) triggered when confirm button pressed
*/

function modal_uploadmap_t(div, onupload)
{
	var myself = this;
	this.div = this;
	this.onupload = onupload;
	
	this.modal = new modal_t(div);
	
	// new modal popup
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


					var dim_text = "Image Dimensions (in pixels): "  + myself.uploaded_map.width + ":" + myself.uploaded_map.height;
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

}

modal_uploadmap_t.prototype.scale_onchange_m=function() // when height or width are changed
{
	var myself = this;
	if (myself.upload_width && myself.upload_height)
	{
		if (myself.upload_width.value >= 1 && myself.upload_width.value <= 100 && myself.upload_height.value >= 1 && myself.upload_height.value <= 100)
			myself.confirm_upload_button.disabled=false;
		else
			myself.confirm_upload_button.disabled=true;
	}
}

modal_uploadmap_t.prototype.confirm_upload_button_pressed_m=function()
{
	var myself = this;
	if (myself.uploaded_map&&myself.onupload)
	{
		
		myself.onupload(myself.uploaded_map.src, myself.upload_width.value, myself.upload_height.value, myself.upload_title.value);
		
		/*
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
		*/
	}

}


modal_uploadmap_t.prototype.show=function()
{
	this.modal.show();
}


modal_uploadmap_t.prototype.hide=function()
{
	this.modal.hide();
}

