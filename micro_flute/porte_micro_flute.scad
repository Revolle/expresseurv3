$fn=100;
e=5;
micro_d = 8 ;

//soprano yamaha
/*
flute_d = 27.8 ;
flute_d2 = 4 ;
flute_h=8; 
flute_e = flute_d2+ 3 ;
flute_micro_x = micro_d + 3 ;
flute_micro_y=micro_d + 3;
flute_micro_z = micro_d + 3 ;
flute_sifflet= 11 ;
flute_d_sifflet= 4 ;
*/

// sopranino Aullos
flute_d = 25.3;
flute_d2 = 4 ;
flute_h=8; 
flute_e = flute_d2+ 3 ;
flute_micro_x = micro_d + 3 ;
flute_micro_y=micro_d + 3;
flute_micro_z = micro_d + 3 ;
flute_sifflet= 11 ;
flute_d_sifflet= 4 ;

flute_m();

module flute_m()
{
    difference()
    {
        hull()
        {
            // bague
            cylinder(d=flute_d+flute_e,h=flute_h);
            // porte-micro
            translate([-flute_sifflet-flute_d_sifflet-1,flute_d/2+flute_micro_y/2-1.5,-micro_d])
                cube([flute_micro_x,flute_micro_y,flute_micro_z],center=true);
        }
        // trou flute
        cylinder(d=flute_d,h=800,center=true);
        // bague flute
        difference()
        {
            translate([0,0,flute_d2/2+0.5  ])
                rotate_extrude(convexity = 10)
                    translate([flute_d/2, 0, 0])
                        circle(d = flute_d2);
            translate([0,-50,0])
                cube([2,100,100],center=true);
        }
        // ouverture sifflet  
        translate([0,50,0])
            cube([flute_sifflet+flute_d_sifflet,100,100],center=true);
        // trou micro
        translate([-15,flute_d/2 + micro_d/2,-micro_d])
            rotate([0,90,0])
                cylinder(d=micro_d,h=30,center=true);    
        translate([-15,flute_d/2 + micro_d/2,-micro_d])
            rotate([0,90,0])
                cylinder(d=micro_d+0.2,h=3,center=true);    
     }
     // simulation flute
     *difference()
     {
        cylinder(d=flute_d,h=50,center=true);
        translate([0,flute_d/2,-flute_h-3])
            rotate([10,0,0])
                cube([flute_sifflet, 5,2*flute_sifflet],center = true);
     }


}
