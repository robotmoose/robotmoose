
include <layla_config.scad>;

// Distance outside axle for reinforcing
thickwall=4.0;

thick=woodscrew_head_dia+thickwall; // Z thickness of bracket
wide=axle_dia+2*thickwall+2*woodscrew_head_dia; // X length of bracket
high=axle_dia+thickwall; // Y height of bracket

// Put in axle mounting screw hole locations
module axle_holes() {
    translate([0,-axle_dia/2+floor,0])
    rotate([-90,0,0])
    for (side=[-1,+1])
        translate([side*(wide/2-woodscrew_head_dia/2),0,0])
            children();
}

// Build bracket to hold axle to wood frame
module axle_bracket()
{
    intersection() {
difference() {
    union() { // plus
        hull() {
            translate([-wide/2,-axle_dia/2,-thick/2])
                cube([wide,thickwall,thick]);
            cylinder(d=axle_dia+2*thickwall,h=thick,center=true);
        }
        // axle_holes() woodscrew_boss();
    }
    union() { // minus
        // space for axle
        translate([0,-epsilon,0])
        cylinder(d=axle_dia,h=2*thick,center=true);
        
        // space for axle mounting holes
        axle_holes() woodscrew_head(10.0);
    }
}
    // Trim all parts to lie inside bounding box:
    translate([-wide/2,-axle_dia/2,-thick/2])
        cube([wide,high,thick]);
    
    }
}

axle_bracket();
