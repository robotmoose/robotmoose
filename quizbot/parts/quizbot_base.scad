sigma=0.1;

base_thickness=5;
base_diameter=42;

center_fudge=2;

servo_width=13;
servo_offset=2;
servo_bracket_height=5;
servo_bracket_thickness=3;
servo_edge=4;

wall_height=4;
wall_thickness=3;
wall_width=16;

horn_width=7;
horn_length=32;
horn_depth=1;

cable_manager_width=12;
cable_manager_height=4;
cable_manager_depth=20;
cable_manager_zoff=2;

union()
{
    difference()
    {
        chop=5;

        //Base
        cylinder(d=base_diameter,h=base_thickness+wall_height,$fn=50);
        
        translate([0,0,base_thickness])
        {
            //Cut Center
            translate([-servo_width/2,-base_diameter/2+servo_edge+1,0])
                cube(size=[servo_width,
                    base_diameter-servo_edge*2-center_fudge,
                    wall_height+chop+sigma]);

            //Cut Walls
            translate([0,-base_diameter/2-sigma,0])
            {
                edge_to_out_wall=(base_diameter-servo_width)/2-wall_thickness;
                translate([base_diameter/2-edge_to_out_wall+sigma,0,0])
                    round_cube(size=[edge_to_out_wall+chop+sigma,
                        base_diameter+sigma*2,
                        wall_height+chop+sigma]);
                translate([-base_diameter/2-chop-sigma,0,0])
                    round_cube(size=[edge_to_out_wall+chop+sigma,
                        base_diameter+sigma*2,
                        wall_height+chop+sigma]);
            }
            translate([-base_diameter/2,0,0])
            {
                cube_width=(base_diameter-wall_width)/2+sigma;
                translate([0,wall_width/2+sigma,0])
                    round_cube(size=[base_diameter,
                        cube_width-servo_edge,
                        wall_height+chop+sigma],
                        rot_z=true);
                translate([0,-base_diameter/2-sigma+servo_edge,0])
                    round_cube(size=[base_diameter,
                        cube_width-servo_edge,
                        wall_height+chop+sigma],
                        rot_z=true);
            }
        }

        //Cut Servo Hole
        translate([-servo_width/2,
            -servo_bracket_thickness/2-servo_offset,
            base_thickness-servo_bracket_height+sigma])
            cube(size=[servo_width,servo_bracket_thickness+sigma,servo_bracket_height]);

        //Servo Horn
        translate([-horn_length/2,-horn_width/2,-sigma])
            cube(size=[horn_length,horn_width,horn_depth]);
        
        //Cut Cable Management Hole
        translate([-cable_manager_width/2,
            base_diameter/2-cable_manager_depth/2,
            base_thickness-cable_manager_zoff])
            round_cube([cable_manager_width,cable_manager_depth,cable_manager_height],rot_z=true);
    }
}

module round_cube(size,radius=1,rot_z=false)
{
	$fn=50;
    
    if(rot_z)
    {
        x=size[1]-radius*2;
        y=size[2]-radius*2;
        z=size[0]-radius;

        translate([0,radius,radius])
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
        z=size[1]-radius;

        translate([radius,z+radius,x+radius])
            rotate([90,90,0])
                minkowski()
                {
                    cube(size=[x,y,z]);
                    cylinder(r=radius);
                }
    }
}