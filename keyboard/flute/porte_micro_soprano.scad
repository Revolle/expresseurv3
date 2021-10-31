de2=19.0;// diamètre flute bas
de1=19.2; // diamètre flute haut
de=(de1+de2)/2;
he=12; // largeur pince
e=4;

dmicro=6;
douverture=dmicro-1.6;
$fn=10;

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
        rotate([0,0,0])
            translate([0,-de+5,0])
                cube([10,2*de,he],center=true);
    }
}
porte_micro();



