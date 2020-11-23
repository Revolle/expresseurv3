e=4; // epaisseur

/*
// flute soprano Yamaha
de2=19.4;// diamètre flute bas
de1=19.6; // diamètre flute haut
de=(de1+de2)/2;
he=12; // largeur pince
da_trou = 0 ;// angle rotation trou
l_ouevrture_pince=10;
*/

/*
// flute alto aulos
de2=25.6;// diamètre flute bas
de1=25.8; // diamètre flute haut
de=(de1+de2)/2;
he=15; // largeur pince
da_trou = 0;  // angle rotation trou
l_ouevrture_pince=10;
*/

/*
// flute tenor aulos
de2=39.6;// diamètre flute bas
de1=38.3; // diamètre flute haut
de=(de1+de2)/2;
he=15; // largeur pince
da_trou = 0;  // angle rotation trou
l_ouevrture_pince=10;
*/

// flute basse aulos
de2=44.9;// diamètre flute bas
de1=45.5; // diamètre flute haut
de=(de1+de2)/2;
he=15; // largeur pince
da_trou = 0;  // angle rotation trou
l_ouevrture_pince=15;

dmicro=6;
douverture=dmicro-1.6;
$fn=40;
module serre_micro()
{
    translate([de/2+e/2+dmicro/2,0,1])
        union()
        {
            difference()
            {
                cylinder(d=dmicro+2*e,h=he,center=true);
                cylinder(d=dmicro+e,h=he,center=true);
                translate([-6,0,0])
                        cube([5,de,he],center=true);        
            }
            difference()
            {
                translate([dmicro/2,0,0])
                    cube([e,douverture-0.5,he],center=true);
                cylinder(d=dmicro,h=he,center=true);
            }
        }
}
module porte_micro()
{
    difference()
    {
        union()
        {
            difference()
            {
                // tube flute
                cylinder(d1=de1+e,d2=de2+e,h=he,center=true); 
                // extrusion tube flute
                translate([0,-e/2,0])
                    cylinder(d1=de1,d2=de2,h=he,center=true); 
            }
            translate([de/2+e/2+dmicro/2,0,0])
            {
                difference()
                {
                    // tube micro
                    cylinder(d=dmicro+e,h=he,center=true); 
                    // extrusion tube micro
                    cylinder(d=dmicro,h=he,center=true);
                    // extrusion passage micro
                    translate([dmicro-e/2,0,0])
                        cube([1.5*e,douverture,he],center=true); 
                }
            }
        }
        rotate([0,0,da_trou])
            translate([0,-de+5,0])
                cube([l_ouevrture_pince,2*de,he],center=true);
    }
}
porte_micro();
*serre_micro();


