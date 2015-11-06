
// Height of platform above Create base
platform_ht=100;

// Thickness of platform plate
platform_thick=1.0;

// Additional width of platform plate around pipe base
platform_extra=4;

// Distance between pipe and pipewalls
pipe_clearance=0.5;

// Outside radius of pipe holes
pipe_OR=0.840*25.4/2+pipe_clearance;

// Wall thickness around pipes
pipe_wall=1.3;

// Wall thickness for tips of peg legs
peg_wall=2.3;

// Half-width (X) of platform, to pipe centerlines
xwid=112;

ribspacing=xwid/4;
ribheight=pipe_OR*0.8;
ribwidth=platform_thick;

// Y bottom and top of platform
ybot=-78;
ytop=84;

// Thin layer to allow bridge across holes
membrane=0.4;

// Avoid roundoff in CSG
epsilon=0.01;


// Offset screw holes in Create 2 top.  
//   M3 or #4-40 work here, or short pointy wood screws
module create2_screw_holes() {
    for (holes=[
        [108.6,87.55],
        [115.11,-78.8],
    ])
    {
        for (side=[-1,+1])
            translate([holes[0]*side,holes[1]])
                children();
    }
}

// XY plane pipe centerlines
module create2_pipe_centers() {
    translate([+xwid,ytop]) rotate([0,0,  0]) children();
    translate([-xwid,ytop]) rotate([0,0, 90]) children();
    translate([-xwid,ybot]) rotate([0,0,180]) children();
    translate([+xwid,ybot]) rotate([0,0,270]) children();
}

// 2D rectangle utility
module rect_lrbt(l,r,b,t) {
    polygon(points=[
        [r,t],
        [l,t],
        [l,b],
        [r,b]
    ]);
}

// 2D outline of base plate, to pipe centerlines
module create2_plate2D() {
    rect_lrbt(-xwid,+xwid,ybot,ytop);
}

module create2_plate3D() {
    union(); {
        linear_extrude(convexity=2,height=platform_thick)
            offset(r=platform_extra)
                create2_plate2D();
        
        for (xrib=[-xwid+ribspacing:ribspacing:xwid-ribspacing])
            linear_extrude(convexity=2,height=ribheight)
                rect_lrbt(xrib-ribwidth/2,xrib+ribwidth/2,ybot,ytop);
        
        for (yrib=[ybot+ribspacing:ribspacing:ytop-ribspacing])
            linear_extrude(convexity=2,height=ribheight)
                rect_lrbt(-xwid,+xwid,yrib-ribwidth/2,yrib+ribwidth/2);
    }
}

// Overall pipe superstructure: 
//    ER is Extra Radius (for forming walls)
//    EH is Extra Height (for poking thru holes)
module create2_pipewerx(ER, EH) {
    r=pipe_OR+ER;
    union() {
        create2_pipe_centers() {
            translate([0,0,-EH*epsilon])
                cylinder(r=r,h=platform_ht+2*EH*epsilon);
        }
        rotate([0,-90,0]) 
        union() {
            translate([pipe_OR+pipe_wall,ybot,0])
                cylinder(r=r,h=2*xwid+EH,center=true);
            translate([pipe_OR+pipe_wall,ytop,0])
                cylinder(r=r,h=2*xwid+EH,center=true);
        }
        rotate([90,0,0]) 
        union() {
            translate([-xwid,pipe_OR+pipe_wall,-(ytop+ybot)/2])
                cylinder(r=r,h=ytop-ybot+EH,center=true);
            translate([+xwid,pipe_OR+pipe_wall,-(ytop+ybot)/2])
                cylinder(r=r,h=ytop-ybot+EH,center=true);
        }
    }
}

// XY plane reinforcing bracket for corners
module create2_corner_bracket2D(scale) {
    rebar=0.8*scale*(platform_ht-pipe_OR-pipe_wall);
    clearcen=rebar*1.2;
    
    wiringholeR=3.5;
    wiringhole=rebar*0.25;
    difference() {
        // Triangle main body
        polygon([
            [0,0],
            [rebar,0],
            [0,rebar]
        ]);
        
        
        // Main center curve
        translate([clearcen,clearcen])
        circle(r=clearcen,$fn=64);
        
        // Wiring hole
        translate([wiringhole,wiringhole])
        circle(r=wiringholeR);
    }
}


// XY plane reinforcing bracket for corners
module create2_corner_bracket3D(scale) {
    linear_extrude(height=pipe_wall,convexity=2)
        create2_corner_bracket2D(scale);
    
    // Little lip to reinforce edge of bracket
    linear_extrude(height=pipe_wall*2,convexity=2)
    difference()
    {
        create2_corner_bracket2D(scale);
        offset(r=-pipe_wall) create2_corner_bracket2D(scale);
    }
}

