var ws;

function setup() {
    if ("WebSocket" in window)
    {
        console.log("WebSocket is supported by your Browser!");
        // Let us open a web socket
        ws = new WebSocket("ws://" + "127.0.0.1" + ":8080/SLAM/driveKatie");
        ws.onopen = function()
        {
            console.log("Socket is open...");
            $("#out").html("<h3>Connected to " + "127.0.0.1" + "</h3>");
        };
        ws.onmessage = function(evt)
        {
            var received_msg = evt.data;
            console.log("Message is received: " + received_msg);
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