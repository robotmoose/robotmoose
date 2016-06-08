// Simpler cone-shaped bracket to hold a chunk of EMT tubing
include <layla_config.scad>;

$fa=3;

// Size of floorplate of bracket (center to outside tip)
radius=50;

// Thickness of base of bracket
floor=3;

// Bracket mounting screw hole locations
module tube_bracket_holes() {
    translate([0,0,+floor]) 
    for (angle=[45:90:360-epsilon])
        rotate([0,0,angle])
            translate([radius*0.85,0,0])
                children();
}


metalscrew_center=[tube_OD/2+wall,0,0.75*radius];

// Build bracket to hold axle to wood frame
module tube_bracket(tube_angles)
{
    intersection() {
        difference() {
            union() { // plus
                // Body
                cylinder(d1=2*radius,d2=tube_OD,h=0.7*radius);
                rotate(tube_angles) {
                    cylinder(d1=1.5*radius,d2=2*wall+tube_OD,h=radius);
					//translate(metalscrew_center) rotate([0,90,0]) woodscrew_boss();
                }
                
                // bosses around screw holes
                tube_bracket_holes() woodscrew_boss();
            }
            union() { // minus
                // main bore
                rotate(tube_angles) {
                    cylinder(d=tube_OD,h=3*radius,center=true,$fa=5);
                    
					#translate(metalscrew_center) rotate([0,90,0]) woodscrew_head(1.5);
                }
                
                // space for screws
                tube_bracket_holes() woodscrew_head(1.5);
                
                // Lightening scoop-outs
                translate([0,0,-epsilon])
                for (angle=[0:90:360-epsilon])
                    rotate([0,0,angle])
                    translate([radius*1.2,0,0])
                        cylinder(r=radius*0.60,h=radius);
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

tube_bracket([0,0,0]);
for (more=[0:1])
    translate([radius*(2+2*more),0,0])
		scale([more*2-1,1,1])  // flip left and right
			tube_bracket([11,0,0]);

