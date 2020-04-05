d0 = 2 ;
d_doigt= 20;
l_doigt = 6 ;
d_corde1 = [2,1]; // diametre cordes
entre_corde = 12;
nbcorde = 3 ;
l0 = 15;
e= 2;
d_tirette = 2 ;

module huitieme_cercle(d)
{
     difference()
    {
        cylinder(h=e+0.1 ,d = d, center = true);
        translate([d/2,0,0]) cube([d,d,e+0.2],center=true);
        rotate([0,0,45]) translate([0,d/2,0]) cube([d,d,e+0.2],center=true);
    }
}
module etoile(d, v)
{
   for(i=[0:4])
   {
        rotate([0,0, 90*i + 45*v ])
            huitieme_cercle(d+0.1);
   }
}

module doigt(l,d_ext,d_int, d_corde, corde)
{
    difference()
    {
        union()
        {
            // tube principale
            cylinder(h=l, d=d_ext, center = false);
            // doigt
            translate([d_doigt/2,0,e/2])
                difference()
                {
                    cube([d_doigt,l_doigt,e],center=true);
                    if ( corde == true )
                    {
                        // encoche corde
                        translate([0,l_doigt/2,0]) 
                            rotate([0,90,0]) 
                            cylinder(h=d_doigt,d=d_corde,center = true);
                    }
                    else
                    {
                        translate([d_doigt/2- d_tirette,0,0]) 
                            cylinder(h=2*d_doigt,d=d_tirette,center = true);
                    }
                }
        }
        // trou pour l'axe
        cylinder(h=l, d=d_int, center = false);
        //assemblage
        translate([0,0,l-e/2]) etoile(d_ext, 0);
    }
}

module doit_fourchette(l,d_ext,d_int, d_corde)
{
    doigt(l,d_ext,d_int,d_corde,true);
    translate([0,0,2*l-e+0.1]) rotate([0,180,-90]) 
        doigt(l,d_ext,d_int,d_corde,false);
}
for(i=[0:nbcorde])
{
    translate([0,0,entre_corde * i])
        doit_fourchette(entre_corde * nbcorde / 2 + 8  ,d0 *( i +1) ,d0 * i + 0.4 ,d_corde1[i]);
}