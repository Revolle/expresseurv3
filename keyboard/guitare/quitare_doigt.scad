e= 2; // epaisseur
d0 = e ; // diametre de base
d_doigt= 20; // longueur doigt
l_doigt = 6 ; // largeur doigt
d_corde = 1.2; // diametre cordes
l_cordes_0 = 17 ; // 43 ;
l_cordes_1 = 18; //51 ;

nbcorde = 3 ; // nombre de cordes
entre_corde = l_cordes_0/(nbcorde-1); // espace entre deux cordes
d_entre_corde = (l_cordes_1/(nbcorde-1) - entre_corde)/10;// increment espace entre deux cordes
d_tirette = 2 ; // diametre trou tirette

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

module doigt(l,d_ext,d_int, corde)
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

module doit_fourchette(l,d_ext,d_int)
{
    doigt(l/2,d_ext,d_int,true);
    translate([0,0,l-e+0.1]) rotate([0,180,-90]) 
        doigt(l/2,d_ext,d_int,false);
}
for(i=[0:nbcorde - 1])
{
    translate([0,0, entre_corde * i])
        doit_fourchette(entre_corde * (nbcorde) - i*entre_corde + (nbcorde -1 -i)*(e+1) ,d0 *( i +2) ,d0 * (i + 1) + 0.4);
}