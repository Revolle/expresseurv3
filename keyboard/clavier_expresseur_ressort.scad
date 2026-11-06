$fn = 10 ;
pouce=2.54;

e = 3 ;

usb_x = 13;
usb_y = 7 ;
usb_d = 4 ;
fil_d = 2 ;

touche_dd = 1 ;
touche_a = 10 ;
touche_x = 9*pouce-touche_dd ;
touche_y = 60 ;
touche_z = 5 ;
touche_n= 3 ;
touche_s = 2 ;
touche_dz = 10 ;
touche_rainure = 0.5 ;
touche_embossage_d = 50 ;
butee_y = touche_y / 4;
butee_dz = touche_dz / 4 ;
touches_x = touche_n * touche_x + (touche_n + 1) * touche_dd ;

axe_d=4 ;
axe_dd = 0.4 ;
axe_dy = -touche_y/2 + (axe_d+3)/2 ;
axe_dz = -touche_z/2 ;

boitier_x = (touche_n + 1)*touche_x ;
echo ( "largeur boitier" , boitier_x ) ;
boitier_y = touche_y + 35  ;
echo ( "profondeur boitier" , boitier_y ) ;
boitier_z = 35 ;
boitier_s = 15 ;
boitier_dx = -5 ;
boitier_coupe_z = boitier_z/2 + axe_dz - touche_z/2;
pilier_dx = 4 ;
pilier_x = 10 ;
renfort_z = 3 ;

//ressort_d = 2.3 ;
//ressort_h = 8 ;
ressort_d = 4.3+0.5 ;
ressort_h = 24 ;
ressort_trou_z = 2 ;
ressort_view = 0 ;
ressort_debut = 1.5 ;
ressort_fin = 3.5 ;

joint_e = 0.7 ;

cny70_x = 7.5 ;
cny70_y = 7.5 ;
cny70_z = 6.0 ;
cny70_patte_x = 5.5 ;
cny70_patte_y = 5.5 ;
cny70_patte_z = 8 ;
cny70_fil_l = e ;

dzusbteensy = 4.3 ;
dzteensy = 0 ;
l_trou_usb = 12 ;
h_trou_usb = 9 ;

poussoir_d = 7 ;
poussoir_h = 7 ;
poussoir_dy = 12 ;
poussoir_dz = -5 ;
usb_dy = 11 ;

vis_d = 2.3 ;
vis_tete_d=4.5 ;
vis_h=11;
vis_tete_h=1.5;
vis_dy = 20 ;

pcb_x = touches_x ;
pcb_y = 3*cny70_y ;
pcb_e = 1.6 ;
pcb_dx1 = 6*pouce ;
pcb_dx2 = -3*pouce ;
pcb_dy=25;
pcb_dz = -1 ;
pcb_a = -6 ;
pcb_support_x = 3*pouce;
pcb_support_y = pcb_y;
pcb_support_z = 20 ;

