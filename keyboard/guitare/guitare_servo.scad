$fn=6;

servo_dessine = 0 ;
finger_dessine = 1 ;
manche_dessine = 0 ;

// epaisseur generique des plaques
epaisseur = 2 ; 

// taille servo, hors fixations
servo_y=14;
servo_x=24;
servo_z=epaisseur/2;
servo_debordement_x=5;
servo_entraxe_x=19.5;
servo_entraxe_y=16.7;
servo_entraxe_d=1.2;

servo_dx=5; // taille de la plaquette de fixation du servo
servo_dy=2.25;
servo_dz = servo_z ;
servo_px=servo_x/2-servo_dx/2;
// moteur du servo
moteur_x=18;
moteur_d=5.5;
moteur_dy=4;
moteur_dx=-3;
// bloc translateur
translateur_x=23;
translateur_y=5;
translateur_z=7;
translateur_dy=-3;
translateur_dx=0;
// decalage pour imbriquer les servos
servo_decalage_x=servo_dx*1.2 ;

// taille doigt
finger_x=22;
finger_y=4;
finger_z=3;
// decentrage du doigt par apport au centre du servo
finger_dy=translateur_dy; 
finger_dz = translateur_z ; 

// position poussé tiré du doigt
finger_d_on_off = 8 ; // decalage entre position on et off
finger_offset = 1 ;
finger_on = finger_offset ; 
finger_off = finger_offset - finger_d_on_off ; 

// taquet de fixation
finger_taquet_x=4.5;
finger_taquet_y=1;
finger_taquet_z=2;
finger_taquet_dz=0;
finger_taquet_trou_d1=1;
finger_taquet_trou_d2=0.5;
finger_taquet_trou_y=finger_taquet_y/4;
finger_taquet_dx=-8;
finger_taquet_fente_y=0.2;

// plaque de support des servos
corde_dy_1=8.5; // distance entre deux cordes sur la case 1
corde_dy_2=8.7; // distance entre deux cordes sur la case 2
corde_dy_3=8.9; // distance entre deux cordes sur la case 3
corde_dy_4=9.1; // distance entre deux cordes sur la case 4
plaque_x = servo_x + servo_debordement_x + servo_decalage_x + 1.5*epaisseur ; 
carre=8.5; // carre balsa rajoute sur le bord du manche
plaque_y_0=53.5+2*carre; // largeur manche sur la case 1
plaque_y_1=54+2*carre; // largeur manche sur la case 1
plaque_y_2=54.5+2*carre; // largeur manche sur la case 2
plaque_y_3=55+2*carre; // largeur manche sur la case 3
plaque_y_4=55.5+2*carre; // largeur manche sur la case 4
plaque_z=epaisseur;

case_dz_1 = 25 ; // dimension case 1
case_dz_2 = 24 ; // dimension case 2
case_dz_3 = 23 ; // dimension case 3
case_dz_4 = 22 ; // dimension case 4

// contrefort sur le côté
contrefort_cote_x=plaque_x+ 20 ;
contrefort_cote_z=case_dz_1;
contrefort_cote_y=epaisseur*1.05;
// diametre fixation vis contrefort côté
contrefort_vis_d = 2.5;
// contrefort supérieur
contrefort_dessus_x=epaisseur*1.05;
contrefort_dessus_z=contrefort_cote_z;

clips_x = 1.5; // largeur clips
clips_y = 0.5 ; // epaisseur clips
clips_dy = 1 ; // debordement clips
clips_z = servo_dz + servo_z + clips_dy ; // hauteur totale clips
clips_i = 60 ; // angle clips

