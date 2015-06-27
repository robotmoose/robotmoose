function getRobotControlScope() {
    return angular.element(document.getElementById("RobotControl")).scope();
}

function startRecording(name) {
    if (ws2.readyState !== ws2.OPEN) {
        alert("WebSocket is not connected yet!");
        return;
    }
    ws2.send(JSON.stringify(new ControlMessage("topic.datalog", "command.datalog.start", name)));
}

function endRecording() {
    if (ws2.readyState !== ws2.OPEN) {
        alert("WebSocket is not connected yet!");
        return;
    }
    ws2.send(JSON.stringify(new ControlMessage("topic.datalog", "command.datalog.end")));
}

var init_switch = false;
function switchRecording(elem, value) {
    if (init_switch) {
        init_switch = false;
        return;
    }

    if (value) {
        if (!$("#input_exp_name").val() || $("#input_exp_name").val() === "Please input a note for the experiment to start") {
            alert("Please input a note first");
            init_switch = true;
            button_switch.prop("checked", false).iphoneStyle("refresh");
            return;
        }

        var name = $("#input_exp_name").val();
        startRecording(name);
    } else {
        endRecording();
    }
}

window.onunload = function() {
    ws.close();
    ws2.close();
    if (chart !== null) {
        chart.destroy();
    }
};

var chart = null;
var options = {
    chart: {
        renderTo: "graph-container",
        type: "spline",
        zoomType: "x"
    },
    title: {
        text: "",
        x: -20
    },
    xAxis: {
        categories: [],
        labels: {
            formatter: function() {
                var tokens = this.value.split(":");
                return tokens[1] + ":" + tokens[2];
            }
        }
    },
    yAxis: {
        min: 0,
        title: {
            text: ""
        }
    },
    legend: {
        enabled: false
    },
    plotOptions: {
        spline: {
            lineWidth: 2,
            marker: {
                enabled: false
            }
        }
    },
    series: []
};

function cleanGraph() {
    if (chart !== null) {
        chart.destroy();
    }
}

function makeGraph(robotId, startTime) {
    cleanGraph();

    $.ajax({
        dataType: "json",
        url: "getDataValues",
        data: {robotId: robotId, startTime: startTime},
        success: function(json) {
            options.xAxis.categories = json["categories"];
            options.xAxis.tickInterval = 100;
            options.series[0] = json["series"][0];
            options.series[1] = json["series"][1];
            options.title.text = "IR Values of Robot " + robotId;
            options.yAxis.title.text = "IR";
            chart = new Highcharts.Chart(options);
            $("text[text-anchor='end'][zIndex='8']").hide();
        }
    });
}