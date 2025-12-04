$fn = 50 ;
e=2;

// diametre vis
vis_d = 1.8 ;
vis_l = 3 ;
vis_tete_d = 4.5 ;
vis_tete_l=2 ;

cable_d = 3.0 ;
cable_usb_d = 3.3 ;

teensy_40_x = 18.1 ; // 17.78 ;
teensy_40_y = 35.7 ; // 35.56 ;
teensy_40_e = 1.8 ; // 1.57 ;

usb_x = 12 ;
usb_y = 40 ;
usb_z = 7 ;
usb_dz = 2.5/2+ teensy_40_e ;

teensy_40_z = max(3.07 + teensy_40_e , usb_dz + usb_z / 2 );

teensy_40_led_dx = -4 ;
teensy_40_led_dy = teensy_40_y / 2 - 4 ;
teensy_40_led_x = 5 ;
teensy_40_led_y = 5 ;

serre_l = 3.5 ;
serre_e = 1.25 ;
serre_tete_e = 5.5 ;

evidement_y = serre_l+vis_tete_d + usb_y ;

// boitier
boitier_x = teensy_40_x + e ;
boitier_y = teensy_40_y  + evidement_y ;
boitier_z = cable_d + 1 + teensy_40_z ;
echo("boitier_z=",boitier_z);
teensy_40_dy = -(boitier_y - teensy_40_y)/2 ;
teensy_40_dz = -3 ;

couvercle_x = boitier_x + e ;

cale_e = e + vis_d ;
fermeture_d=1 ;
patte_dy = 15;


module vis()
{
    translate([0,0,-vis_tete_l/2])
        cylinder(d1=vis_d,d2=vis_tete_d,h=vis_tete_l,center=true);
    translate([0,0,-vis_l/2-vis_tete_l])
        cylinder(d=vis_d,h=vis_l,center=true);
}
module teensy_40()
{
    // Plaque
    translate([0,0,teensy_40_e/2])
        cube([teensy_40_x, teensy_40_y, teensy_40_e],center = true );
     //USB
    translate([0,-teensy_40_y/2 - usb_y / 2 + e , usb_dz])
        cube([usb_x,usb_y, usb_z],center = true );
    // led
    translate([teensy_40_led_dx,teensy_40_led_dy,(teensy_40_z + e)/ 2 + teensy_40_e])
        cube([teensy_40_led_x,teensy_40_led_y, teensy_40_z + e ],center = true );
}
module patte(ee)
{
    difference()
    {
        cube([2*e,ee,2*e],center=true);
        cylinder(d=vis_d,h=10,center=true);
    }

}
module boitier()
{
    difference()
    {
        union()
        {
            difference()
            {
                // boitier
                cube([boitier_x+2*e, boitier_y + 2*e, boitier_z + 2*e],center= true);
                // evidement
                cube([boitier_x, boitier_y , boitier_z ],center= true);
                // moins couvercle
                translate([0,0,boitier_z/2 +e/2])
                    cube([couvercle_x ,boitier_y, e ],center= true);
                // moins teensy
                *translate([0,teensy_40_dy,teensy_40_dz])
                    rotate([0,0,180])
                        teensy_40();
            }
            // serrcable
            translate([0,boitier_y/2 -e/2 - serre_tete_e,0])
                cube([couvercle_x , e , boitier_z ],center= true);
        }
        // trou cables
        translate([0 ,boitier_y / 2 ,-cable_usb_d/2 ])
            rotate([90,0,0])
                cylinder(d=cable_usb_d,h=50,center=true);
        translate([0 ,boitier_y / 2 , boitier_z / 4 + e -cable_usb_d/2])
            cube([cable_usb_d,50,boitier_z/2+e + cable_usb_d/2+e],center=true);
    }
    // patte vis 
    translate([boitier_x/2-e,boitier_y/2 -serre_tete_e/2 ,boitier_z/2 -e])
        patte(serre_tete_e) ;
    translate([boitier_x/2-e,-boitier_y/2 + patte_dy ,boitier_z/2 -e])
        patte(2*e) ;
    translate([-boitier_x/2+e,boitier_y/2 -serre_tete_e/2 ,boitier_z/2 -e])
        patte(serre_tete_e) ;
    translate([-boitier_x/2+e,-boitier_y/2 + patte_dy ,boitier_z/2 -e])
        patte(2*e) ;

}

module couvercle()
{
    translate([0,0 ,boitier_z/2 +e/2])
    difference()
    {
        union()
        {
            // couvercle
            cube([boitier_x+e-0.5,boitier_y-0.5,e],center=true);
            translate([0,boitier_y/2 + e/2,0])
                cube([cable_usb_d,e,e],center=true);
        }
        // trou-vis
        translate([boitier_x/2-e,boitier_y/2 -serre_tete_e/2 ,e/2])
                vis();
        translate([-boitier_x/2+e,boitier_y/2 -serre_tete_e/2 ,e/2])
                vis();
        translate([boitier_x/2-e,-boitier_y/2 +patte_dy ,e/2])
                vis();
        translate([-boitier_x/2+e,-boitier_y/2 +patte_dy ,e/2])
                vis();
        // moins teensy
        translate([0,teensy_40_dy,-8])
            rotate([0,0,180])
                teensy_40();
    }

}
*vis();
*teensy_40();
*difference()
{
    union()
    {
        boitier();
        translate([0,teensy_40_dy,teensy_40_dz])
            rotate([0,0,180])
            teensy_40() ;
    }
    *translate([0,0,105])
        cube([200,200,200],center = true );
}
*translate([0,0,10])
    couvercle();
translate([boitier_x + 5 ,0,-boitier_z/2 ]) couvercle();
translate([0 ,0,boitier_z/2 + e ]) boitier() ;
