$fn = 10 ;
e = 3 ;

cny70_x = 7.5 ;
cny70_y = 7.5 ;
cny70_z = 6.0 ;
cny70_patte_x = 5.5 ;
cny70_patte_y = 5.5 ;
cny70_patte_z = 8 ;
cny70_fil_e = 2 ;

cny70_capsule_x = cny70_x + 2*e ;
cny70_capsule_y = cny70_y + 2*e ;
cny70_capsule_z = cny70_z + cny70_patte_z + cny70_fil_e  ;

fil_d = 3.8;
lien_l = 3 ;
lien_e = 1.5 ;
a = 15 ;

module cny70(ep)
{
     // pattes
    translate([0,0,fil_d/2])
        cube([cny70_x,cny70_y + 2 * cny70_patte_z ,fil_d],center = true);
    // fil
    translate([0,0,fil_d/2])
        rotate([90,0,0])
            cylinder(d=fil_d,h=15,center=true);
   // capteur
    translate([0,0,cny70_z/2 + fil_d + 1])
        cube([cny70_x,cny70_y+ep,cny70_z],center = true);
    translate([0,0,fil_d + 1/2])
        cube([cny70_x-1.5,cny70_y-1.5,1],center = true);

}
module decoupe_cny70()
{
    // pattes
    translate([5,0,fil_d/2])
        cube([cny70_x+10,cny70_y + 2 * cny70_patte_z ,fil_d],center = true);
    // fil
    translate([0,0,fil_d/2])
        rotate([90,0,0])
            cylinder(d=fil_d,h=60,center=true);
    translate([5,0,fil_d/2])
        cube([10,60,fil_d],center=true);
   // capteur
    translate([5,0,cny70_z/2 + fil_d+2])
        cube([cny70_x+10,cny70_y,cny70_z],center = true);
    translate([5,0,cny70_z/2 + fil_d + 1])
        cube([cny70_x+10,cny70_y,cny70_z],center = true);
    translate([5,0,fil_d + 1/2])
        cube([cny70_x-1.5+10,cny70_y-1.5,1],center = true);
   
}
difference() {
    hull() {
        translate([0,0,2*e ]) 
            rotate([0 , a  ,0]) 
                cny70(15) ;
        translate([-5,0,e/2])
            cube([cny70_capsule_x + 10 ,cny70_capsule_y + 40 ,e],center = true);   
        }
    translate([0,0,2*e ]) 
        rotate([0 , a  ,0]) 
            decoupe_cny70() ;
}

*translate([0,0,2*e ]) 
    rotate([0 , a  ,0]) 
        cny70(0) ;
*decoupe_cny70() ;
