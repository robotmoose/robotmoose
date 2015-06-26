(function() {

    var n = 243,
            duration = 750,
            now = new Date(Date.now() - duration),
            data = d3.range(n).map(function() {
        return 0;
    });

    var margin = {top: 6, right: 0, bottom: 20, left: 40},
    width = 960 - margin.right,
            height = 120 - margin.top - margin.bottom;

    var x = d3.time.scale()
            .domain([now - (n - 2) * duration, now - duration])
            .range([0, width]);

    var y = d3.scale.linear()
            .range([height, 0]);

    var line = d3.svg.line()
            .interpolate("basis")
            .x(function(d, i) {
        return x(now - (n - 1 - i) * duration);
    })
            .y(function(d, i) {
        return y(d);
    });

    var svg = d3.select("#d3").append("p").append("svg")
            .attr("width", width + margin.left + margin.right)
            .attr("height", height + margin.top + margin.bottom)
            .style("margin-left", -margin.left + "px")
            .append("g")
            .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

    svg.append("defs").append("clipPath")
            .attr("id", "clip")
            .append("rect")
            .attr("width", width)
            .attr("height", height);

    var axis = svg.append("g")
            .attr("class", "x axis")
            .attr("transform", "translate(0," + height + ")")
            .call(x.axis = d3.svg.axis().scale(x).orient("bottom"));

    var path = svg.append("g")
            .attr("clip-path", "url(#clip)")
            .append("path")
            .data([data])
            .attr("class", "line");

    tick();

    function tick() {

        // update the domains
        now = new Date();
        x.domain([now - (n - 2) * duration, now - duration]);
        y.domain([0, d3.max(data)]);

        // push the random data
        data.push(Math.floor((Math.random() * 10) + 1));

        // redraw the line
        svg.select(".line")
                .attr("d", line)
                .attr("transform", null);

        // slide the x-axis left
        axis.transition()
                .duration(duration)
                .ease("linear")
                .call(x.axis);

        // slide the line left
        path.transition()
                .duration(duration)
                .ease("linear")
                .attr("transform", "translate(" + x(now - (n - 1) * duration) + ")")
                .each("end", tick);

        // pop the old data point off the front
        data.shift();

    }

})();