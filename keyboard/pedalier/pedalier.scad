$fn=50;

teensy_x = 17.8 ;
teensy_y = 36 ;
teensy_z = 9 ;

capteur_d = 18.5 ;
capteur_e = 0.5 ;
capteur_y = 8 ;
capteur_x = 57-capteur_d;
capteur_pin_x = 4 ;
capteur_a = 2 ;


connecteur_x = 5;
connecteur_y = 6;
connecteur_z = 4 ;
e=4.5 ;

plaque_x=185;
plaque_y=teensy_y +  e;

capteur_dx = plaque_x /2 - capteur_d / 2 - e ;

module capteur(trou)
{
    hull()
    {
        cylinder(d=capteur_d,h = capteur_e,center=true);
        translate([capteur_d/2,capteur_y/2,0])
            cube([1,1,capteur_e],center=true);
        translate([capteur_d/2,-capteur_y/2,0])
            cube([1,1,capteur_e],center=true);
    }
    translate([capteur_d/2,0,0])
        rotate([0,capteur_a,0])
            union()
            {
                translate([capteur_x/2,0,0])
                   cube([capteur_x,capteur_y,capteur_e],center = true);
                translate([capteur_x + capteur_pin_x/2 ,2.54/2,0])
                   cube([capteur_pin_x,1,capteur_e],center = true);
                translate([capteur_x + capteur_pin_x/2 ,-2.54/2,0])
                   cube([capteur_pin_x,1,capteur_e],center = true);
                translate([capteur_x + 1 + connecteur_x /2 ,0,-1])
                   cube([connecteur_x*trou,connecteur_y*trou,connecteur_z],center = true);
            }
}

module teensy()
{
    cube([teensy_x,teensy_y,teensy_z],center= true );
}

module support()
{
    cube([plaque_x,plaque_y,e],center=true);
    translate([capteur_dx,0,e/2])
        cylinder(d=capteur_d+e,h = capteur_e,center=true);
    translate([-capteur_dx,0,e/2])
        cylinder(d=capteur_d+e,h = capteur_e,center=true);
    translate([0,0,(teensy_z + e)/2 - e/2])
        cube([teensy_x + 2*e ,plaque_y,teensy_z + e],center=true);

}

rotate([90,0,0])
difference()
{
    support() ;
    translate([-capteur_dx,0,e/2+capteur_e/2])
        capteur(1);
    translate([capteur_dx,0,e/2+capteur_e/2])
        rotate([0,0,180])
            capteur(1);
    //teensy
    translate([0,e/2,teensy_z/2-e/2])
        cube([teensy_x,teensy_y,teensy_z],center= true );
    //raccords
    translate([0,0,-18])
        cube([65,connecteur_y,40],center= true );
}

*union()
{
    translate([-capteur_dx,0,e/2+capteur_e/2])
        capteur(1);
    translate([capteur_dx,0,e/2+capteur_e/2])
        rotate([0,0,180])
            capteur(1);
    translate([0,e/2,teensy_z/2-e/2])
        teensy() ;
}