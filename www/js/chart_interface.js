function chart_interface_t(div) {
	if(!div) return null;

	var _this=this;

	// Stores chart data and related elements. Indexed by sensor name.
	this.charts = {
		canvas: {},
		chart: {},
		data_points: {},
		header: {},
		image: {}
	}
	this.images = {
		kinect: new Image()
	}
	this.images.kinect.src = "/images/Half_Circle_Angles.jpg";
	this.kinect = {};
	this.kinect.flap_counter = 0;
	this.kinect.flap_sound_playing = false;

	// ***** Create UI ***** //
	this.div=div;
	this.controls_div=document.createElement("div");
	this.chart_div=document.createElement("div");

	this.chart_drop=new dropdown_t(this.controls_div);

	this.add_button=document.createElement("input");
	this.add_button.type="button";
	this.add_button.className="btn btn-primary";
	this.add_button.style.marginLeft=10;
	this.add_button.style.disabled = false;
	this.add_button.value="Add";
	this.add_button.title_add="Click here to add a chart for the selected sensor.";
	this.add_button.title=this.add_button.title_add;
	this.add_button.addEventListener("click", function() {
		_this.add_chart();
	});

	this.remove_button=document.createElement("input");
	this.remove_button.type="button";
	this.remove_button.className="btn btn-primary";
	this.remove_button.style.marginLeft=10;
	this.remove_button.style.disabled = false;
	this.remove_button.value="Remove";
	this.remove_button.title_remove="Click here to remove the chart for the selected sensor.";
	this.remove_button.title=this.remove_button.title_remove;
	this.remove_button.addEventListener("click", function() {
		_this.remove_chart();
	});

	// Add created UI elements to tab.
	this.controls_div.appendChild(this.add_button);
	this.controls_div.appendChild(this.remove_button);
	this.div.appendChild(this.controls_div);
	this.div.appendChild(document.createElement("br"));
	this.div.appendChild(this.chart_div);
}

chart_interface_t.prototype.connect_joints = function(ctx, joint1, joint2, offset) {
	if(joint1 == null || joint2 == null) return;

	ctx.beginPath();
	ctx.moveTo(joint1.screen_x - offset.x, joint1.screen_y - offset.y);
	ctx.lineTo(joint2.screen_x - offset.x, joint2.screen_y - offset.y);
	ctx.lineWidth = 4;
	ctx.strokeStyle="blue";
	ctx.stroke();
}

chart_interface_t.prototype.Vec2 = function(x_value, y_value) {
	return {x: x_value, y: y_value};
}

