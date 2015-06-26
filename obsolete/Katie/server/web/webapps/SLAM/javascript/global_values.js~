var server_ip = "172.30.28.212";//"156.56.246.86";//156.56.246.86

function ControlMessage(topic, command, content)
{
    this.topic = topic;
    this.command = command;
    this.content = content;
}

var COMMAND_TYPES;

$.ajax({
    url: "/SLAM/javascript/protocol.json",
    type: "get",
    async: false,
    dataType: "json",
    success: function(data) {
        COMMAND_TYPES = data.commands;
    }
});
