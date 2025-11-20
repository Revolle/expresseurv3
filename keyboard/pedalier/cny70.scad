$fn = 50 ;
e = 3 ;

ruban_e = 2 ;
ruban_languette_e = 3.5 ;
ruban_l = 31 ;

butee_z = 2*e ;

chaussure_l_min = 80;
chaussure_l_max = 85;

fil_d = 3.5 ;

cny70_x = 7.2 ;
cny70_y = 7.2 ;
cny70_z = 6.2 ;
cny70_patte_x = 5 ;
cny70_patte_y = 5 ;
cny70_patte_z = 6 ;
cny70_fil_l = e ;

cny70_capsule_x = cny70_x + 2*e ;
cny70_capsule_y = cny70_x + 2*e ;
cny70_capsule_z = cny70_z + cny70_patte_z + cny70_fil_l + e ;
module cny70()
{
    translate([0,0,cny70_z/2])
        cube([cny70_x,cny70_y,cny70_z],center = true);
    translate([0,0,cny70_patte_z/2+cny70_z])
        cube([cny70_patte_x,cny70_patte_y,cny70_patte_z],center = true);
    translate([0,0,(cny70_fil_l+6)/2+cny70_patte_z+cny70_z])
        cylinder(d=fil_d,h=cny70_fil_l +6,center=true);

}
module cny70_capsule()
{
    difference()
    {
        translate([0,0,cny70_capsule_z/2])
            cube([cny70_capsule_x,cny70_capsule_y,cny70_capsule_z],center = true);
        cny70();
    }
}

module plaque()
{
    difference()
    {
        union()
        {
            //plaque
            hull()
            {
                // plaque
                translate([ chaussure_l_max/2, 0, e/2 ])
                    cube([ chaussure_l_max , ruban_l + 2*e , e ], center = true);
                // support capteur
                translate([-cny70_capsule_x/2, cny70_capsule_y/2 + ruban_l / 2, e/2])
                    cube([cny70_capsule_x,cny70_capsule_y, e],center = true);
                // support butee
                translate([-(ruban_languette_e + 2 *e)/2, 0, e/2])
                    cube([ruban_languette_e + 2 *e,ruban_l + 2*e, e],center = true);
            }
            //butee
            translate([ - e / 2,0,butee_z/2 + e/2])
                cube([e, ruban_l + 2*e , butee_z ], center = true);
            // capteur
            translate([-cny70_capsule_x/2,cny70_capsule_y/2 + ruban_l / 2 ,0 ])
                cny70_capsule();

       }
       // fente adaptation taille
        translate([100 + chaussure_l_min,0,0])
            cube([200, ruban_l  , 2*e ], center = true);
        // fente ruban          
        translate([-ruban_languette_e/2 - e,0,0])
            cube([ruban_languette_e, ruban_l  , 100 ], center = true);            
        // capteur
        translate([-cny70_capsule_x/2, cny70_capsule_y/2 + ruban_l / 2, 0])
            cny70();
    }
}
//cny70();
plaque() ;



