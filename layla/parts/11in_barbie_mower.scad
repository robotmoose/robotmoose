module round_cube(size,radius=1)
{
	$fn=50;
	x=size[2]-radius*2;
	y=size[0]-radius*2;
	z=size[1]-radius*2;

	translate([radius,z+radius*2,x+radius])
	rotate([90,90,0])
	minkowski()
	{
		cube(size=[x,y,z]);
		cylinder(r=radius);
	}
}

//Rounded primitives for openscad
//(c) 2013 Wouter Robers 
module round_cylinder(d=10,h=10,roundness=0.5)
{
	$fn=50;

	if(roundness==0)
	{
		cylinder(d=d,h=h);
	}
	else
	{
		translate([0,0,0])
			hull()
			{
				rotate_extrude()
					translate([d/2-roundness,roundness,0])
					circle(r=roundness);

				rotate_extrude()
					translate([d/2-roundness,h-roundness,0])
					circle(r=roundness);
			}
	}
}

module half_sphere(d=20,h=100)
{
	scale([1,1,2*h/d])
	{
		difference()
		{
			sphere(d=d);
			translate([-d/2,-d/2,-d])
				cube(size=[d,d,d]);
		}
	}
}

module 11in_mower(spline_width=3.5,inner_diameter=36,
	teeth_thickness=12,teeth_num=10,teeth_height=10,roundness=1,floor=2)
{
	//actual spline_width is 2.5
	//actual inner_diameter is 35
	
	translate([0,0,-teeth_height-floor])
		difference()
		{
			cylinder(d=inner_diameter+teeth_thickness*2,h=teeth_height+floor);

			round_cylinder(d=inner_diameter,h=teeth_height,
				roundness=roundness);

			translate([0,0,-teeth_height/2])
				cylinder(d=inner_diameter,h=teeth_height);

			for(ii=[0:teeth_num])
				rotate(360/teeth_num*ii)
					translate([-spline_width/2,0,-roundness*2])
						round_cube(size=[spline_width,
							inner_diameter/2+teeth_thickness,
							teeth_height+roundness*2],
							radius=roundness);
		}
}

module barbie_adapter(channel_length=50,channel_width=13.3,channel_height=18,
	center_cut_diameter=32,roundness=1,wall=5,floor=2,wiggle=0.75,channel_overcut=0)
{
	difference()
	{
		half_sphere(d=channel_length+wall*2,h=channel_height+floor);
		translate([0,0,floor])
			union()
			{
				for(ii=[0:2])
					rotate(90*ii)
						translate([-(channel_width+wiggle)/2,
							-(channel_length+wiggle+channel_overcut)/2,0])
								round_cube(size=[channel_width+wiggle,
									channel_length+wiggle+channel_overcut,
									channel_width+wiggle*4],
									radius=roundness);
				round_cylinder(d=center_cut_diameter+wiggle,h=channel_height,
					roundness=roundness);
			}
	}
}

module 11in_barbie_mower()
{
	difference()
	{
		translate([0,0,12])
		{
			barbie_adapter();
			11in_mower();
		}
		
		cylinder(d=16,h=30);
	}
}

11in_barbie_mower();
difference()
{
    cylinder(d=18,h=9.7);
    
    translate([0,0,-1])
        cylinder(d=16,h=11.7);
}