$fn=50;
hole_centers=68;
hole_diameter=3.5;
mount_board_thickness=18;
bracket_to_hole_centers=30-mount_board_thickness;
walls=6;
hole_depth=30;
mount_hole_diameter=6;
cube_hole_size=[20,hole_depth,hole_diameter+1];

block_size=[hole_centers+walls*4,10,bracket_to_hole_centers];
mount_block_size=[block_size.x+walls*2+mount_hole_diameter*2,block_size.y,6];
 
difference()
{
    union()
    {
        translate([-block_size.x/2,-block_size.y/2,-walls])
            cube(size=block_size);
        translate([-mount_block_size.x/2,-mount_block_size.y/2,0])
            cube(size=mount_block_size);
    }
    
   rotate([90,0,0]) 
    {
        translate([hole_centers/2,0,-hole_depth/2])
            scale([5,1,1])
                cylinder(d=hole_diameter,h=hole_depth);
        translate([-hole_centers/2,0,-hole_depth/2])
            scale([5,1,1]) 
                cylinder(d=hole_diameter,h=hole_depth);
 }
    
    /* translate([hole_centers/2-cube_hole_size.x/2-4,-hole_depth/2,-cube_hole_size.z/2])
        cube(size=cube_hole_size);
        translate([-hole_centers/2-cube_hole_size.x/2+4,-hole_depth/2,-cube_hole_size.z/2])
        cube(size=cube_hole_size); */
    
   translate([-block_size.x/2-walls,0,-hole_depth/2])
    {
        translate([block_size.x+walls*2,0,0])
            cylinder(d=mount_hole_diameter,h=hole_depth);
        cylinder(d=mount_hole_diameter,h=hole_depth);
    }
   
    
    /*translate([-hole_centers/2-cube_hole_size.x/2+4,hole_depth/2,0])
        rotate([90,0,0])
        {
            translate([hole_centers,0,0])
                scale([2,1,1]) cylinder(d=hole_diameter,h=hole_depth);
            scale([2,1,1]) cylinder(d=hole_diameter,h=hole_depth);
        } */ 
   
    
    /*translate([-block_size.x/2+walls,hole_depth/2,0])
        rotate([90,0,0])
        {
            translate([hole_centers,0,0])
                cylinder(d=hole_diameter,h=hole_depth);
            cylinder(d=hole_diameter,h=hole_depth);
        }*/
}