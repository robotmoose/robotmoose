
// Quizbot face mount

clear=0.4;
qb_width=16+2*clear;
qb_deep=4+2*clear;
qb_high=14+2*clear;
wall=0.8; // mm wall thickness
module facemount() {
    union() {
        difference() {
            translate([0,0,-wall])
                cube(size=[qb_width+2*wall,qb_deep+2*wall,qb_high],center=true);
            translate([0,0,0])
           #     cube(size=[qb_width,qb_deep,qb_high],center=true);
            
            translate([0,0,qb_high/2])
                rotate([90,0,0])
                    cylinder(r=6,h=20);
        }
    }
}

//for (sh=[0,1,2,3])
//    translate([sh*40,0,0])
        facemount();
