// Ridiculously complex bracket to hold a chunk of EMT tubing
include <layla_config.scad>;

// Size of floorplate of bracket
radius=75;

// Size of hole for retaining the tubing (quick detach)
quickdetach_dia=0.25*25.4+3*clearance;
quickdetach_z=radius*0.75;

// Bracket mounting screw hole locations
module tube_bracket_holes() {
    translate([0,0,+floor]) 
    for (angle=[45:90:360-epsilon])
        rotate([0,0,angle])
            translate([radius*0.8,0,0])
                children();
}

// A fin is a 2D outline standing out from the tube
//  for mechanical reinforcing.
module tube_fin_2D() {
    difference() {
        // outside of fin
        polygon([
            [tube_OD/2,radius],
            [tube_OD/2+wall,radius],
            [0.85*radius,0],
            [radius,-radius],
            [tube_OD/2,-radius]
            ]);
        
        // lightening hole
        translate([(radius-tube_OD/2)*0.32+tube_OD/2,radius*0.27])
            circle(r=0.16*radius);
    }
}

// This is the 3D fin
module tube_fin_3D(fin_angle) {
    finboss=3.5*wall;
    rotate([90,0,fin_angle]) // stand up
    union() {
        // main plate of fin
        linear_extrude(height=wall,convexity=4,center=true)
            offset(r=-epsilon) tube_fin_2D();
        
        // thicker rim of fin
        linear_extrude(height=finboss,convexity=6,center=true) {
            difference() {
                tube_fin_2D();
                offset(r=-finboss) tube_fin_2D();
            }
        }
    }
}

// Build bracket to hold axle to wood frame
module tube_bracket(tube_angles)
{
    intersection() {
        difference() {
            union() { // plus
                // flat floorplate
                hull() 
                    tube_bracket_holes()
                        translate([0,0,-floor])
                        cylinder(h=floor,d=woodscrew_head_dia*2.2);
                
                // tube wall and fins
                rotate(tube_angles) {
                    // main tube
                    cylinder(d=tube_OD+2*wall,h=3*radius,center=true,$fa=5);
                    
                    // quickdetach boss
                    translate([0,0,quickdetach_z])
                      rotate([90,0,0])
                        cylinder(d=quickdetach_dia+5*wall,h=tube_OD+5*wall,center=true);
                    
                    // reinforcing tubelets
                    tubelet_h=5*wall;
                    for (tubelet=[0:0.25:1.0])
                        translate([0,0,tubelet*(radius-tubelet_h)])
                            cylinder(d=tube_OD+6*wall,h=tubelet_h);
                    
                    // Fin array
                    fin_del=9; // angle change around screw hole
                    for (fin_angle=[45:90:360-epsilon]) {
                        tube_fin_3D(fin_angle-fin_del); // left
                        tube_fin_3D(fin_angle+fin_del); // right
                    }
                }
                
                // bosses around screw holes
                tube_bracket_holes() woodscrew_boss();
            }
            union() { // minus
                // main bore
                rotate(tube_angles) {
                    cylinder(d=tube_OD,h=3*radius,center=true,$fa=5);
                    
                    // quickdetach hole
                    translate([0,0,quickdetach_z])
                      rotate([90,0,0])
                        cylinder(d=quickdetach_dia,h=2*tube_OD,center=true);
                }
                
                // space for screws
                tube_bracket_holes() woodscrew_head(0.1);
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
