sigma=0.1;

base_thickness=5;
base_diameter=42;

servo_width=13;
servo_offset=4;
servo_bracket_height=5;
servo_bracket_thickness=3;
servo_edge=4;

wall_height=4;
wall_thickness=3;
wall_width=18;

horn_width=7;
horn_length=32;
horn_depth=1;

cable_manager_walls=2;
cable_manager_slit=2.3;
cable_manager_channel=1.5;
cable_manager_width=6;
cable_manager_height=2;

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
            translate([-servo_width/2,-base_diameter/2+servo_edge,0])
                cube(size=[servo_width,
                    base_diameter-servo_edge,
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
                        cube_width,
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
    }

    //Cable Manager
    translate([-(cable_manager_width+cable_manager_walls)/2,base_diameter/2,0])
    {
        difference()
        {
            translate([0,-1,0])
                cube([cable_manager_walls+cable_manager_width,
                    cable_manager_walls*2+
                    cable_manager_slit+1,
                    cable_manager_height]);
            
            translate([0,cable_manager_walls,-sigma])
            {
                translate([cable_manager_walls,0,0])
                    cube([cable_manager_width-cable_manager_walls,
                        cable_manager_slit,
                        cable_manager_height+sigma*2]);
                translate([-sigma,0,0])
                    cube([cable_manager_width+sigma,
                        cable_manager_channel,
                        cable_manager_height+sigma*2]);
            }
        }
    }
}

module round_cube(size,radius=1,rot_z=false)
{
	$fn=50;
    
    if(rot_z)
    {
        x=size[1]-radius/2;
        y=size[2];
        z=size[0]-radius/2;

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


























/*difference()
{
    union()
    {
        //Base
        cylinder(d=base_diameter,h=base_thickness,$fn=100);

        //Walls
        translate([servo_width/2,-wall_width/2,base_thickness])
            cube(size=[wall_thickness,wall_width,wall_height]);
        translate([-servo_width/2-wall_thickness,-wall_width/2,base_thickness])
            cube(size=[wall_thickness,wall_width,wall_height]);

        //Cable Manager
        translate([-(cable_manager_width+cable_manager_walls)/2,base_diameter/2,0])
        {
            difference()
            {
                translate([0,-1,0])
                    cube(size=[cable_manager_walls+cable_manager_width,
                        cable_manager_walls*2+cable_manager_slit+1,base_thickness]);
                
                translate([0,cable_manager_walls,-sigma])
                {
                    translate([cable_manager_walls,0,0])
                        cube(size=[cable_manager_width-cable_manager_walls,
                            cable_manager_slit,base_thickness+sigma*2]);
                    translate([-sigma,0,0])
                        cube(size=[cable_manager_width+sigma,cable_manager_channel,
                            base_thickness+sigma*2]);
                }
            }
        }
    }

    //Servo Horn
    translate([-horn_width/2,-horn_length/2,-sigma])
        cube([horn_width,horn_length,horn_depth]);
}*/