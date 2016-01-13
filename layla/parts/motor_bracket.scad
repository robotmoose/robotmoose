// Peg Perego gearbox mounting adapter

include <layla_config.scad>;
$fa=5;

// Thickness of plate between axle and this bracket
plate_thick=19+clearance; // 0.75*25.4+clearance;

// X thickness of our mounting ears
x_ears=4.0;

// X distance from center of axle to this bracket
x_start=axle_dia/2+plate_thick;

// Y width of base plate, including mounting "ears"
width_ears=90;

// Y width of central region
width_neck=65;

// Z thickness
thick=15;

/* 
Motor projection database:
  (0,0) is location of axle
   [0]X left   [1]Y bot   [2]diam  [3]depth 
*/
holes=[
    [60.46, -19.84, 39.94, 110.0], // motor
    [49.8, -18.24, 8.25, 20.0], // micro peg
    [31.95,-29.46, 14, 20.0], // axle 1
    [41.32,5.87, 14.83, 20.0], // axle 2
];
n_holes=4;

module poke_hole(holei, add_radius=0.0, add_depth=0.0, scale_depth=1.0)
{
    h=holes[holei];
    diam=h[2];
    radius=diam/2;
    depth=add_depth+h[3]*scale_depth;
    translate([h[0]+radius,h[1]+radius,-depth])
        cylinder(d=2*(radius+clearance+wiggle+add_radius),h=depth);
}

module poke_motor_holes() {
    poke_hole(0);
    poke_hole(0,3.0, 2.5,0.0);
    poke_hole(1);
    poke_hole(1,2.0, 4.0,0.0);
    poke_hole(2);
    poke_hole(3);
  #  translate([53,8,-3])
        rotate([0,0,-20])
            cube([10,8,10]);
}

module motor_bracket_holes() {
    translate([x_start+x_ears,0,thick/2])
    rotate([0,90,0])
    for (side=[-1,+1])
        translate([0,side*(width_ears/2-woodscrew_head_dia/2),0])
            children();

}

difference() {
    union() {
        hull() {
            translate([x_start,-width_neck/2,0])
                cube([25,width_neck,thick]);
            translate([81,0,0])
                cylinder(d=50,h=thick);
        }
        
        translate([x_start,-width_ears/2,0])
            cube([x_ears,width_ears,thick]);
        
        intersection() {
            translate([x_start,-width_ears/2,0])
                cube([80,width_ears,thick]);
            union() {
                motor_bracket_holes() woodscrew_boss();
            }
        }
    }
    union() { // minus
        translate([0,0,thick+epsilon])
            poke_motor_holes();
        
       motor_bracket_holes() woodscrew_head(0.1);
    }

}

