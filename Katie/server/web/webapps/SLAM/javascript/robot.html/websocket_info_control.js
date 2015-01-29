var ws2;

function setupInfoControl() {
    if ("WebSocket" in window)
    {
        ws2 = new WebSocket("ws://" + server_ip + ":8080/SLAM/infoControl");
        ws2.onopen = function()
        {
            console.log("Socket infoControl is open...");
            ws2.send(JSON.stringify(new ControlMessage("topic.datalog", "command.datalog.check")));
        };
        ws2.onmessage = function(evt)
        {
            var received_msg = evt.data;
            console.log("Message is received: " + received_msg);
            var message = $.parseJSON(received_msg);
            if (message.topic === "topic.datalog") {
                if (message.command === "command.datalog.start") {
                    init_switch = true;
                    button_switch.prop("checked", true).iphoneStyle("refresh");

                    $("#input_exp_name").hide();
                    $("#show_exp_name").text("Current Experiment Note: " + message.content[1]);
                    $("#show_exp_name").show();
                    $("#show_exp_id").text("Current Experiment ID: " + message.content[0]);
                    $("#show_exp_id").show();

                } else if (message.command === "command.datalog.end") {
                    init_switch = true;
                    button_switch.prop("checked", false).iphoneStyle("refresh");

                    $("#show_exp_name").hide();
                    $("#show_exp_id").hide();
                    $("#input_exp_name").val("Please input a note for the experiment to start");
                    $("#input_exp_name").show();
                }
            } else if (message.topic === "topic.dialog.count") {
                $("#total_dialogs").text(message.command);
            }

        };
        ws2.onclose = function()
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