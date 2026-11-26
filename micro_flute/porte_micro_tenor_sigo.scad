$fn=100;
e=3 ;

micro_d= 8 ;
micro_l=14 ;

tenor_x=31.6 ;
tenor_y=10 ;
tenor_z=15;
tenor_d=45 ;
tenor_dz = sqrt(tenor_d*tenor_d/4 - tenor_x*tenor_x/4) ;
tenor_arr_d=3;
tenor_sifflet_x=18;
tenor_micro_dz = sqrt(tenor_d*tenor_d/4 - tenor_sifflet_x*tenor_sifflet_x/4) ;

module sifflet()
{
    // cube central
    cube([tenor_x,tenor_y,tenor_z],center = true) ;
    difference()
    {
        //bombe superieur
        translate([0,0,tenor_z/2-tenor_dz])
            rotate([90,0,0])
                cylinder(d= tenor_d ,h=tenor_y,center = true);
        translate([50+tenor_x/2,0,0])
            cube([100,100,100],center=true);
        translate([-50-tenor_x/2,0,0])
            cube([100,100,100],center=true);
        translate([0,0,-50])
            cube([100,100,100],center=true);
    }
}
module porte_micro()
{
    difference()
    {
        union()
        {
            // boitier
            scale([1.2,0.98,1.2])
                sifflet();
            hull()
            {
                difference()
                {
                    // boitier
                    scale([1.2,0.98,1.2])
                        sifflet();
                    translate([-50+tenor_sifflet_x/2,0,0])
                        cube([100,100,100],center=true);
                }
                //porte_micro
                translate([tenor_sifflet_x/2+micro_l/2,-tenor_y/2-micro_d/2-e/2,tenor_z/2+micro_d/2+e/2+tenor_micro_dz-tenor_dz])
                    cube([micro_l,micro_d+e,micro_d+e],center=true);
            }
        }
        // moins la flute
        scale([1.0,4.0,1.0])
            sifflet();
        translate([0,0,-tenor_z/2-50])
            cube([100,100,100],center=true);
        // trou micro
        translate([tenor_sifflet_x/2+micro_l/2,-tenor_y/2-micro_d/2-e/2,tenor_z/2+micro_d/2+e/2+tenor_micro_dz-tenor_dz])
            rotate([0,90,0])
                cylinder(d=micro_d, h = micro_l+10, center = true );
    // teton antiglissement
        translate([0,0,-tenor_z/2+e])
            rotate([0,90,0])
                cylinder(d=e,h=50,center=true);
    }
}


*sifflet() ;
difference()
{
    porte_micro() ;
    translate([0,-100-tenor_y/2,0])
        cube([200,200,200],center = true );
}
