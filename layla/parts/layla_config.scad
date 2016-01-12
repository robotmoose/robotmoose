// Main parameters used for most Layla parts
$fa=15;
$fs=0.1;

// This number avoids roundoff problems
epsilon=0.02;

// This is how much your printer squishes extra around holes
//   bigger value leaves more space for globs.
clearance=0.4;

// Removeable parts leave this much extra diameter
wiggle=0.2;

// Wall thickness for most parts--two nozzle diameters typ.
wall=0.8;

// Floor thickness--at least 4 layers
floor=1.2;


// This is the main drive axle
axle_dia=0.375*25.4+2*clearance;

// Main frame tubing
tube_OD=23.5+2*clearance+wiggle;


// Main woodscrews: Everbilt #8x9/16" lath screws
woodscrew_head_dia=12.5;
woodscrew_shaft_OD=4.5;
woodscrew_shaft_ID=3.0;
woodscrew_shaft_len=20.0;
woodscrew_boss=1.0; // extra thickness under screw head

// Clearance for woodscrew to pass through (minus)
module woodscrew_head(head_clear_scale=1.0) 
{
	head_clear=0.5*floor+epsilon+head_clear_scale*10;
    translate([0,0,woodscrew_boss]) {
        // Head clearance
        cylinder(d1=woodscrew_head_dia,d2=woodscrew_head_dia+head_clear,h=head_clear);
        
        // Through clearance for shaft of screw
        translate([0,0,+epsilon])
        scale([1,1,-1])
            cylinder(d=woodscrew_shaft_OD,h=woodscrew_shaft_len);
    }
}

// Raised area reinforcing woodscrew (plus)
module woodscrew_boss()
{
    boss_ht=1.5*floor+woodscrew_boss;
    translate([0,0,-floor])
        cylinder(d1=woodscrew_head_dia*1.8,d2=woodscrew_head_dia*1.3,h=boss_ht);
}

/*
difference() {
    woodscrew_boss();
    woodscrew_head();
}
*/