module finger_taquet()
{
    translate([finger_taquet_dx,0,-finger_z/2+finger_taquet_z/2-0.01])
    difference()
    {
            union()
            {
                // encoche pour le taquet du servo-moteur
                cube([finger_taquet_x,finger_taquet_y,finger_taquet_z],center=true);
                // fente pour le clipsage
                translate([-5+finger_taquet_x/2,0,0])
                    cube([10,finger_taquet_fente_y,finger_z*3], center=true);
            }
            // deux troncs de cone pour se "prendre" sur le trou du taquet du servo-moteur
            translate([0,0,finger_taquet_dz+(-finger_taquet_z+finger_taquet_trou_d1)/2])
            union()
            {
                translate([0,(finger_taquet_y-finger_taquet_trou_y+0.05)/2,0]) 
                    rotate([90,0,0])
                        cylinder(h=finger_taquet_trou_y,d1=finger_taquet_trou_d1,d2=finger_taquet_trou_d2,
                            center=true);
                translate([0,-(finger_taquet_y-finger_taquet_trou_y+0.05)/2,]) 
                    rotate([-90,0,0])
                        cylinder(h=finger_taquet_trou_y,d1=finger_taquet_trou_d1,d2=finger_taquet_trou_d2,
                            center=true);
            }
    }

}
module finger(dx)
{
    // doigt avec une encoche pour appuyer la corde
    translate([dx/2,finger_dy,0])
        difference()
        {
            //doigt
            cube([finger_x + dx ,finger_y,finger_z],center=true);
            // - corde
            translate([(dx+finger_x )/2+2.7,0,0]) 
                cylinder(2*finger_y,d=7,center=true);
            // - encoche
            translate([-dx/2,0,0]) 
                finger_taquet();
        }
}
module servo(dx,pos)
{
    if ( servo_dessine == 1)
    // servomoteur
    translate([0,0, servo_z/2 ]) 
        union()
        {
            // servo hors fixation
            cube([servo_x,servo_y,servo_z], center=true);
            // les engrenages
            translate([-servo_x/2-servo_debordement_x/2,0,3*servo_z])
                cube([1.2*servo_debordement_x,servo_y,6*servo_z], center=true);
            // le bloc translateur
            translate([translateur_dx,translateur_dy,servo_z/2+ translateur_z/2])
               cube([translateur_x,translateur_y,translateur_z], center=true);
            // le moteur
            translate([moteur_dx,moteur_dy,servo_z/2+ moteur_d/2])
                rotate([0,90,0])
                    cylinder(h=moteur_x , d1=moteur_d, d2=moteur_d,center=true);
            // 4 plaquettes de fixation
            translate([-servo_px,-servo_y/2-servo_dy/2,0]) 
                cube([servo_dx,servo_dy,servo_z],center=true);
            translate([servo_px,-servo_y/2-servo_dy/2,0]) 
                cube([servo_dx,servo_dy,servo_z],center=true);
            translate([servo_px,servo_y/2+servo_dy/2,0]) 
                cube([servo_dx,servo_dy,servo_z],center=true);
            translate([-servo_px,servo_y/2+servo_dy/2,0]) 
                cube([servo_dx,servo_dy,servo_z],center=true);
            if ( finger_dessine == 1)
                // finger
                translate([finger_x/2+pos,0,servo_z/2+finger_z/2+finger_dz]) finger(dx);
        }
}
module clips()
{
    union()
    {
        // tige clips
        translate([0,clips_y/2,clips_z/2])
            cube([clips_x,clips_y,clips_z ],center=true);
        // crochet clips
        translate([0,-clips_dy/2,clips_z - clips_dy/2])
        difference()
        {
            cube([clips_x,clips_dy,clips_dy],center=true);
            translate([-clips_x,clips_dy/2-(clips_dy/2)/(tan(clips_i))-(clips_dy*2)/(tan(clips_i)),-clips_dy*2])
            rotate([clips_i,0,0])
                cube([clips_x*2,clips_dy*4,clips_dy*4],center=false);
        }
    }
}
module pico()
{
    union()
    {
        // picot positionnement
        translate([0,0,servo_z])
            cylinder(servo_dz+servo_z+1  ,d=servo_entraxe_d,center=true);
        // embase 
        translate([0,0,servo_dz/2])
            cylinder(servo_dz,d=servo_entraxe_d * 2,center=true);
    }
}
module porte_servo(dx,pos)
{
    // fixation servo sur la plaque
    union()
    {
        // 4 picots de fixation
        translate([-servo_entraxe_x/2,-servo_entraxe_y/2,0]) 
            pico();
        translate([servo_entraxe_x/2,servo_entraxe_y/2,0]) 
            pico();
        translate([servo_entraxe_x/2,-servo_entraxe_y/2,0]) 
            pico();
        translate([-servo_entraxe_x/2,servo_entraxe_y/2,0]) 
            pico();
    }
    union()
    {
        translate([-clips_x,-servo_y/2,0]) 
            rotate([0,0,180]) clips();
        translate([clips_x,servo_y/2,0]) 
            rotate([0,0,0]) clips();
    }
    translate([0,0,servo_dz]) servo(dx,pos); 
}
module groupe_servo(corde_dy1,corde_dy2)
{
    // les 6 servos
    union()
    {
        translate([0,-finger_dy,epaisseur/2])
           union()
            {
               translate([0,corde_dy1*(-2.5),0]) 
                    porte_servo(0,$t==0?finger_off:finger_on) ;
               translate([-servo_decalage_x,corde_dy1*(-0.5),0]) 
                    porte_servo(servo_decalage_x,$t==0?finger_off:finger_on) ;
               translate([0,corde_dy1*(1.5),0]) 
                    porte_servo(0,$t==0?finger_on:finger_off) ;
           }
        translate([0,finger_dy,-epaisseur/2]) 
            rotate([180,0,00]) 
               union()
                {
                   translate([0,corde_dy2*(-2.5),0]) 
                        porte_servo(0,$t==0?finger_off:finger_on) ;
                   translate([-servo_decalage_x,corde_dy2*(-0.5),0]) 
                        porte_servo(servo_decalage_x,$t==0?finger_on:finger_off) ;
                   translate([0,corde_dy2*(1.5),0]) 
                        porte_servo(0,$t==0?finger_on:finger_off) ;
                }
    }
}

