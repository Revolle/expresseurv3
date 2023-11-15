$fn = 10;

/*
led carré
capteur_x = 4.44;
capteur_z = 5.72;
capteur_y = 1.57;
capteur_dzz = 1.22 ;
capteur_dz = capteur_z/2 - capteur_dzz ;
capteur_dx_patte = 2.54/2 ;
capteur_z_patte = 8 ;
capteur_d = 1.5 ;
capteur_dz_c = capteur_dzz + capteur_d/2 ;
capteur_z_empreinte = capteur_z + capteur_z_patte ;
capteur_e_patte = 0.5 ;
*/
// led ronde
/*
led_d = 3 ;
led_d2 = 4 ;
led_h = 4.2 ;
led_h2 = 1 ;
*/
led_d = 5 ;
led_d2 = 8 ;
led_support_d = 7 ;
led_h = 8 ;
led_h2 = 1 ;
led_e_patte = 0.5; 
led_z_patte1 = 3 ;
led_z_patte2 = 20 ;
led_p = 2.54;

dx_v = 13 ;
boitier_x = 75 ;
boitier_y = 110 ;
boitier_z = 30 ;

vis_d = 2.6 ;
vis_l = 17 ;
vis_d2 = 5.8 ;
vis_l2 = 2.5 ;

doigt_x = 25 ; 
doigt_n = 4 ;
doigt_dz = doigt_x/2 -0.5 ;
captages_dz = boitier_z /2 -4.5 ;
captages_dy = 0 ;

captage_fy = 1.2 ;
captage_d = 1.4* doigt_x ;
captage_y =  captage_d * captage_fy  ;

e = 4 ;
clavier_x = doigt_n * doigt_x+ 6 ;
clavier_y = captage_y + 2*(led_h+led_h2+led_z_patte1+2*led_e_patte) ;
clavier_z = 10 ;

ci_x = boitier_x - 2*e ;
ci_y = clavier_y ;
ci_z = 12 ;
ci_ddz = 3 ;

embase_x = 75 ;
embase_y = 120 ;
embase_z = 25 ;

teensy_x = 18.5;
teensy_y = 36;
teensy_z = 5 ;
teensy_x2 = 12;
teensy_y2 = 20;
teensy_z2 = 7.5 ;
teensy_dx = -15 ;
teensy_dy = boitier_y/2 - teensy_y/2 -e -3 ;
teensy_p = 0.6 ;
teensy_pz = 6 ;
teensy_dp = 2.54 ;

s2_x =34.5 ;
s2_y = 24.2;
s2_z = 5 ;
s2_y2 = 5 ;
s2_z2 = 12 ;
s2_dx = 32 ;
s2_dy = -5 ;

potarl_x = 35.2;
potarl_y = 9.8;
potarl_z = 6.5;
potarl_zz = 11 ;
potarl_b_xx = 28 ;
potarl_b_x = 5 ;
potarl_b_y = 3 ;
potarl_b_z = 10 ;
potar_dx1 = 9 ;
potar_dx2 = 3 ;

bouton_nb = 3 ;
bouton_d1 = 10 ;
bouton_d2 = 6 ; 
bouton_h1 = 20 ;
bouton_h2 = 6 ;
bouton_e = 20;

jack_d1 = 8 ;
jack_h1 = 20 ;
jack_d2 = 6 ;
jack_h2 = 3 ;
jack_d3 = 8 ;
jack_h3 = 1;

led_nb = 3 ;
led_e = led_d2 + 1 ;
led_dx = - ci_x / 4 + 2 ;

 
transperce = 0 ;
empreinte = 1 ;

