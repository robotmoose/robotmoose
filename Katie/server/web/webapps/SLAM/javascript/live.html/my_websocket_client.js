var ws;

function setup() {
    if ("WebSocket" in window)
    {
        console.log("WebSocket is supported by your Browser!");
        // Let us open a web socket
        ws = new WebSocket("ws://" + server_ip + ":8080/SLAM/liveData");
        ws.onopen = function()
        {
            console.log("Socket is open...");
            $("#out").html("<h3>Connected to " + server_ip + "</h3>");
        };
        ws.onmessage = function(evt)
        {
            var received_msg = evt.data;

            var vs = received_msg.split(" ");
            var values = new Array();
            for (var i = 0; i < vs.length; i++) {
                var t = vs[i].trim();
                if (t.length === 0) {
                    continue;
                }
                values.push(t);
            }

            if (values.length < 12) {
                $("#out").html("<h3 style='color: #ce0f0f;'>" + "Data packet is corrupted: " + values.length + "</h3>");
                return;
            }
            piezo = parseInt(values[0]);
            therm = parseInt(values[1]);
            light = parseInt(values[2]);
            rear_range = parseInt(values[3]);
            ir_therm1 = parseInt(values[4]);
            ir_therm2 = parseInt(values[5]);
            bump_wheel = parseInt(values[6]);
            cd1 = parseInt(values[7]);
            cd2 = parseInt(values[8]);
            cd3 = parseInt(values[9]);
            cd4 = parseInt(values[10]);
            cd5 = parseInt(values[11]);
            addValue();
        };
        ws.onclose = function()
        {
            console.log("Connection is closed...");
            $("#out").html("<h3 style='color: #ce0f0f;'>WebSocket connection is closed, please refresh this page</h3>");
        };
    }
    else
    {
        // The browser doesn't support WebSocket
        $("#out").html("<h3 style='color: #ce0f0f;'>WebSocket NOT supported by your Browser!</h3>");
    }
}

setup();

window.onunload = function() {
    ws.close();
};