chart_interface_t.prototype.refresh=function(json) {
	var sensor_list = [];
	for(let prop in json) {
		if(!json.hasOwnProperty(prop)) {
			continue;
		}

		switch(prop) {
			case "power":
				for(let subprop in json[prop]) {
					if(!json[prop].hasOwnProperty(subprop)) {
						continue;
					}
					if(subprop == "servo" || subprop == "pwm") {
						for(i=0; i < json[prop][subprop].length; ++i) {
							sensor_list.push(subprop + "_" + i);
							if(is_value(this.charts.data_points[subprop + "_" + i])) {
								this.charts.data_points[subprop + "_" + i].datasets[0].data.push(json[prop][subprop][i]);
								this.charts.data_points[subprop + "_" + i].labels.push(0);
								if(this.charts.data_points[subprop + "_" + i].datasets[0].data.length > 60) {
									this.charts.data_points[subprop + "_" + i].datasets[0].data.shift();
									this.charts.data_points[subprop + "_" + i].labels.shift();
								}
								this.charts.chart[subprop + "_" + i].update();
								this.charts.header[subprop + "_" + i].innerHTML = subprop + "_" + i + " (" + json[prop][subprop][i] + ")";
							}
						}
					}
				}
				break;
			// Sensors where there can be only one.
			case "bumper":
			case "heartbeats":
			case "latency":
				sensor_list.push(prop);
				if(is_value(this.charts.data_points[prop])) {
					this.charts.data_points[prop].datasets[0].data.push(json[prop]);
					this.charts.data_points[prop].labels.push(0);
					if(this.charts.data_points[prop].datasets[0].data.length > 30) {
						this.charts.data_points[prop].datasets[0].data.shift();
						this.charts.data_points[prop].labels.shift();
					}
					this.charts.chart[prop].update();
					this.charts.header[prop].innerHTML = prop + " (" + json[prop] + ")";
				}
				break;

			case "kinect":
				sensor_list.push(prop);
				if(is_value(this.charts.canvas["kinect"])) {
					for(let subprop in json[prop]) {
						if(!json[prop].hasOwnProperty(subprop)) {
							continue;
						}
						if(subprop == "angle") {
							if(is_value(this.charts.canvas["kinect"]["angle"])) { // Draw the background and the angled line
								var ctx = this.charts.canvas["kinect"]["angle"].getContext("2d");
								ctx.clearRect(0, 0, this.charts.canvas["kinect"]["angle"].width, this.charts.canvas["kinect"]["angle"].height);
								ctx.drawImage(this.images.kinect,0,0, this.charts.canvas["kinect"]["angle"].width, this.charts.canvas["kinect"]["angle"].height);
								ctx.save()
								var arrow_base_offset = this.Vec2(2, -16);
								ctx.translate(this.charts.canvas["kinect"]["angle"].width/2+arrow_base_offset.x, this.charts.canvas["kinect"]["angle"].height+arrow_base_offset.y);
								ctx.beginPath();
								ctx.moveTo(0,0);
								var arrow_length = this.charts.canvas["kinect"]["angle"].height*3/4;
								var arrow_tip = this.Vec2(
									Math.cos((json["kinect"]["angle"]-90)*Math.PI/180.0)*arrow_length,
									Math.sin((json["kinect"]["angle"]-90)*Math.PI/180.0)*arrow_length
								);


								ctx.lineTo(arrow_tip.x,arrow_tip.y);
								ctx.lineWidth = 5;
								ctx.strokeStyle="blue";
								ctx.stroke();
								ctx.restore();
								this.charts.header["kinect"]["angle"].innerHTML = "Direction of Arrival: " + json["kinect"]["angle"] + "\xB0";
							}
						}
						if(subprop == "flapping") {
							if(json["kinect"]["flapping"]) {
								if(this.kinect.flap_counter < 50)
									this.kinect.flap_counter++;
								if(this.kinect.flap_counter >= 10) {
									if(!(robot_ui.widgets.sound.play_sound_button.disabled || this.flap_sound_playing)) {
										robot_ui.widgets.sound.request_sound();
										this.flap_sound_playing = true;
									}
								}
							}
							else {
								if(this.kinect.flap_counter > 0) {
									this.kinect.flap_counter--;
								}
								if(this.kinect.flap_counter < 10) {
									if(robot_ui.widgets.sound.play_sound_button.disabled && this.flap_sound_playing) {
										robot_ui.widgets.sound.stop_sound();
									}
									this.flap_sound_playing = false;
								}
							}
							this.charts.header["kinect"]["joints"].innerHTML = "Kinect Skeleton Tracker (Flap Count: " + this.kinect.flap_counter + "/50)";
						}
						if(subprop == "joints") {
							var ctx = this.charts.canvas["kinect"]["joints"].getContext("2d");
							ctx.fillStyle="grey";
							ctx.fillRect(0, 0, this.charts.canvas["kinect"]["joints"].width, this.charts.canvas["kinect"]["joints"].height);
							if(json["kinect"]["joints"]["left_shoulder"] !== null && json["kinect"]["joints"]["right_shoulder"] !== null) {
								ctx.save();
								var origin = this.Vec2(
									this.charts.canvas["kinect"]["joints"].width/4,
									this.charts.canvas["kinect"]["joints"].height/4
								);
								ctx.translate(origin.x, origin.y);
								var shoulder_center = this.Vec2(
									(json["kinect"]["joints"]["left_shoulder"].screen_x + json["kinect"]["joints"]["right_shoulder"].screen_x)/2,
								 	(json["kinect"]["joints"]["left_shoulder"].screen_y + json["kinect"]["joints"]["right_shoulder"].screen_y)/2
								);

								var offset = this.Vec2(0,0);//(shoulder_center.x - origin.x, shoulder_center.y - origin.y);

								ctx.fillStyle="yellow";
								ctx.fillRect(shoulder_center.x-offset.x-5, shoulder_center.y-offset.y-5, 10, 10);

								// Connect joints with lines
								this.connect_joints(
									ctx,
									json["kinect"]["joints"]["left_shoulder"],
									json["kinect"]["joints"]["right_shoulder"],
									offset
								);
								this.connect_joints(
									ctx,
									json["kinect"]["joints"]["left_shoulder"],
									json["kinect"]["joints"]["left_elbow"],
									offset
								);
								this.connect_joints(
									ctx,
									json["kinect"]["joints"]["left_elbow"],
									json["kinect"]["joints"]["left_hand"],
									offset
								);
								this.connect_joints(
									ctx,
									json["kinect"]["joints"]["right_shoulder"],
									json["kinect"]["joints"]["right_elbow"],
									offset
								);
								this.connect_joints(
									ctx,
									json["kinect"]["joints"]["right_elbow"],
									json["kinect"]["joints"]["right_hand"],
									offset
								);
								if(json["kinect"]["joints"]["head"] !== null
								){
									ctx.beginPath();
									ctx.moveTo(json["kinect"]["joints"]["head"].screen_x - offset.x, json["kinect"]["joints"]["head"].screen_y - offset.y);
									ctx.lineTo(shoulder_center.x - offset.x, shoulder_center.y - offset.y);
									ctx.lineWidth = 4;
									ctx.strokeStyle="blue";
									ctx.stroke();
								}
								// Draw points for joints
								for(let subsubprop in json["kinect"]["joints"]) {
									if(!json["kinect"]["joints"].hasOwnProperty(subsubprop))
										continue;

									if(json["kinect"]["joints"][subsubprop] != null) {
										ctx.fillStyle="black";
										ctx.fillRect(
											json["kinect"]["joints"][subsubprop].screen_x-7.5-offset.x,
											json["kinect"]["joints"][subsubprop].screen_y-7.5-offset.y,
											15,
											15
										);
									}
								}

								ctx.restore();
							}
						}
					}
				}
				break;

			// Sensor where there can be multiple, and they are stored in an array.
			case "analog":
			case "encoder_raw":
			case "ultrasonic":
				for(i=0; i < json[prop].length; ++i) {
					sensor_list.push(prop + "_" + i);
					if(is_value(this.charts.data_points[prop + "_" + i])) {
						this.charts.data_points[prop + "_" + i].datasets[0].data.push(json[prop][i]);
						this.charts.data_points[prop + "_" + i].labels.push(0);
						if(this.charts.data_points[prop + "_" + i].datasets[0].data.length > 30) {
							this.charts.data_points[prop + "_" + i].datasets[0].data.shift();
							this.charts.data_points[prop + "_" + i].labels.shift();
						}
						this.charts.chart[prop + "_" + i].update();
						this.charts.header[prop + "_" + i].innerHTML = prop + "_" + i + " (" + json[prop][i] + ")";
					}
				}
				break;
			// Create 2 Sensors
			case "light":
				sensor_list.push(prop);
				if(is_value(this.charts.data_points[prop])) {
					for(i=0; i<6; ++i) {
						this.charts.data_points[prop].datasets[0].data[i]=json[prop][i];
					}
					this.charts.chart[prop].update();
				}

				break;
			case "floor":
				sensor_list.push(prop);
				if(is_value(this.charts.data_points[prop])) {
					for(i=0; i<4; ++i) {
						this.charts.data_points[prop].datasets[0].data[i]=json[prop][i]
					}
					this.charts.chart[prop].update();
				}
				break;
			default:
				break;
		}
	}
	this.chart_drop.build(sensor_list);
}