// Cylindrical footpeg for stand.  Extends in minus Z direction
module create2_footpeg() {
    headR=5.5; // mounting bolt head radius
    cylinderR=2*pipe_OR-headR; // outside of cylinder
    scale([1,1,-1]) 
    cylinder(r1=headR,r2=headR+cylinderR,h=1.2*cylinderR);
}

// Outside of footpeg, used to trim down the pipe walls
module create2_footpeg_outside() {
    z=pipe_OR*1.5;
    difference() {
        translate([0,0,-z])
            cylinder(r=1.8*pipe_OR,h=1.2*z,$fn=6);
        create2_footpeg();
    }
}
// Outside of footpeg, used to trim down the pipe walls
module create2_footpegs_outside() {
     create2_screw_holes() {
         translate([0,0,platform_ht])
            create2_footpeg_outside();
    }
}

// Bottom of "feet"
module create2_footpegs() {    
  intersection() {
    create2_pipewerx(pipe_wall/2,0);
        
    translate([0,0,platform_ht-peg_wall])
    difference() {
        create2_screw_holes() union() {
            // Tapering internal support cylinders
            difference() {
                translate([0,0,peg_wall])
                    create2_footpeg();
                create2_footpeg();
            }
        }
        
        create2_screw_holes() {
            // Actual mounting bolts
            translate([0,0,+membrane])
                cylinder(r=1.7, h=10, $fn=10);
        }  
    }
  }
}

include <arduino.scad>

// Arudino Mega bracket
module create2_arduino() {
    mega_dx=10;
    mega_dy=0;
    mega_wid=105;
    mega_ht=60;
    wall_wid=mega_wid+15;
    wall_ht=mega_ht+15;
    translate([-wall_wid,0,0])
    difference() {
        // cube(size=[wall_wid,wall_ht,pipe_wall]);
        difference() {
            // Outside plate:
            cube(size=[wall_wid,wall_ht,3*pipe_wall]);
            
            // Inside hollow:
            translate([+pipe_wall,-pipe_wall,pipe_wall])
            cube(size=[wall_wid,wall_ht,3*pipe_wall]);
            
        }
        
        // Mounting holes, for M3 screws
        translate([-pipe_OR+mega_dx,pipe_OR+mega_dy+mega_ht,-epsilon])
        rotate([0,0,-90])
        holePlacement(boardType=MEGA2560) {
            cylinder(r=1.3, h=2*pipe_wall);
        }
        
        // Lightening hole
        translate([wall_wid/2,wall_ht/2,-epsilon])
            cylinder(r=25,h=2*pipe_wall);
    }
}


// The entire support platform
module create2_platform() {
    union() {
    difference() {
        union() { // overall plus
            create2_plate3D();
            
            // Pipe outsides
            create2_pipewerx(pipe_wall,0.0);
            
            // Corner reinforcing
            create2_pipe_centers() {
                for (front=[-1,+1]) 
                for (side=[0,1]) rotate([0,0,90*side])
                    translate([front*(pipe_OR+pipe_wall),0,pipe_OR+pipe_wall])
                    rotate([90,0,-90])
                    scale([(1-2*side),1,front])
                        create2_corner_bracket3D(0.8+0.2*front);
                    
            }
            
        // Add arduino mounting bracket
            translate([xwid,ybot+pipe_OR+pipe_wall,pipe_OR])
                rotate([90,0,0])
                    create2_arduino();
        }
        union() { // overall minus
            // Holes for pipes
            create2_pipewerx(0.0,pipe_OR*10);
            
            // Corner clearance
            corner_clear=4*pipe_OR;
            create2_pipe_centers() 
            {
                translate([corner_clear*0.6,corner_clear*0.6,0])
                    sphere(r=corner_clear);
            }

            // Trim top of feet
            create2_footpegs_outside();
        }
    }
    // Add footpegs
        create2_footpegs();
    
    
    }
}

intersection() {
    create2_platform();

    // cross section
    //translate([-xwid,-(ytop-ybot)/2,pipe_OR*1.5])
    //    cube(size=[2*xwid,(ytop-ybot),platform_ht]);
    
    // full top half:
    //translate([-2*xwid,-2*(ytop-ybot)/2,pipe_OR*1.4])
    //    cube(size=[4*xwid,2*(ytop-ybot),platform_ht]);
    
    // One corner only
    //translate([0.7*xwid,0.65*(ytop-ybot)/2,0])
    //    cube(size=[4*xwid,2*(ytop-ybot),platform_ht]);
}

//create2_footpegs();
// import("irobot_cover_v4.stl");