module contrefort_cote()
{
    translate([contrefort_cote_x/2,0,0]) 
    difference()
       {
        cube([contrefort_cote_x,contrefort_cote_y,contrefort_cote_z],center=true);
        //trous de vis
        translate([contrefort_cote_x/2 - 1.2*contrefort_vis_d,0,-contrefort_cote_z/3])
           rotate([90,0,0])
                cylinder(contrefort_cote_y*4,d=contrefort_vis_d,center=true);
        translate([contrefort_cote_x/2 - 1.2*contrefort_vis_d,0,contrefort_cote_z/3]) 
            rotate([90,0,0])
                cylinder(contrefort_cote_y*4,d=contrefort_vis_d,center=true);
        }
}
module contrefort_dessus(plaque_y)
{
    translate([-plaque_x/2-epaisseur/2,0,0]) 
        cube([contrefort_dessus_x,plaque_y+2*epaisseur,contrefort_dessus_z],center=true);
}
module contrefort_dessous(plaque_y)
{
    translate([(plaque_x+epaisseur)/2,0,0]) 
        cube([contrefort_dessus_x,plaque_y+2*epaisseur,3*epaisseur],center=true);
}
module plaque_support(plaque_y)
{
    // plaques de support
    cube([plaque_x,plaque_y,plaque_z],center=true);
}
module plaque_renfort(plaque_y)
{
     union()
    {
        // plaque de dessus
        *contrefort_dessus(plaque_y);
        *contrefort_dessous(plaque_y);
    }
}
module plaque_cote(plaque_y)
{
    // plaques de côté
    union()
    {
        translate([-(plaque_x+epaisseur)/2-0.05,plaque_y/2+epaisseur/2,0]) contrefort_cote();
        translate([-(plaque_x+epaisseur)/2-0.05,-plaque_y/2-epaisseur/2,0]) contrefort_cote();
    }
}
module manche(y,dy)
{
    if ( manche_dessine == 1)
    translate([38.5,0,0])
    union()
    {
        cube([30,y-2*carre,contrefort_cote_z],center=true);
        translate([-30/2+carre/2,0,0])
            cube([carre,y,contrefort_cote_z],center=true);
        translate([-18,0,0])
        union()
        {
            translate([0,dy*(-2.5),0])
                cylinder(h=contrefort_cote_z,d=4,center=true);
            translate([0,dy*(-1.5),0]) 
                cylinder(h=contrefort_cote_z,d=3.5,center=true);
            translate([0,dy*(-0.5),0]) 
                cylinder(h=contrefort_cote_z,d=3,center=true);
            translate([0,dy*(0.5),0]) 
                cylinder(h=contrefort_cote_z,d=3,center=true);
            translate([0,dy*(1.5),0]) 
                cylinder(h=contrefort_cote_z,d=2.5,center=true);
            translate([0,dy*(2.5),0]) 
                cylinder(h=contrefort_cote_z,d=2,center=true);
        }
    }
}
module bloc(corde_dy1,corde_dy2,plaque_y)
{
    union()
    {
        manche(plaque_y,(corde_dy1+corde_dy2)/2);
        groupe_servo(corde_dy1,corde_dy2);
        // plaques de contrefort servo_x + servo_debordement_x + servo_decalage_x + epaisseur
       translate([(-servo_debordement_x - servo_decalage_x )/2 -epaisseur/8,0,0])
        {
            plaque_support(plaque_y) ;
            plaque_renfort(plaque_y) ;
            plaque_cote(plaque_y) ;
        }

    }
}