chart_interface_t.prototype.add_chart=function() {
	var _this = this;

	switch(_this.chart_drop.value) {
		case "kinect":
			if(!is_value(_this.charts.canvas["kinect"])) {
				_this.charts.canvas["kinect"] = {};
				_this.charts.header["kinect"] = {};
			}
			if(!is_value(_this.charts.canvas["kinect"]["angle"])) {
			 	_this.charts.canvas["kinect"]["angle"] = document.createElement("canvas");
			 	_this.charts.header["kinect"]["angle"] = document.createElement("h4");
				_this.chart_div.appendChild(_this.charts.header["kinect"]["angle"]);
			 	_this.chart_div.appendChild(_this.charts.canvas["kinect"]["angle"]);

			 	_this.charts.header["kinect"]["angle"].innerHTML = _this.chart_drop.value;
			 	_this.charts.canvas["kinect"]["angle"].width = 800;
			 	_this.charts.canvas["kinect"]["angle"].height = 400;
			 	_this.charts.canvas["kinect"]["angle"].style.width="100%";
			 	var ctx = this.charts.canvas["kinect"]["angle"].getContext("2d");
			 	ctx.drawImage(this.images.kinect,0,0, this.charts.canvas["kinect"]["angle"].width, this.charts.canvas["kinect"]["angle"].height);
			}
			if(!is_value(_this.charts.canvas["kinect"]["joints"])) {
				_this.charts.canvas["kinect"]["joints"] = document.createElement("canvas");
				_this.charts.header["kinect"]["joints"] = document.createElement("h4");
				_this.chart_div.appendChild(_this.charts.header["kinect"]["joints"]);
			 	_this.chart_div.appendChild(_this.charts.canvas["kinect"]["joints"]);

			 	_this.charts.header["kinect"]["joints"].innerHTML = "Kinect Skeleton Tracker";
			 	_this.charts.canvas["kinect"]["joints"].width = 1280;
			 	_this.charts.canvas["kinect"]["joints"].height = 800;
			 	_this.charts.canvas["kinect"]["joints"].style.width="100%";
			}
			break;

		default:
			if(!is_value(_this.charts.data_points[_this.chart_drop.value])) {
				_this.charts.canvas[_this.chart_drop.value] = document.createElement("canvas");
		 		_this.charts.header[_this.chart_drop.value] = document.createElement("h4");
		 		_this.charts.header[_this.chart_drop.value].innerHTML = _this.chart_drop.value;
		 		_this.charts.canvas[_this.chart_drop.value].width = 500;
		 		_this.charts.canvas[_this.chart_drop.value].height = 100;

				_this.chart_div.appendChild(_this.charts.header[_this.chart_drop.value]);
				_this.chart_div.appendChild(_this.charts.canvas[_this.chart_drop.value]);

				// Initialize the data
				_this.charts.data_points[_this.chart_drop.value] = {
					labels:[],
					datasets: [{
						data: [],
						// Line Options
						fill: false, // Don't fill area under line
						pointRadius: 0, // Don't draw points
						borderColor: "#0BB5FF"
					}]
				}

				switch(_this.chart_drop.value) {
					case "light":
						_this.charts.canvas[_this.chart_drop.value].width = 400;
						_this.charts.canvas[_this.chart_drop.value].height = 200;
						for(i=0; i<6; ++i) {
							_this.charts.data_points[_this.chart_drop.value].datasets[0].data.push(0);
							_this.charts.data_points[_this.chart_drop.value].labels.push(i.toString());
						}
						for(i=6; i<13; ++i) {
							_this.charts.data_points[_this.chart_drop.value].datasets[0].data.push(0);
							_this.charts.data_points[_this.chart_drop.value].labels.push("");
						}
						_this.charts.data_points[_this.chart_drop.value].datasets.push({
							data: [],
							fill:false,
							pointRadius: 0,
							borderColor: "rgba(0,0,0,0)"
						});
						for(i=0; i<13; ++i) {
							_this.charts.data_points[_this.chart_drop.value].datasets[1].data.push(175);
						}

						_this.charts.data_points[_this.chart_drop.value].datasets[0].fill = true;
						_this.charts.data_points[_this.chart_drop.value].datasets[0].pointRadius = 2;

						_this.charts.chart[_this.chart_drop.value] = new Chart(_this.charts.canvas[_this.chart_drop.value].getContext('2d'), {
							type: 'radar',
							data: _this.charts.data_points[_this.chart_drop.value],
							options: {
								responsive: true,
								maintainAspectRatio: true,
								animation: {
									duration: 300
								},
								tooltips: {
									enabled: false
								},
								legend: {
									display: false
								},
								scale: {
									ticks: {
										display: false
									}
								}
							}
						});
						break;
					case "floor":
						for(i=0; i<4; ++i) {
							_this.charts.data_points[_this.chart_drop.value].datasets[0].data.push(0);
							_this.charts.data_points[_this.chart_drop.value].labels.push(i.toString());
						}
						_this.charts.data_points[_this.chart_drop.value].datasets[0].backgroundColor = "#0BB5FF";
						_this.charts.chart[_this.chart_drop.value] = new Chart(_this.charts.canvas[_this.chart_drop.value].getContext('2d'), {
							type: 'bar',
							data: _this.charts.data_points[_this.chart_drop.value],
							options: {
								responsive: true,
								maintainAspectRatio: true,
								animation: {
									duration: 300
								},
								tooltips: {
									enabled: false
								},
								legend: {
									display: false
								},
								scales: {
									yAxes: [{
										ticks: {
											beginAtZero: true
										}
									}]
								}
							}
						 });
						break;
					default:
						_this.charts.chart[_this.chart_drop.value] = new Chart(_this.charts.canvas[_this.chart_drop.value].getContext('2d'), {
							type: 'line',
							data: _this.charts.data_points[_this.chart_drop.value],
							options: {
								responsive: true,
								maintainAspectRatio: true,
								animation: {
									duration: 10
								},
								legend: {
									display: false
								},
								scales: {
									xAxes: [{
										display: false
									}],
									yAxes: [{
										gridLines: {
											display: false
										}
									}]
								}
							}
						});
						break;
				}
			}
			break;
	}
}