module vis()
{
    translate([0,0,-vis_l/2])
        cylinder(d = vis_d, h = vis_l, center = true);
    translate([0,0,-vis_l2/2])
        cylinder(d1 = vis_d, d2 = vis_d2 , h = vis_l2, center = true);
    
}
module boulon(d_boulon,h_boulon)
{
    difference()
    {
        cube([d_boulon,d_boulon,h_boulon],center=true);
        dd=sqrt(d_boulon*d_boulon/2);
        translate([dd,dd,0])
            rotate([0,0,45])
                cube([d_boulon,d_boulon,h_boulon],center=true);
        translate([-dd,dd,0])
            rotate([0,0,45])
                cube([d_boulon,d_boulon,h_boulon],center=true);
        translate([-dd,-dd,0])
            rotate([0,0,45])
                cube([d_boulon,d_boulon,h_boulon],center=true);
        translate([dd,-dd,0])
            rotate([0,0,45])
                cube([d_boulon,d_boulon,h_boulon],center=true);
    }	
}
module bouton()
{
    translate([0,0,-bouton_h1/2])
        cylinder(d=bouton_d1,h=bouton_h1,center=true);
    translate([0,0,bouton_h2/2])
        cylinder(d=bouton_d2,h=bouton_h2,center=true);
   *translate([0,0,- bouton_hboulon/2])
        boulon(bouton_dboulon,bouton_hboulon);
}
module s2()
{
    translate([0,0,s2_z/2])
        cube([s2_x,s2_y,s2_z],center = true);
    translate([0,s2_y/2-s2_y2/2,s2_z + s2_z2/2])
        cube([s2_x,s2_y2,s2_z2],center = true);
}
module teensy()
{
    translate([0,0,teensy_z/2])
        cube([teensy_x,teensy_y,teensy_z],center = true);
    translate([0,teensy_y/2+teensy_y2/2,teensy_z2/2])
        cube([teensy_x2,teensy_y2,teensy_z2],center = true);
    for(i=[-3 : 1 : 3 ])
    {
    translate([teensy_dp * i ,- teensy_y/2 + 1,-teensy_pz/2])
        cube([teensy_p,teensy_p,teensy_pz],center=true);
    }
    for(i=[7 : 1 : 10 ])
    {
    translate([teensy_dp*3,- teensy_y/2 + 1 + i * teensy_dp ,-teensy_pz/2])
        cube([teensy_p,teensy_p,teensy_pz],center=true);
    }
    for(i=[7 : 1 : 10 ])
    {
    translate([-teensy_dp*3,- teensy_y/2 + 1 + i * teensy_dp ,-teensy_pz/2])
        cube([teensy_p,teensy_p,teensy_pz],center=true);
    }

}
module potarl()
{
    if ( empreinte == 1 )
    {
        translate([0,0,-potarl_zz/2])
            cube([potarl_x,potarl_y,potarl_zz],center = true);
        translate([0,0,potarl_b_z/2])
            cube([potarl_b_xx,potarl_b_y,potarl_b_z],center = true);
    }
    else
    {
        translate([0,0,-potarl_z/2])
            cube([potarl_x,potarl_y,potarl_z],center = true);
        translate([0,0,potarl_b_z/2])
            cube([potarl_b_x,potarl_b_y,potarl_b_z],center = true);
    }
}
module jack()
{
    translate([0,0,-jack_h1/2-jack_h2-jack_h3])
        cylinder(d=jack_d1, h = jack_h1 , center = true);
    translate([0,0,-jack_h2/2-jack_h3])
        cylinder(d=jack_d2, h = jack_h2 , center = true);
    translate([0,0,-jack_h3/2])
        cylinder(d=jack_d3, h = jack_h3 , center = true);
}
module led()
{
    // corps optique
    translate([0,0,-led_h/2])
        cylinder(d = led_d, h = led_h, center = true);
    // embase
    translate([0,0,-led_h2/2 -led_h])
        cylinder(d = led_d2, h = led_h2, center = true);
    // pattes
    translate([led_p /2 ,0,-led_h2 -led_h - led_z_patte2  /2])
        cube([led_e_patte,led_e_patte,led_z_patte2],center=true);
    translate([-led_p /2 ,0,-led_h2 -led_h - led_z_patte2 /2])
        cube([led_e_patte,led_e_patte,led_z_patte2],center=true);
    if ( empreinte == 1 )
    {
        translate([0,0,-led_z_patte2/2 -led_h - led_h2])
            cylinder(d = led_d2, h = led_z_patte2, center = true);
    }
}
module led_plie()
{
    // corps optique
    translate([0,0,-led_h/2])
        cylinder(d = led_d, h = led_h, center = true);
    // embase
    translate([0,0,-led_h2/2 -led_h])
        cylinder(d = led_d2, h = led_h2, center = true);
    // pattes
    translate([led_p /2 ,0,-led_h2 -led_h - led_z_patte1 /2])
        cube([led_e_patte,led_e_patte,led_z_patte1],center=true);
    translate([led_p /2 ,led_z_patte2 / 2 - led_e_patte / 2,-led_h2 -led_h - led_z_patte1])
        cube([led_e_patte,led_z_patte2,led_e_patte],center=true);
    translate([-led_p /2 ,0,-led_h2 -led_h - led_z_patte1 /2])
        cube([led_e_patte,led_e_patte,led_z_patte1],center=true);
    translate([-led_p /2 ,led_z_patte2 / 2 - led_e_patte / 2,-led_h2 -led_h - led_z_patte1])
        cube([led_e_patte,led_z_patte2,led_e_patte],center=true);
    if ( empreinte == 1 )
    {
        // empreinte pour les pattes 
        translate([0 ,(led_z_patte2 )/ 2  ,-led_h2 -led_h - (led_z_patte1+ 2 * led_e_patte)/2])
            cube([led_d2 ,led_z_patte2  ,led_z_patte1+ 2 * led_e_patte ],center=true);
        translate([0,0,-(led_z_patte1+ 2 * led_e_patte)/2 -led_h - led_h2])
            cylinder(d = led_d2, h = led_z_patte1+ 2 * led_e_patte, center = true);
    }
    
}

