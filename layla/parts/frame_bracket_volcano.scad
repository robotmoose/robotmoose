// Ridiculously complex bracket to hold a chunk of EMT tubing
include <layla_config.scad>;

// Size of floorplate of bracket
radius=50;

floor=3;

// Bracket mounting screw hole locations
module tube_bracket_holes() {
    translate([0,0,+floor]) 
    for (angle=[45:90:360-epsilon])
        rotate([0,0,angle])
            translate([radius*0.8,0,0])
                children();
}


// Build bracket to hold axle to wood frame
module tube_bracket(tube_angles)
{
    intersection() {
        difference() {
            union() { // plus
                // Body
                cylinder(d1=2*radius,d2=tube_OD,h=0.7*radius);
                rotate(tube_angles) {
                    cylinder(d1=1.5*radius,d2=3*wall+tube_OD,h=radius);
                }
                
                // bosses around screw holes
                tube_bracket_holes() woodscrew_boss();
            }
            union() { // minus
                // main bore
                rotate(tube_angles) {
                    cylinder(d=tube_OD,h=3*radius,center=true,$fa=5);
                    
                }
                
                // space for screws
                tube_bracket_holes() woodscrew_head(1.5);
            }
        }
       
        // times half-sphere to trim all outside edges
        intersection() {
            sphere(r=radius,$fn=32);
            translate([-radius,-radius,0])
                cube([radius*2,radius*2,radius]);
        }
    }
}

tube_bracket([8,0,0]);
