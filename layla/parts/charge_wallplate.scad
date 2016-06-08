$fn=50;

include <layla_config.scad>;

wallplate_x=200; // dimensions of steel charge plates
wallplate_y=50; 
wallplate_midy=2; // half the separation between plates
wallplate_out=4; // outside perimeter beyond plexi
wallplate_in=2; // inside perimeter over steel

wallplate_plexi_z=2.5; // thickness of acrylic base
wallplate_steel_z=1.3; // thickness of steel contact plates
wallplate_over_z=3.0; // thickness of plastic over steel

wallplate_plexi_x=wallplate_x;
wallplate_plexi_y=2*wallplate_y+2*wallplate_midy;
echo("Plexi geometry: ",wallplate_plexi_x,wallplate_plexi_y);

// Charge plate geometry
module plates_2D(inset=0)
{
	for (sides=[-1,+1])
		scale([1,sides,1])
			translate([-wallplate_x/2,wallplate_midy])
				offset(r=-inset)
					square([wallplate_x,wallplate_y]);
}

// Plexiglass geometry
module plexi_2D()
{
	square([wallplate_plexi_x,wallplate_plexi_y],center=true);
}

module ymirror() {
	children();
	mirror([0,1,0]) children();
}

module wallplate_screwholes()
{
	for (side=[-1,+1])
		translate([side*(wallplate_plexi_x/2+0.5*woodscrew_head_dia),
			wallplate_plexi_y/2-woodscrew_head_dia/2,
				0])
				children();
}

module wallplate_wirehole() 
{
	translate([wallplate_plexi_x/2,0]) circle(d=40);
}

// Whole plate
module wallplate() {
	difference() {
		h=wallplate_over_z+wallplate_steel_z+wallplate_plexi_z;
		union() { // plus
			fillet=8;
			linear_extrude(height=h) 
				offset(r=-fillet) offset(r=+fillet) // round
				offset(r=wallplate_out) // fatten walls
				{
					plexi_2D();
					ymirror() hull() wallplate_screwholes() circle(d=woodscrew_head_dia);
					wallplate_wirehole();
				}
		}
		// minus:
		// Screw holes
		#ymirror() wallplate_screwholes() rotate([180,0,0]) woodscrew_head();
		
		big=20;
		// Wire hole
		translate([0,0,wallplate_over_z/2]) linear_extrude(height=big) wallplate_wirehole();
		
		// inset for plexiglass plate
		translate([0,0,wallplate_over_z+wallplate_steel_z]) {
			linear_extrude(height=big) plexi_2D();
		}
		
		// inset for steel plates
		translate([0,0,wallplate_over_z]) {
			linear_extrude(height=big,convexity=4) plates_2D(0.0);
		}
		
		// thru holes for charge plates
		translate([0,0,-epsilon]) {
			linear_extrude(height=big,convexity=4) plates_2D(wallplate_in);
		}
	}
}

wallplate();