module captage(rot)
{
    // emetteur
    translate([0,-captage_y / 2   ,0 ])
        rotate([0,rot,0])
            rotate([-90,0,0])
                led_plie();
    // recepteur
    translate([0,captage_y / 2 ,0])
        rotate([0,rot,0])
            rotate([90,180,0])
                led_plie();
    // tube optique
    rotate([90,0,0])
        cylinder(d=led_d , h = captage_y + transperce*100 ,center= true); 

    translate([0,0,captage_d/2-led_d/2-1])
        difference()
        {
            scale([1,captage_fy,1])
                sphere(d = captage_d );
            translate([doigt_x*3/2+1,0,0])
                cube([2*doigt_x,2*doigt_x,2*doigt_x],center=true);
            translate([-doigt_x*3/2-1,0,0])
                cube([2*doigt_x,2*doigt_x,2*doigt_x],center=true);
            translate([0,0,doigt_x])
                cube([2*doigt_x,2*doigt_x,2*doigt_x],center=true);
        }
}
module captages()
{
    for(i=[0 : 1 : doigt_n - 1 ])
    {
        translate([i * doigt_x - doigt_x * (doigt_n - 1 )/ 2,0,0])
            // capteur emetteur/recepteur
            captage(i * (120/doigt_n)  - (120/doigt_n)*(doigt_n - 1)/2 ) ;
    }
}
module composants()
{
        union()
        {
            // boutons
            for(i=[0 : 1 : bouton_nb - 1 ])
            {
                translate([-boitier_x/2 ,-20+ i * bouton_e - bouton_e * (bouton_nb - 1 )/ 2,0])
                {
                    rotate([0,-90,0])
                        bouton();
                }
            }
            // potar
            translate([dx_v,-boitier_y/2 + e/2 ,potarl_y/2  +1])
                rotate([90,0,0])
                    potarl();
            translate([dx_v,-boitier_y/2  + e/2 , - potarl_y/2  - 1 ])
                rotate([90,0,0])
                    potarl();

            // leds
            for(i=[0 : 1 : led_nb - 1 ])
            {
                translate([i * led_e - led_e * (led_nb - 1 )/ 2 +dx_v ,-boitier_y/2 + 25  ,boitier_z/2])
                    rotate([0,0,0])
                        cylinder(d=led_support_d,h=12,center= true);
            }

            // teensy
            translate([teensy_dx,teensy_dy,-boitier_z/2+teensy_pz+2])
                teensy();
            // s2
            *translate([boitier_x/2-e - s2_y/2 ,9  ,-boitier_z/2+5])
                rotate([0,0,-90])
                s2();
            // jack
            translate([23,boitier_y/2 ,0])
                rotate([-90,0,0])
                    jack();
            
        }
}

