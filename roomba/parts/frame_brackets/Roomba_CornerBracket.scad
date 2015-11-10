// Corner bracket for roomba
$fs = 0.01;

wiggle = 0.5;
bar_side = 25.4+wiggle;
bracket_thickness = 2.5;
bracket_height = 20;
epsilon=0.1;
base_thickness = 2.5;
side_support_height = 8;
top_hole = 3;
top_hole_offset = 2;
side_hole = 3.5;
center = (bracket_thickness + bar_side)/2;

module bracket_nohole() {
    translate([-(bracket_thickness + bar_side)/2, -(bracket_thickness + bar_side)/2, 0])
        linear_extrude(height=bracket_height)
            square(bar_side+bracket_thickness);
    translate([(bracket_thickness + bar_side)/2, 0, bracket_height-side_support_height])
        cylinder(d2= bar_side+bracket_thickness, d1=1, h=side_support_height);
    translate([-(bracket_thickness + bar_side)/2, 0, bracket_height-side_support_height])
        cylinder(d2= bar_side+bracket_thickness, d1=1, h=side_support_height);
}
module bracket_mainhole() {
    difference(){
        bracket_nohole();
        translate([-(bar_side)/2, -(bar_side)/2, -base_thickness])
            linear_extrude(height=bracket_height+2*epsilon)
                square(bar_side);
    }
}
module bracket_sidehole() {
    difference(){
        bracket_mainhole();
        translate([-center-epsilon, 0, bracket_height-base_thickness-25.4*.5])
            rotate([0, 90, 0])
                cylinder(d=side_hole, h= bar_side + bracket_thickness+2*epsilon);
    }
}

module bracket(){
    difference(){
        bracket_sidehole();
        translate([center+top_hole_offset, 0, 0])
            cylinder(d=top_hole, h=bracket_height+2*epsilon);
        translate([-(center+top_hole_offset), 0,  0])
            cylinder(d=top_hole, h=bracket_height+2*epsilon);
    }
}

bracket();