module vis(em , dd)
{
    // dd : augmentation du trou prepercage
    // em : augentation trou tete
    translate([0,0,-vis_h/2-(vis_tete_h)])
    {
        cylinder(d=vis_d+dd,h=vis_h,center=true);
        translate([0,0,vis_h/2+(vis_tete_h+em)/2])
            cylinder(d=vis_tete_d,h=vis_tete_h+em,center=true);
    }
}
module teensy2()
{
     union()
    {
        for(i=[-6,-5,-4,-3,2,3,5])
        {
            translate([-3*pouce,(i+0.5)* pouce ,4]) cube([0.7,0.7,8],center=true) ;
            translate([3*pouce,(i+0.5)* pouce ,4]) cube([0.7,0.7,8],center=true) ;
        }
        // circuit
        translate([0,0,7])
            cube([7.5*pouce,12*pouce,1.2],center=true);
        // usb
        translate([0,10.5,dzusbteensy])
            difference()
            {
                cube([7,9,4],true);
                cube([5,10,2],true);
            }
        // bouton
        translate([0,-4.5*pouce,4.5])
            cube([3,3,3],true);
   }
}
module cny70(pattex)
{
    // capteur
    translate([0,0,cny70_z/2])
        cube([cny70_x,cny70_y,cny70_z],center = true);
    // fils cone
    *hull()
    {
        translate([0,0,0.05+cny70_z])
            cube([cny70_patte_x,cny70_patte_y,0.1],center = true);
        translate([0,0,cny70_patte_z+cny70_z])
            cylinder(d=fil_d,h=4,center=true);
    }
    *translate([0,0,cny70_patte_z+cny70_z+cny70_fil_l/2])
        cylinder(d= d_fil,h=cny70_fil_l,center= true);
    // pattes
    *translate([0,0,pattex*cny70_patte_z/2+cny70_z])
        cube([cny70_patte_x,cny70_patte_y,pattex*cny70_patte_z],center = true);
    // fil
    *translate([0,0,(cny70_fil_l+6)/2+cny70_patte_z+cny70_z])
        cylinder(d=fil_d,h=cny70_fil_l +6,center=true);
    //pattes
    translate([pouce/2,pouce/2,7])
        cube([1,1,3],center=true);
    translate([-pouce/2,pouce/2,7])
        cube([1,1,3],center=true);
    translate([pouce/2,-pouce/2,7])
        cube([1,1,3],center=true);
    translate([-pouce/2,-pouce/2,7])
        cube([1,1,3],center=true);

}
module resistance()
{
    translate([0,0,3/2])
    {
        translate([0,2*pouce,-1.5])
            cube([1,1,4],center=true);
        translate([0,-2*pouce,-1.5])
            cube([1,1,4],center=true);
        cube([1,4*pouce,1],center=true);
        cube([pouce,3*pouce,pouce],center=true);
    }
}
module pcb()
{
    difference()
    {
        translate([0,0,pcb_e/2])
        {
            cube([pcb_x,pcb_y,pcb_e],center=true);
            // capteurs
            for(i=[1:touche_n])
            {
                translate([(i-(touche_n+1)/2) * ( touche_x + touche_dd ) ,0,pcb_e/2])
                {
                    translate([0,5*pouce/2,cny70_z])
                        rotate([180,0,0])
                            cny70(0.3);
                    translate([2*pouce,pouce,0])
                        resistance();
                    translate([3*pouce,pouce,0])
                        resistance();
                }
            }
            translate([-12*pouce,pouce,0])
                resistance();
            // vis
            translate([pcb_dx1,0,2])
                vis(0,0);
            translate([pcb_dx2,0,2])
                vis(0,0);
        }
    }
}
module touche()
{
    difference()
    {
        union()
        {
            hull()
            {
                translate([touche_x/2-touche_s/2, touche_y/2 - touche_s/2,touche_z/2- touche_s/2])
                    sphere(d=touche_s);
                translate([-(touche_x/2-touche_s/2), touche_y/2- touche_s/2,touche_z/2- touche_s/2])
                    sphere(d=touche_s);
                translate([touche_x/2-touche_s/2, -(touche_y/2- touche_s/2),touche_z/2- touche_s/2])
                    sphere(d=touche_s);
                translate([-(touche_x/2-touche_s/2), -(touche_y/2- touche_s/2),touche_z/2- touche_s/2])
                    sphere(d=touche_s);
                translate([touche_x/2-touche_s/2, touche_y/2- touche_s/2,-(touche_z/2- touche_s/4)])
                    cube([touche_s/2,touche_s/2,touche_s/2],center=true);
                translate([-(touche_x/2-touche_s/2), touche_y/2- touche_s/2,-(touche_z/2- touche_s/4)])
                    cube([touche_s/2,touche_s/2,touche_s/2],center=true);
                translate([touche_x/2-touche_s/2, -(touche_y/2- touche_s/2),-(touche_z/2- touche_s/4)])
                    cube([touche_s,touche_s,touche_s],center=true);
                translate([-(touche_x/2-touche_s/2), -(touche_y/2- touche_s/2),-(touche_z/2- touche_s/4)])
                    cube([touche_s,touche_s,touche_s],center=true);
                translate([0,-touche_y/2 + (axe_d+3)/2 , -touche_z/2 ])
                    cube([touche_x, axe_d+3,axe_d+3],center=true);
            }
            // axe
            translate([0,-touche_y/2 + (axe_d+3)/2 , -touche_z/2 ])
                cube([touche_x, axe_d+3,axe_d+3],center=true);
            // rondelle d ecartement
            translate([touche_dd/2,-touche_y/2 + (axe_d+3)/2 , -touche_z/2 ])
                rotate([0,90,0])
                    cylinder(d=axe_d + 3, h = touche_x + touche_dd ,center=true);
            // butee
             translate([0,-touche_y/2 - butee_y / 2 + (axe_d+3)/2, -touche_z/2 - butee_dz + e/2 -e ])
                cube([touche_x, butee_y + (axe_d+3) ,e],center=true);
           
        }
        // trou axe
        translate([0,axe_dy, axe_dz  ])
            rotate([0,90,0])
                    cylinder(d=axe_d+axe_dd,h= touche_x+2,center=true);
        // bossage
        translate([0,touche_y/2 - 11 ,touche_embossage_d/2+1.2])
            sphere(d=touche_embossage_d);
    }
    // bord avant
    translate([0,touche_y/2 -e/2 - touche_s/2 - e - touche_dd ,-touche_z/2 - touche_dz/2])
        cube([touche_x-touche_s,e,touche_dz],center = true );

}
module touche_incline(a)
{
    translate([0,axe_dy,axe_dz])
        rotate([-a,0,0])
            translate([0,-axe_dy,-axe_dz])
                touche();
}

