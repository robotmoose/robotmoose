sigma=0.1;

base_thickness=2;
base_diameter=42;

neck_height=24;
neck_width=9;
neck_thickness=8;
neck_offset=20;
roundness=3;

horn_width=7;
horn_length=20;
horn_depth=0.5;
horn_yfudge=1.5;
horn_zfudge=1;

union()
{
    difference()
    {
        chop=5;

        //Base
        cylinder(d=base_diameter,h=base_thickness+neck_height,$fn=50);
        
        translate([0,neck_offset,base_thickness])
        {
            //Cut Walls
            translate([0,-base_diameter/2-sigma,0])
            {
                edge_to_out_wall=(base_diameter-neck_width)/2;
                translate([base_diameter/2-edge_to_out_wall+sigma,0,0])
                    round_cube(size=[edge_to_out_wall+chop+sigma,
                        base_diameter+sigma*2,
                        neck_height+chop+sigma],
                        radius=roundness);
                translate([-base_diameter/2-chop-sigma,0,0])
                    round_cube(size=[edge_to_out_wall+chop+sigma,
                        base_diameter+sigma*2,
                        neck_height+chop+sigma],
                        radius=roundness);
            }
            translate([-base_diameter/2,0,0])
            {
                cube_width=(base_diameter-neck_thickness)/2+sigma;
                translate([0,neck_thickness/2+sigma,0])
                    round_cube(size=[base_diameter,
                        cube_width,
                        neck_height+chop+sigma],
                        rot_z=true,
                        radius=roundness);
                translate([0,-base_diameter/2-neck_offset-sigma,0])
                    round_cube(size=[base_diameter,
                        cube_width+neck_offset,
                        neck_height+chop+sigma],
                        rot_z=true,
                        radius=roundness);
            }
        }

        //Cut Servo Horn
        translate([-horn_width/2,
            neck_offset-neck_thickness/2+horn_yfudge-sigma,
            base_thickness+neck_height-horn_length-horn_zfudge])
            cube(size=[horn_width,horn_depth+sigma,horn_length]);
    }
}

module round_cube(size,radius=1,rot_z=false)
{
	$fn=50;
    
    if(rot_z)
    {
        x=size[1]-radius/2;
        y=size[2]-radius/2;
        z=size[0];

        translate([0,0,radius])
            rotate([90,0,90])
                minkowski()
                {
                    cube(size=[x,y,z]);
                    cylinder(r=radius);
                }
    }
    else
    {
        x=size[2]-radius*2;
        y=size[0]-radius*2;
        z=size[1];

        translate([radius,z+radius-radius/2,x+radius])
            rotate([90,90,0])
                minkowski()
                {
                    cube(size=[x,y,z]);
                    cylinder(r=radius);
                }
    }
}