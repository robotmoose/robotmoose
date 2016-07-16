function init() {
	robot = {};
	superstar_select = document.getElementById('superstar-select');
	year_select = document.getElementById('year-select');
	school_select = document.getElementById('school-select');
	robot_select = document.getElementById('robot-select');
	connect_button = document.getElementById('connect-button');
	log_area = document.getElementById('log');

	superstar_select.addEventListener('change', superstar_changed);
	year_select.addEventListener('change', year_changed);
	school_select.addEventListener('change', school_changed);
	connect_button.addEventListener('touchend', connect);

	serial.requestPermission(null, function() {
		log('You must grant this app permission to use serial devices.')
	});

	superstar_changed();
}

function log(text) {
	log_area.innerHTML += text;
	log_area.innerHTML += '<br />';
	log_area.scrollTop = log_area.scrollHeight;
}

function superstar_changed() {
	robot.superstar = superstar_select.value;
	superstar_sub(robot, '', function(data) {
		data.forEach(function(entry) {
			var option = document.createElement('option');
			option.textContent = entry;
			year_select.add(option);
			year_changed();
		})
	}, function(error) {
		log(error);
	});
}

function year_changed() {
	robot.year = year_select.value;
	superstar_sub(robot, '', function(data) {
		data.forEach(function(entry) {
			var option = document.createElement('option');
			option.textContent = entry;
			school_select.add(option);
			school_changed();
		})
	}, function(error) {
		log(error);
	});
}

function school_changed() {
	robot.school = school_select.value;
	superstar_sub(robot, '', function(data) {
		data.forEach(function(entry) {
			var option = document.createElement('option');
			option.textContent = entry;
			robot_select.add(option);
		})
	}, function(error) {
		log(error);
	});
}

function robot_changed() {
	robot.name = robot_select.value;
}

function connect() {
	log('Connecting...');
	connection = new connection_t(log, log, log, log, log, log);
}

document.addEventListener('deviceready', init);