chart_interface_t.prototype.remove_chart=function() {
	var _this = this;
	if(is_value(_this.charts.canvas[_this.chart_drop.value])) {
		switch(_this.chart_drop.value) {
			case "kinect":
				// Clean up
				_this.chart_div.removeChild(_this.charts.canvas["kinect"]["angle"]);
				_this.chart_div.removeChild(_this.charts.header["kinect"]["angle"]);
				_this.charts.canvas["kinect"]["angle"] = null;
				_this.charts.header["kinect"]["angle"] = null;

				_this.chart_div.removeChild(_this.charts.canvas["kinect"]["joints"]);
				_this.chart_div.removeChild(_this.charts.header["kinect"]["joints"]);
				_this.charts.canvas["kinect"]["joints"] = null;
				_this.charts.header["kinect"]["joints"] = null;

				_this.charts.canvas["kinect"] = null;
				_this.charts.header["kinect"] = null;

				break;
			default:
				// Clean up
				_this.charts.chart[_this.chart_drop.value].destroy();
				_this.chart_div.removeChild(_this.charts.canvas[_this.chart_drop.value]);
				_this.chart_div.removeChild(_this.charts.header[_this.chart_drop.value]);
				_this.charts.canvas[_this.chart_drop.value] = null;
				_this.charts.header[_this.chart_drop.value] = null;
				_this.charts.data_points[_this.chart_drop.value] = null;
				_this.charts.chart[_this.chart_drop.value] = null;
				break;
			}

	}
}