module boitier_clavier()
{
    difference()
    {
        union()
        {
            hull()
            {
                translate([clavier_x/2+e/2,clavier_y/2+e/2,clavier_z/2-e/2])
                    sphere(d=e);
                translate([-clavier_x/2-e/2,clavier_y/2+e/2,clavier_z/2-e/2])
                    sphere(d=e);
                translate([clavier_x/2+e/2,-clavier_y/2-e/2,clavier_z/2-e/2])
                    sphere(d=e);
                translate([-clavier_x/2-e/2,-clavier_y/2-e/2,clavier_z/2-e/2])
                    sphere(d=e);
                translate([clavier_x/2+e/2,clavier_y/2+e/2,-clavier_z/2+e/2])
                    sphere(d=e);
                translate([-clavier_x/2-e/2,clavier_y/2+e/2,-clavier_z/2+e/2])
                    sphere(d=e);
                translate([clavier_x/2+e/2,-clavier_y/2-e/2,-clavier_z/2+e/2])
                    sphere(d=e);
                translate([-clavier_x/2-e/2,-clavier_y/2-e/2,-clavier_z/2+e/2])
                    sphere(d=e);
            }
        }
        translate([0,0,0])
            cube([clavier_x,clavier_y,clavier_z],center = true);

   }
}
module boitier()
{
    difference()
    {
        // boitier global
        union()
        {
            hull()
            {
                translate([boitier_x/2-e/2,boitier_y/2-e/2,boitier_z/2-e/2])
                    sphere(d=e);
                translate([-boitier_x/2+e/2,boitier_y/2-e/2,boitier_z/2-e/2])
                    sphere(d=e);
                translate([boitier_x/2-e/2,-boitier_y/2+e/2,boitier_z/2-e/2])
                    sphere(d=e);
                translate([-boitier_x/2+e/2,-boitier_y/2+e/2,boitier_z/2-e/2])
                    sphere(d=e);
                translate([boitier_x/2-e/2,boitier_y/2-e/2,-boitier_z/2+e/2])
                    sphere(d=e);
                translate([-boitier_x/2+e/2,boitier_y/2-e/2,-boitier_z/2+e/2])
                    sphere(d=e);
                translate([boitier_x/2-e/2,-boitier_y/2+e/2,-boitier_z/2+e/2])
                    sphere(d=e);
                translate([-boitier_x/2+e/2,-boitier_y/2+e/2,-boitier_z/2+e/2])
                    sphere(d=e);
            }
            translate([0,(boitier_y - clavier_y)/2-e,clavier_z/2 + boitier_z/2])
                boitier_clavier();
            translate([0,boitier_y/2-e/2,boitier_z/2])
                cube([boitier_x-e,e,e],center=true);
            translate([0,boitier_y/2-3*e/2-clavier_y,boitier_z/2])
                cube([boitier_x-e,e,e],center=true);
        }
        cube([boitier_x-2*e,boitier_y-2*e,boitier_z-2*e],center= true);
        translate([0,(boitier_y - clavier_y)/2-e,boitier_z/2])
            cube([boitier_x-2*e,clavier_y,2*e],center= true);
        
              
    }
    // support teensy
    translate([teensy_dx,teensy_dy,-boitier_z/2+teensy_pz/2])
        cube([teensy_x+3,teensy_y+3,teensy_pz],center = true);
    // support s2 
    translate([boitier_x/2-e - s2_y/2 - (s2_y/2-4.5 ),9 + (s2_x/2-4.5)  ,-boitier_z/2+10/2+0.5])
        cylinder(d=3.4,h = 10, center = true);

}
/*
                // CI
                translate([0,0,-ci_ddz])
                    cube([ci_x,ci_y,ci_z],center=true);
                // vis
                translate([26,boitier_y/2,boitier_z/2-5])
                    rotate([-90,0,0])
                        vis() ;
                translate([-26,boitier_y/2,boitier_z/2-5])
                    rotate([-90,0,0])
                        vis() ;
                translate([26,-boitier_y/2,boitier_z/2-5])
                    rotate([90,0,0])
                        vis() ;
                translate([-26,-boitier_y/2,boitier_z/2-5])
                    rotate([90,0,0])
                        vis() ;
            }
            translate([0,0,-ci_ddz])
            {
                // encastrement teensy
                translate([teensy_dx,ci_y/2-teensy_y/2 ,-ci_z/2+teensy_z/2])
                    cube([teensy_x+4,teensy_y+4,teensy_z],center=true);
                // encastrement s2
                translate([s2_dx,s2_dy ,-ci_z/2+s2_z/2])
                    cube([s2_x+4,s2_y+4,s2_z],center=true);
                // support plateau
                translate([ci_x/2-e/2,ci_y/2-e/2 ,0])
                     cube([e,e,ci_z],center=true);
                translate([ci_x/2-e/2,-ci_y/2+e/2 ,0])
                     cube([e,e,ci_z],center=true);
                translate([-ci_x/2+e/2,ci_y/2-e/2 ,0])
                     cube([e,e,ci_z],center=true);
                translate([-ci_x/2+e/2,-ci_y/2+e/2 ,0])
                     cube([e,e,ci_z],center=true);
            }
        }
        translate([0,0,-ci_ddz])
            composants();
    }
}
*/
module clavier()
{
    difference()
    {
        cube([clavier_x,clavier_y,clavier_z],center = true);
        // capteurs
        translate([0,0,0])
            captages();
    }
}
*potarl() ;
*vis();
*s2() ;
*teensy() ;
*boulon(10,2);
*bouton();
*jack();
*led();
*led_plie() ;
*capteur();
*boitier();
*captage();
*captages();
difference()
{
    union()
    {
    boitier();
    composants();
    }
*    composants();
    *translate([210,0,0])
            cube([400,400,400],center=true);
}
//translate([0,0,-ci_ddz])
*embase() ;
*boitier_clavier();
*translate([0,(boitier_y - clavier_y)/2-e,clavier_z/2 + boitier_z/2+13])
    clavier();