module tout()
{
    // Assemblage des blocs par case :
    translate([0,0,0]) 
        difference()
        {
            bloc(corde_dy_1,corde_dy_1,plaque_y_0) ;
            // enleve la moitié inférieure qui tomberait sur la case "zéro" :
            translate([0,0,-(case_dz_1+epaisseur)/2]) 
                cube([200,200,case_dz_1],center=true); 
        }
    translate([0,0,case_dz_1]) 
            bloc(corde_dy_1,corde_dy_2,plaque_y_1) ;
    translate([0,0,case_dz_1+case_dz_2]) 
            bloc(corde_dy_2,corde_dy_3,plaque_y_2) ;
    translate([0,0,case_dz_1+case_dz_2+case_dz_3]) 
            bloc(corde_dy_3,corde_dy_4,plaque_y_3) ;
    translate([0,0,case_dz_1+case_dz_2+case_dz_3+case_dz_4]) 
        difference()
        {
            bloc(corde_dy_4,corde_dy_4,plaque_y_4) ;
            // enleve la moitié supérieure qui tomberait sur la case "5"
            translate([0,0,(case_dz_1+epaisseur)/2])
                cube([200,200,case_dz_1],center=true);
        }
}

module fingers()
{
for(i=[1:(4*4)])
    translate([0,(i-1)*(finger_y+1),0])
        finger(0);
for(i=[1:(4*2)])
    translate([0,-i*(finger_y+1),0])
        finger(servo_decalage_x);
translate([0,3*(finger_y+1),0])
    rotate([90,0,0])
        cylinder(h=(4*4+4*2-1)*(finger_y+1),d1=1,d2=1,center=true);
}
module test()
{
    difference()
    {
        union()
        {
            bloc(corde_dy_1,corde_dy_2,plaque_y_1) ;
            *union()
            {
            translate([0,0,finger_z+ 2*epaisseur]) 
                finger(0);
            translate([0,0,-finger_z- 2*epaisseur]) 
                finger(-servo_decalage_x);
            translate([0,finger_dy,0])
                rotate([0,0,0])
                    cylinder(h=8*epaisseur,d1=1,d2=1,center=true);
            }
        }
        // decuope pour voir dedans
        //translate([0,504,0]) cube([1000,1000,1000],center=true);
    }
}
*tout();
fingers();
*test();
*porte_servo(0,0);
*rotate([0,0,180]) clips();

