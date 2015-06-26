var ws;

function setupDriveKatie() {
    if ("WebSocket" in window)
    {
        ws = new WebSocket("ws://" + server_ip + ":8080/SLAM/driveKatie");
        ws.onopen = function()
        {
            console.log("Socket driveKatie is open...");
        };
        ws.onmessage = function(evt)
        {
            var received_msg = evt.data;
            console.log("Message is received: " + received_msg);
            var m = JSON.parse(received_msg);
            if (m.command === "command.report.status") {
                if (m.content === false) {
                    getRobotControlScope().setOnline(false);
                    alert("Robot not online");
                    return;
                }
            } else if (m.topic === "topic.robot.status") {
                var robotInfo = JSON.parse(m.content);
                if (robotInfo.robot_id === robotId) {
                    if (robotInfo.model_e && !robotInfo.model_t) {
                        getRobotControlScope().setModel(1, 0);
                    } else if (!robotInfo.model_e && robotInfo.model_t) {
                        getRobotControlScope().setModel(0, 1);
                    } else if (!robotInfo.model_e && !robotInfo.model_t) {
                        getRobotControlScope().setModel(0, 0);
                    } else {
                        alert("Model E and Model T are not valid!");
                    }

                    if (robotInfo.turn_a) {
                        getRobotControlScope().setQuickTurn(1);
                    } else {
                        getRobotControlScope().setQuickTurn(0);
                    }

                    if (robotInfo.do_explore) {
                        getRobotControlScope().setExplore(1);
                    } else {
                        getRobotControlScope().setExplore(0);
                    }

                    if (robotInfo.do_roomba) {
                        getRobotControlScope().setRoomba(1);
                    } else {
                        getRobotControlScope().setRoomba(0);
                    }

                    if (robotInfo.auto_take) {
                        getRobotControlScope().setRotation(robotInfo.rotation);
                        getRobotControlScope().setInterval(robotInfo.interval);
                        getRobotControlScope().setAutoTake(true, 1);
                    } else {
                        getRobotControlScope().setAutoTake(false, 0);
                    }

                    getRobotControlScope().setOnline(true);
                } else {
                    console.log("Received status not for this robot: " + robotInfo.robot_id);
                }
            } else if (m.topic === "topic.refresh") {
                getRobotControlScope().refresh();
            } else if (m.topic === "topic.new.photo") {
                //getRobotControlScope().refresh();
            } else if (m.topic === "topic.perception") {
                var data = JSON.parse(m.content);
                if (data[0] === robotId) {
                    if (data[1] === "nothing") {
                        getRobotControlScope().setPerception("I see nothing");
                    } else if (data[1] === "kick") {
                        getRobotControlScope().setPerception("Someone kicked me");
                    }
                }
            } else if (m.topic === "topic.execute") {
                var data = JSON.parse(m.content);
                if (data[0] === robotId) {
                    getRobotControlScope().setCommandExecution(data[1]);
                }
            } else if (m.topic === "topic.current.heading") {
                var data = JSON.parse(m.content);
                if (data[0] === robotId) {
                    getRobotControlScope().setCurrentHeading(data[1]);
                }
            } else if (m.topic === "topic.rear.distance") {
                var data = JSON.parse(m.content);
                if (data[0] === robotId) {
                    getRobotControlScope().setRearDistance(data[1]);
                }
            }
        };
        ws.onclose = function()
        {
            console.log("Connection is closed...");
            alert("Connection is closed");
        };
    }
    else
    {
        // The browser doesn't support WebSocket
        console.log("WebSocket NOT supported by your Browser!");
    }
}