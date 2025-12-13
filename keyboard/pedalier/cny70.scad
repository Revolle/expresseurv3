$fn = 50 ;
e = 3 ;

ruban_e = 2 ;
ruban_languette_e = 3.5 ;
ruban_l = 31 ;

butee_z = 2*e ;

chaussure_l_min = 80;
chaussure_l_max = 85;

fil_d = 3.0 ;

cny70_x = 7.5 ;
cny70_y = 7.5 ;
cny70_z = 6.0 ;
cny70_patte_x = 5.5 ;
cny70_patte_y = 5.5 ;
cny70_patte_z = 8 ;
cny70_fil_l = e ;

ycaptplus = 1 ;
cny70_capsule_x = cny70_x + 2*e ;
cny70_capsule_y = cny70_x + 2*e ;
cny70_capsule_z = cny70_z + cny70_patte_z + cny70_fil_l  ;

d_fil = 3.8;
lien_l = 3 ;
lien_e = 1.5 ;

// diametre vis
vis_d = 1.6 ;
vis_l = 3 ;
vis_tete_d = 4.5 ;
vis_tete_l=2 ;

module vis()
{
    translate([0,0,5])
        cylinder(d=vis_tete_d+1,h=10,center=true);
    translate([0,0,-vis_tete_l/2])
        cylinder(d1=vis_d,d2=vis_tete_d,h=vis_tete_l,center=true);
    translate([0,0,-vis_l/2-vis_tete_l])
        cylinder(d=vis_d,h=vis_l,center=true);
}
module cny70()
{
    // capteur
    translate([0,0,cny70_z/2])
        cube([cny70_x,cny70_y,cny70_z],center = true);
    // fils
    hull()
    {
        translate([0,0,0.05+cny70_z])
            cube([cny70_patte_x,cny70_patte_y,0.1],center = true);
        translate([0,0,cny70_patte_z+cny70_z])
            cylinder(d=fil_d,h=4,center=true);
    }
    translate([0,0,cny70_patte_z+cny70_z+cny70_fil_l/2])
        cylinder(d= d_fil,h=cny70_fil_l,center= true);
    *translate([0,0,cny70_patte_z/2+cny70_z])
        cube([cny70_patte_x,cny70_patte_y,cny70_patte_z],center = true);
    *translate([0,0,(cny70_fil_l+6)/2+cny70_patte_z+cny70_z])
        cylinder(d=fil_d,h=cny70_fil_l +6,center=true);

}
module cny70_capsule()
{
    difference()
    {
        translate([0,ycaptplus/2,cny70_capsule_z/2])
            cube([cny70_capsule_x,cny70_capsule_y+ycaptplus,cny70_capsule_z],center = true);
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
                translate([-cny70_capsule_x/2, cny70_capsule_y/2 + ruban_l / 2+ycaptplus/2, e/2])
                    cube([cny70_capsule_x,cny70_capsule_y+ycaptplus, e],center = true);
                // support butee
                translate([-cny70_capsule_x/2, 0, e/2])
                    cube([cny70_capsule_x ,ruban_l + 2*e, e],center = true);
            }
            //butee
            translate([ - e / 2,0,butee_z/2 + e/2])
                cube([e, ruban_l + 2*e , butee_z ], center = true);
            // capteur
            translate([-cny70_capsule_x/2,cny70_capsule_y/2 + ruban_l / 2 ,0 ])
                cny70_capsule();
            // accroche cable
            translate([ -cny70_capsule_x/2 ,-(ruban_l + 2*e)/2+ (2*e+lien_l)/2,e/2 + 2])
                cube([cny70_capsule_x, (2*e+lien_l) , e ], center = true);
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
        translate([-cny70_capsule_x, cny70_capsule_y/2 + ruban_l / 2, 0])
            cube([cny70_capsule_x,d_fil,100],center = true);
       // accroche cable
        //dessous
        translate([ -cny70_capsule_x/2 -e -1 ,-(ruban_l + 2*e)/2+ (2*e+lien_l)/2,lien_e/2])
            cube([cny70_capsule_x, lien_l , lien_e ], center = true);
        //cotes
        translate([ -cny70_capsule_x + lien_e/2,-(ruban_l + 2*e)/2+ (2*e+lien_l)/2,0])
            cube([lien_e, lien_l , 10 ], center = true);
        translate([  -e -ruban_languette_e - lien_e/2,-(ruban_l + 2*e)/2+ (2*e+lien_l)/2,0])
            cube([lien_e, lien_l , 10 ], center = true);

        // vis
        translate([-cny70_capsule_x/2, cny70_capsule_y + ruban_l / 2 +ycaptplus -0.5,cny70_z/ 2])
            rotate([-90,0,0])
                vis() ;
        translate([-cny70_capsule_x/2 - fil_d, cny70_capsule_y + ruban_l / 2 +ycaptplus-3,cny70_capsule_z - vis_tete_d +1 ])
            rotate([-90,0,0])
                vis() ;
        }
}
*difference()
{
    translate([0, 0, cny70_capsule_z/2])
        cube([cny70_capsule_x,cny70_capsule_y, cny70_capsule_z],center = true);
    cny70();
}
*cny70();
*difference()
{
    plaque() ;
    translate([500,0,0])
        cube([1000,1000,1000],center=true);
    translate([0,-484,0])
        cube([1000,1000,1000],center=true);
}
*plaque();
translate ([0,ruban_l +30,0,])
    rotate([0,0,180])
        mirror([1,0,0]) plaque();


