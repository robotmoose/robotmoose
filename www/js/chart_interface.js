function chart_interface_t(div) {
	if(!div) return null;

	var _this=this;

	// Stores chart data and related elements. Indexed by sensor name.
	this.charts = {
		canvas: {},
		//smoothie: {},
		chart: {},
		//data: {},
		data_points: {},
		header: {}
	};

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
	this.add_button.onclick=function() {_this.add_chart()};

	this.remove_button=document.createElement("input");
	this.remove_button.type="button";
	this.remove_button.className="btn btn-primary";
	this.remove_button.style.marginLeft=10;
	this.remove_button.style.disabled = false;
	this.remove_button.value="Remove";
	this.remove_button.title_remove="Click here to remove the chart for the selected sensor.";
	this.remove_button.title=this.remove_button.title_remove;
	this.remove_button.onclick=function() {_this.remove_chart()};

	// Add created UI elements to tab.
	this.controls_div.appendChild(this.add_button);
	this.controls_div.appendChild(this.remove_button);
	this.div.appendChild(this.controls_div);
	this.div.appendChild(document.createElement("br"));
	this.div.appendChild(this.chart_div);
}

chart_interface_t.prototype.refresh=function(json) {
	var sensor_list = [];
	for(var prop in json) {
		if(!json.hasOwnProperty(prop)) {
			continue;
		}

		switch(prop) {
			case "power":
				for(var subprop in json[prop]) {
					if(!json[prop].hasOwnProperty(subprop)) {
						continue;
					}
					if(subprop == "servo" || subprop == "pwm") {
						for(i=0; i < json[prop][subprop].length; ++i) {
							sensor_list.push(subprop + "_" + i);
							if(doesExist(this.charts.data_points[subprop + "_" + i])) {
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
				if(doesExist(this.charts.data_points[prop])) {
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
				if(doesExist(this.charts.data_points["kinect"])) {
					for(var subprop in json[prop]) {
						if(!json[prop].hasOwnProperty(subprop)) {
							continue;
						}
						if(subprop == "angle") {
							if(doesExist(this.charts.data_points["kinect"]["angle"])) {
								var ctx = this.charts.canvas["kinect"]["angle"].getContext("2d");
								ctx.clearRect(0, 0, this.charts.canvas["kinect"]["angle"].width, this.charts.canvas["kinect"]["angle"].height);
								ctx.save()
								ctx.translate(this.charts.canvas["kinect"]["angle"].width/2, this.charts.canvas["kinect"]["angle"].height)
								ctx.beginPath();
								//ctx.moveTo(this.charts.canvas["kinect"]["angle"].width/2, this.charts.canvas["kinect"]["angle"].height/2);
								ctx.moveTo(0,0);
								var arrow_length = this.charts.canvas["kinect"]["angle"].height*3/4;
								var arrow_tip_y = -Math.sin((json["kinect"]["angle"]+90)*Math.PI/180.0)*arrow_length;//-this.charts.canvas["kinect"]["angle"].height/2;
								var arrow_tip_x = Math.cos((json["kinect"]["angle"]+90)*Math.PI/180.0)*arrow_length;//+this.charts.canvas["kinect"]["angle"].width/2;



								ctx.lineTo(arrow_tip_x,arrow_tip_y);
								ctx.lineWidth = 10;
								ctx.strokeStyle="blue";
								ctx.stroke();
								ctx.restore();

								//this.charts.data_points["kinect"]["angle"].datasets[0].data[0]=json["kinect"]["angle"];
								//this.charts.chart["kinect"]["angle"].update();
								this.charts.header["kinect"]["angle"].innerHTML = "Direction of Arrival: " + json["kinect"]["angle"] + "\xB0";
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
					if(doesExist(this.charts.data_points[prop + "_" + i])) {
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
				if(doesExist(this.charts.data_points[prop])) {
					for(i=0; i<6; ++i) {
						this.charts.data_points[prop].datasets[0].data[i]=json[prop][i];
					}
					this.charts.chart[prop].update();
				}

				break;
			case "floor":
				sensor_list.push(prop);
				if(doesExist(this.charts.data_points[prop])) {
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

var doesExist=function(variable) {
	if(typeof variable === "undefined" || variable === null)
		return false;
	return true;
}

chart_interface_t.prototype.add_chart=function() {
	var _this = this;
	switch(_this.chart_drop.value) {
		case "kinect":
			if(!doesExist(_this.charts.data_points["kinect"])) {
				_this.charts.data_points["kinect"] = {};
				_this.charts.canvas["kinect"] = {};
				_this.charts.header["kinect"] = {};
				_this.charts.chart["kinect"] = {};
			}
			if(!doesExist(_this.charts.data_points["kinect"]["angle"])) {
			 	_this.charts.canvas["kinect"]["angle"] = document.createElement("canvas");
			 	_this.charts.header["kinect"]["angle"] = document.createElement("h4");
			 	_this.charts.header["kinect"]["angle"].innerHTML = _this.chart_drop.value;
			 	_this.charts.canvas["kinect"]["angle"].width = 400;
			 	_this.charts.canvas["kinect"]["angle"].height = 200;

				_this.chart_div.appendChild(_this.charts.header["kinect"]["angle"]);
				_this.chart_div.appendChild(_this.charts.canvas["kinect"]["angle"]);

				// Initialize the data
				_this.charts.data_points["kinect"]["angle"] = {
					labels:[],
					datasets: [{
						data: [],
						// Line Options
						fill: false, // Don't fill area under line
						pointRadius: 0, // Don't draw points
						borderColor: "#0BB5FF"
					}]
				};

				// // Create the chart
				// _this.charts.chart["kinect"]["angle"] = new Chart(_this.charts.canvas["kinect"]["angle"].getContext('2d'), {
				// 	type: 'line',
				// 	data: _this.charts.data_points["kinect"]["angle"],
				// 	options: {
				// 		responsive: true,
				// 		maintainAspectRatio: true,
				// 		animation: {
				// 			duration: 10
				// 		},
				// 		legend: {
				// 			display: false
				// 		},
				// 		scales: {
				// 			xAxes: [{
				// 				display: false
				// 			}],
				// 			yAxes: [{
				// 				gridLines: {
				// 					display: false
				// 				}
				// 			}]
				// 		}
				// 	}
				// });

			}
			break;

		default:
			if(!doesExist(_this.charts.data_points[_this.chart_drop.value])) {
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
						// _this.charts.canvas[_this.chart_drop.value].style="-webkit-transform: rotate(-71deg);"
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
	// if(!doesExist(_this.charts.data_points[_this.chart_drop.value])) {
	// 	switch(_this.chart_drop.value) {
	// 		case "kinect":

	// 			if(!doesExist(_this.charts.data_points["kinect"]["angle"])) {
	// 			 	_this.charts.canvas["kinect"]["angle"] = document.createElement("canvas");
	// 			 	_this.charts.header["kinect"]["angle"] = document.createElement("h4");
	// 			 	_this.charts.header["kinect"]["angle"].innerHTML = _this.chart_drop.value;
	// 			 	_this.charts.canvas["kinect"]["angle"].width = 500;
	// 			 	_this.charts.canvas["kinect"]["angle"].height = 100;

	// 				_this.chart_div.appendChild(_this.charts.header["kinect"]["angle"]);
	// 				_this.chart_div.appendChild(_this.charts.canvas["kinect"]["angle"]);

	// 				// Initialize the data
	// 				_this.charts.data_points[_this.chart_drop.value] = {
	// 					labels:[],
	// 					datasets: [{
	// 						data: [],
	// 						// Line Options
	// 						fill: false, // Don't fill area under line
	// 						pointRadius: 0, // Don't draw points
	// 						borderColor: "#0BB5FF"
	// 					}]
	// 				};
	// 			}
	// 			break;

	// 		default:
	// 			_this.charts.canvas[_this.chart_drop.value] = document.createElement("canvas");
	// 	 		_this.charts.header[_this.chart_drop.value] = document.createElement("h4");
	// 	 		_this.charts.header[_this.chart_drop.value].innerHTML = _this.chart_drop.value;
	// 	 		_this.charts.canvas[_this.chart_drop.value].width = 500;
	// 	 		_this.charts.canvas[_this.chart_drop.value].height = 100;

	// 			_this.chart_div.appendChild(_this.charts.header[_this.chart_drop.value]);
	// 			_this.chart_div.appendChild(_this.charts.canvas[_this.chart_drop.value]);

	// 			// Initialize the data
	// 			_this.charts.data_points[_this.chart_drop.value] = {
	// 				labels:[],
	// 				datasets: [{
	// 					data: [],
	// 					// Line Options
	// 					fill: false, // Don't fill area under line
	// 					pointRadius: 0, // Don't draw points
	// 					borderColor: "#0BB5FF"
	// 				}]
	// 			};
	// 			break;
	// 	}
		// Create the actual chart
		// switch(_this.chart_drop.value) {
		// 	case "light":
		// 		_this.charts.canvas[_this.chart_drop.value].width = 400;
		// 		_this.charts.canvas[_this.chart_drop.value].height = 200;
		// 		// _this.charts.canvas[_this.chart_drop.value].style="-webkit-transform: rotate(-71deg);"
		// 		for(i=0; i<6; ++i) {
		// 			_this.charts.data_points[_this.chart_drop.value].datasets[0].data.push(0);
		// 			_this.charts.data_points[_this.chart_drop.value].labels.push(i.toString());
		// 		}
		// 		for(i=6; i<13; ++i) {
		// 			_this.charts.data_points[_this.chart_drop.value].datasets[0].data.push(0);
		// 			_this.charts.data_points[_this.chart_drop.value].labels.push("");
		// 		}
		// 		_this.charts.data_points[_this.chart_drop.value].datasets.push({
		// 			data: [],
		// 			fill:false,
		// 			pointRadius: 0,
		// 			borderColor: "rgba(0,0,0,0)"
		// 		});
		// 		for(i=0; i<13; ++i) {
		// 			_this.charts.data_points[_this.chart_drop.value].datasets[1].data.push(175);
		// 		}

		// 		_this.charts.data_points[_this.chart_drop.value].datasets[0].fill = true;
		// 		_this.charts.data_points[_this.chart_drop.value].datasets[0].pointRadius = 2;

		// 		_this.charts.chart[_this.chart_drop.value] = new Chart(_this.charts.canvas[_this.chart_drop.value].getContext('2d'), {
		// 			type: 'radar',
		// 			data: _this.charts.data_points[_this.chart_drop.value],
		// 			options: {
		// 				responsive: true,
		// 				maintainAspectRatio: true,
		// 				animation: {
		// 					duration: 300
		// 				},
		// 				tooltips: {
		// 					enabled: false
		// 				},
		// 				legend: {
		// 					display: false
		// 				},
		// 				scale: {
		// 					ticks: {
		// 						display: false
		// 					}
		// 				}
		// 			}
		// 		});
		// 		break;
		// 	case "floor":
		// 		for(i=0; i<4; ++i) {
		// 			_this.charts.data_points[_this.chart_drop.value].datasets[0].data.push(0);
		// 			_this.charts.data_points[_this.chart_drop.value].labels.push(i.toString());
		// 		}
		// 		_this.charts.data_points[_this.chart_drop.value].datasets[0].backgroundColor = "#0BB5FF";
		// 		_this.charts.chart[_this.chart_drop.value] = new Chart(_this.charts.canvas[_this.chart_drop.value].getContext('2d'), {
		// 			type: 'bar',
		// 			data: _this.charts.data_points[_this.chart_drop.value],
		// 			options: {
		// 				responsive: true,
		// 				maintainAspectRatio: true,
		// 				animation: {
		// 					duration: 300
		// 				},
		// 				tooltips: {
		// 					enabled: false
		// 				},
		// 				legend: {
		// 					display: false
		// 				},
		// 				scales: {
		// 					yAxes: [{
		// 						ticks: {
		// 							beginAtZero: true
		// 						}
		// 					}]
		// 				}
		// 			}
		// 		 });
		// 		break;
		// 	case "kinect":
		// 		_this.charts.chart["kinect"]["angle"] = new Chart(_this.charts.canvas["kinect"]["angle"].getContext('2d'), {
		// 			type: 'line',
		// 			data: _this.charts.data_points["kinect"]["angle"],
		// 			options: {
		// 				responsive: true,
		// 				maintainAspectRatio: true,
		// 				animation: {
		// 					duration: 10
		// 				},
		// 				legend: {
		// 					display: false
		// 				},
		// 				scales: {
		// 					xAxes: [{
		// 						display: false
		// 					}],
		// 					yAxes: [{
		// 						gridLines: {
		// 							display: false
		// 						}
		// 					}]
		// 				}
		// 			}
		// 		});
		// 		break;
		// 	default:
		// 		_this.charts.chart[_this.chart_drop.value] = new Chart(_this.charts.canvas[_this.chart_drop.value].getContext('2d'), {
		// 			type: 'line',
		// 			data: _this.charts.data_points[_this.chart_drop.value],
		// 			options: {
		// 				responsive: true,
		// 				maintainAspectRatio: true,
		// 				animation: {
		// 					duration: 10
		// 				},
		// 				legend: {
		// 					display: false
		// 				},
		// 				scales: {
		// 					xAxes: [{
		// 						display: false
		// 					}],
		// 					yAxes: [{
		// 						gridLines: {
		// 							display: false
		// 						}
		// 					}]
		// 				}
		// 			}
		// 		});
		// 		break;
		// }669
		//6
// 	}
// }

chart_interface_t.prototype.remove_chart=function() {
	var _this = this;
	if(doesExist(_this.charts.data_points[_this.chart_drop.value])) {
		switch(_this.chart_drop.value) {
			case "kinect":
				// Clean up
				//_this.charts.chart["kinect"]["angle"].destroy();
				_this.chart_div.removeChild(_this.charts.canvas["kinect"]["angle"]);
				_this.chart_div.removeChild(_this.charts.header["kinect"]["angle"]);
				_this.charts.canvas["kinect"]["angle"] = null;
				_this.charts.header["kinect"]["angle"] = null;
				_this.charts.data_points["kinect"]["angle"] = null;
				_this.charts.chart["kinect"]["angle"] = null;

				_this.charts.canvas["kinect"] = null;
				_this.charts.header["kinect"] = null;
				_this.charts.data_points["kinect"] = null;
				_this.charts.chart["kinect"] = null;

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


// function drawArrow(canvas, fromx, fromy, tox, toy){
//                 //variables to be used when creating the arrow
//                 var ctx = canvas.getContext("2d");
//                 var headlen = 10;

//                 var angle = Math.atan2(toy-fromy,tox-fromx);

//                 //starting path of the arrow from the start square to the end square and drawing the stroke
//                 ctx.beginPath();
//                 ctx.moveTo(fromx, fromy);
//                 ctx.lineTo(tox, toy);
//                 ctx.strokeStyle = "#cc0000";
//                 ctx.lineWidth = 22;
//                 ctx.stroke();

//                 //starting a new path from the head of the arrow to one of the sides of the point
//                 ctx.beginPath();
//                 ctx.moveTo(tox, toy);
//                 ctx.lineTo(tox-headlen*Math.cos(angle-Math.PI/7),toy-headlen*Math.sin(angle-Math.PI/7));

//                 //path from the side point of the arrow, to the other side point
//                 ctx.lineTo(tox-headlen*Math.cos(angle+Math.PI/7),toy-headlen*Math.sin(angle+Math.PI/7));

//                 //path from the side point back to the tip of the arrow, and then again to the opposite side point
//                 ctx.lineTo(tox, toy);
//                 ctx.lineTo(tox-headlen*Math.cos(angle-Math.PI/7),toy-headlen*Math.sin(angle-Math.PI/7));

//                 //draws the paths created above
//                 ctx.strokeStyle = "#cc0000";
//                 ctx.lineWidth = 22;
//                 ctx.stroke();
//                 ctx.fillStyle = "#cc0000";
//                 ctx.fill();
//             }