module boitier_cube(de)
{
    hull()
    {
        // coins arrondis
        translate([boitier_x/2 - boitier_s/2  - de , boitier_y/2- boitier_s/2 - de , boitier_z/2- boitier_s/2- de ])
            sphere(d=boitier_s);
        translate([-(boitier_x/2 - boitier_s/2 - de ) , boitier_y/2- boitier_s/2 - de , boitier_z/2- boitier_s/2 - de ])
            sphere(d=boitier_s);
        translate([boitier_x/2 - boitier_s/2  - de , -(boitier_y/2- boitier_s/2 - de ) , boitier_z/2- boitier_s/2 - de ])
            sphere(d=boitier_s);
        translate([-(boitier_x/2 - boitier_s/2 - de ) , -(boitier_y/2- boitier_s/2- de ), boitier_z/2- boitier_s/2- de ])
            sphere(d=boitier_s);
        translate([boitier_x/2 - boitier_s/2 +boitier_dx - de  , boitier_y/2- boitier_s/2- de  , -(boitier_z/2- boitier_s/2- de ) ])
            sphere(d=boitier_s);
        translate([-(boitier_x/2 - boitier_s/2+boitier_dx - de  ) , boitier_y/2- boitier_s/2- de , - (boitier_z/2- boitier_s/2 - de )])
            sphere(d=boitier_s);
        translate([boitier_x/2 - boitier_s/2 +boitier_dx - de  , -(boitier_y/2- boitier_s/2 - de ) ,- (boitier_z/2- boitier_s/2- de )])
            sphere(d=boitier_s);
        translate([-(boitier_x/2 - boitier_s/2 +boitier_dx - de ) , -(boitier_y/2- boitier_s/2 - de ) ,- (boitier_z/2- boitier_s/2- de )])
            sphere(d=boitier_s);
        
        // aretes droites pour le bord des touches
        translate([0 , boitier_y/2 - de -e,  (boitier_z/2- de - e/2 )])
            cube([boitier_x - boitier_s,e,e] ,center = true );
        translate([0 , boitier_y/2 - de -e, - (boitier_z/2- de - e/2 )])
            cube([boitier_x - boitier_s,e,e] ,center = true );
    }
}
module boitier(vis_dd)
{
    difference()
    {
        // boitier plein
        boitier_cube(0) ;
        // evidement
        difference()
        {
            boitier_cube(e);
            // butee
            translate([0,(boitier_y - touche_y)/2 - butee_y  - touche_y/2 + e/2, -butee_dz -e/2 - joint_e])
                cube([200,e,boitier_z -2*e - butee_dz -e - joint_e -0.8],center = true);
            // piliers axe
            translate([0,(boitier_y - touche_y)/2 + axe_dy, 0])
                difference()
                {
                    cube([boitier_x,10,100],center = true);
                    cube([touches_x,10,100],center = true);
                }
            // piliers vis
            translate([boitier_x/2-pilier_dx,boitier_y/2 - vis_dy, 0])
                cube([pilier_x,pilier_x,100],center = true);
            translate([boitier_x/2-pilier_dx,-(boitier_y/2 - vis_dy), 0])
                cube([pilier_x,pilier_x,100],center = true);
            translate([-(boitier_x/2-pilier_dx),boitier_y/2 - vis_dy, 0])
                cube([pilier_x,pilier_x,100],center = true);
            translate([-(boitier_x/2-pilier_dx),-(boitier_y/2 - vis_dy), 0])
                cube([pilier_x,pilier_x,100],center = true);
            // support bouton poussoir
            translate([-boitier_x/2+7,-boitier_y/2+poussoir_dy , poussoir_dz ])
                cube([8,poussoir_d + 10,poussoir_d + 12] , center = true );
            // support fil usb
            translate([-(boitier_x/2-pilier_dx),-boitier_y/2-3+poussoir_dy, boitier_coupe_z ])
                cube([14, 14,20] , center = true );
            // support pcb
            rotate([pcb_a,0,0])
                translate([pcb_dx1,pcb_dy,-pcb_support_z/2 + pcb_dz])
                    cube([pcb_support_x,pcb_support_y,pcb_support_z],center = true);
            rotate([pcb_a,0,0])
                translate([pcb_dx2,pcb_dy,-pcb_support_z/2 + pcb_dz])
                    cube([pcb_support_x,pcb_support_y,pcb_support_z],center = true);
        }
        // vis boitier
        translate([boitier_x/2-pilier_dx-1,boitier_y/2 - vis_dy, boitier_z/2 - 2])
            rotate([180,0,0])
                translate([0,0,vis_h+vis_tete_h])
                    vis(20,vis_dd);
        translate([-(boitier_x/2-pilier_dx-1),boitier_y/2 - vis_dy, boitier_z/2 - 2])
            rotate([180,0,0])
                translate([0,0,vis_h+vis_tete_h])
                    vis(20,vis_dd);
        translate([boitier_x/2-pilier_dx-1,-(boitier_y/2 - vis_dy), boitier_z/2 - 2])
            rotate([180,0,0])
                translate([0,0,vis_h+vis_tete_h])
                    vis(20,vis_dd);
        translate([-(boitier_x/2-pilier_dx-1),-(boitier_y/2 - vis_dy), boitier_z/2 - 2])
            rotate([180,0,0])
                translate([0,0,vis_h+vis_tete_h])
                    vis(20,vis_dd);
        // vis pcb
        rotate([pcb_a,0,0])
            translate([pcb_dx1,pcb_dy,-boitier_z/2 + vis_h  + 2.5])
                cylinder(d=vis_d, h = vis_h+2,center = true);
        rotate([pcb_a,0,0])
            translate([pcb_dx2,pcb_dy,-boitier_z/2 + vis_h  + 2.5])
                cylinder(d=vis_d, h = vis_h+2,center = true);
        // touches
        translate([0,boitier_y/2-touche_y/2,50+boitier_z/2-touche_z-touche_dz])
            cube([touches_x ,touche_y+touche_dd*2,100],center = true);
       // axe touches
        translate([0, (boitier_y - touche_y)/2 + axe_dy , boitier_z/2 - touche_z/2-touche_z/2])
            rotate([0,90,0])
                cylinder(d=axe_d,h= boitier_x -9,center = true);
       // trou fil usb
        translate([ - boitier_x/2 , -boitier_y / 2 +usb_dy ,   boitier_coupe_z])
            rotate([0,90,,0])
                cylinder(d=usb_d , h = 35,center = true);
        // trou bouton poussoir
        translate([-boitier_x/2+e,-boitier_y/2+12 , poussoir_dz ])
            rotate([0,90,0])
                    cylinder(d=poussoir_d, h = poussoir_h + 20  , center = true );
        // passe-fil
        translate([-24,(boitier_y - touche_y)/2 - butee_y  - touche_y/2 + e/2,-boitier_z/2+7])
            cube([8,20,6],center = true);
    }
    // rondelle d ecartement
    difference()
    {
        translate([-touches_x / 2 + touche_dd/2, (boitier_y - touche_y)/2 + axe_dy , boitier_z/2 - touche_z/2-touche_z/2])
            rotate([0,90,0])
                cylinder(d=axe_d + 3, h = touche_dd ,center=true);
       // axe touches
        translate([0, (boitier_y - touche_y)/2 + axe_dy , boitier_z/2 - touche_z/2-touche_z/2])
            rotate([0,90,0])
                cylinder(d=axe_d,h= boitier_x -9,center = true);
    }

}
module touches(a)
{
    for(i=[1:touche_n])
    {
        translate([(i-(touche_n+1)/2) * ( touche_x + touche_dd ) , (boitier_y - touche_y)/2 , boitier_z/2 - touche_z/2])
            touche_incline(a) ;
    }
}
module renfort_vis()
{
        translate([boitier_x/2-pilier_dx-1,boitier_y/2 - vis_dy, boitier_coupe_z -renfort_z / 2])
                    cube([pilier_x,pilier_x,renfort_z],center = true);
        translate([-(boitier_x/2-pilier_dx-1),boitier_y/2 - vis_dy, boitier_coupe_z -renfort_z / 2])
                    cube([pilier_x,pilier_x,renfort_z],center = true);
        translate([boitier_x/2-pilier_dx-1,-(boitier_y/2 - vis_dy), boitier_coupe_z -renfort_z / 2])
                    cube([pilier_x,pilier_x,renfort_z],center = true);
        translate([-(boitier_x/2-pilier_dx-1),-(boitier_y/2 - vis_dy), boitier_coupe_z -renfort_z / 2])
                    cube([pilier_x+4,pilier_x,renfort_z],center = true);

}
module boitier_bas()
{
    difference()
    {
        boitier(0.3) ;
        union()
        {
            translate([0,0,50+boitier_coupe_z])
                cube([100,100,100],center=true);
            renfort_vis();
        }  
    }
}
module boitier_haut()
{
    difference()
    {
        boitier(0) ;
        difference()
        {
            translate([0,0,-50+boitier_coupe_z])
                cube([100,100,100],center=true);
            renfort_vis();
        }  
    }
}
*rotate([180,0,0])
    translate([0,0,vis_h+vis_tete_h])
        vis(15,0);
*teensy2() ;
*cny70() ;
*capteur(20) ;
*difference()
{
    touche() ;
    translate([500,0,0])
        cube([1000,1000,1000],center=true);
}
*touche();
*touche_incline(touche_a) ;
*boitier(0);
boitier_bas() ;
translate([boitier_x  +2,0,-boitier_coupe_z - boitier_z/2 + renfort_z ])
    boitier_haut() ;
*pcb();
*difference()
{
    union()
    {
       *boitier(0);
       boitier_bas()
       *boitier_haut();
       *touches(0);
       *touches(touche_a);
        // teensy
        translate([0,-30,0])
            rotate([90,0,0]) rotate([0,0,90])  teensy2() ;
        //  pcb
        rotate([pcb_a,0,0])
            translate([0,pcb_dy, pcb_dz])
                pcb();
    }
    translate([500 + boitier_x/2 -4.5,00,0])
        cube([1000,1000,1000],center=true);
}
*difference()
{
    translate([0,0,-50+boitier_coupe_z])
        cube([100,100,100],center=true);
    renfort_vis();
}  
