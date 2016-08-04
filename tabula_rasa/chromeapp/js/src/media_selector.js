function media_selector_t(div,video)
{
	if(!div) return null;

	this.div = div;
	this.el = new_div(this.div);
	var _this = this;

	this.dropdown = new dropdown_t(this.el, null, function(){_this.set_camera();});
	this.dropdown.set_width("100%");
	this.video_el=video;
	this.interval=setInterval(function (){_this.update_list_m();},250);
};

media_selector_t.prototype.update_list_m = function()
{
	var _this = this;
	navigator.mediaDevices.enumerateDevices()
		.then(function(devices) {
			var video_input_labels = [];
			_this.devices=[];
			devices.forEach(function(device) {
				if(device.kind === "videoinput") {
					_this.devices.push(device);
					video_input_labels.push(device.label);
				}
			});
			_this.dropdown.build(video_input_labels, null);
	});
};

media_selector_t.prototype.set_camera=function()
{
	if(chrome.runtime.lasstError)
		console.log("DO SOMETHING WITH THIS");
	var selected_camera = this.devices[this.dropdown.selected_index()].deviceId;
	this.video_el.executeScript({code:"console.log('injected script'); \
				var local_video = document.getElementById('local-video'); \
				console.log(local_video); var camera= \"\
				" + selected_camera + "\"; console.log(camera);\
				"});

}
