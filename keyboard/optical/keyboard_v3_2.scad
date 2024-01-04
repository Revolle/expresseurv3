$fn = 50;


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

vis_d = 2.6 ;
vis_l = 12 ;
vis_d2 = 5.8 ;
vis_l2 = 2.5 ;

doigt_x = 22 ; 
doigt_n = 3 ;
doigt_dz = doigt_x/2 -0.5 ;
//captages_dz = boitier_z /2 -4.5 ;
captages_dy = 0 ;

captage_fy = 1.2 ;
captage_d = 1.4* doigt_x ;
captage_y =  captage_d * captage_fy  ;

e = 3 ;
jeu = 0.2;

boitier_x = 72 ;
boitier_y = 113 ;
boitier_z = 29  ;

clavier_xneeded = doigt_n * doigt_x ;
echo ("clavier_xneeded = " ,clavier_xneeded);
clavier_xmini = boitier_x - 2 * e ;
echo ("clavier_xmini = " ,clavier_xmini);
clavier_x = clavier_xneeded < clavier_xmini  ? clavier_xmini : clavier_xneeded  ;
clavier_y = captage_y + 2*(led_h+led_h2+led_z_patte1+2*led_e_patte) ;
clavier_z = 10 ;
clavier_dy = -clavier_y/2 + boitier_y/2 - e ;
clavier_dz = -clavier_z/2 + boitier_z / 2 ;
clavier_vis_dy = 8 ;

ci_x = boitier_x - 2*e ;
ci_y = clavier_y ;
ci_z = 12 ;
ci_ddz = 3 ;

embase_x = 75 ;
embase_y = 120 ;
embase_z = 25 + clavier_z  ;

teensy_x = 17.78;
teensy_y = 36;
teensy_z = 5 ;
teensy_e = 1.57 ;
teensy_x2 = 12;
teensy_y2 = 20;
teensy_z2 = 7.5 ;
teensy_dx = 14 ;
teensy_dy = boitier_y/2 - teensy_y/2 -e -7 ;
echo ( "teensy_dy=", teensy_dy, " teensy-dx=", teensy_dx);
teensy_dz = 0 ;
teensy_p = 0.6 ;
teensy_pz = 6 ;
teensy_dp = 2.54 ;
teensy_led_h = 20 ;
teensy_led_dx = 2.54 ;
teensy_led_dy = -teensy_y/2 + 2.54 ;

s2_x =34.5 ;
s2_y = 24.2;
s2_z = 5 ;
s2_y2 = 5 ;
s2_z2 = 9 ;
s2_dx = 10;
s2_dy = -11 ;
s2_dz = -6 ;
s2_e = 1.75;
s2_trou_d = 3.5;
s2_trou_dx = -s2_x /2 +4 ;
s2_trou_dy = -s2_y/2 + 4 ;


potarl_x = 35.2;
potarl_y = 9.8;
potarl_z = 6.5;
potarl_zz = 11 ;
potarl_b_xx = 28 ;
potarl_b_x = 5 ;
potarl_b_y = 3 ;
potarl_b_z = 10 ;
potarl_dy = -boitier_y/2 + e + 7 + potarl_x/2 ;
potarl_dxx = 4 ;
potarl_dx = boitier_x/2  - e/2 - potarl_dxx ;
potarl_protect_z = e ;

potarr_bd =4 ;
potarr_bh =8 ;
potarr_d = 7 ;
potarr_h = 5 ;
potarr_d2 = 17 ;
potarr_h2 = 9;
potarr_x = 17 ;
potarr_y = 15 ;

potarrg_bd =6 ;
potarrg_bh =8 ;
potarrg_d = 10 ;
potarrg_h = 7 ;
potarrg_d2 = 20 ;
potarrg_h2 = 10;
potarrg_x = 17 ;
potarrg_y = 16 ;

bouton_nb = 2 ;
bouton_d1 = 7 ;
bouton_d2 = 6 ; 
bouton_h1 = 20 ;
bouton_h2 = 5 ;
bouton_e = 23;
bouton_dy = -29 ;
bouton_dz = -6 ;
bouton_dboulon = 10+0.2 ;
bouton_hboulon = 1.5 ;

jack_d1 = 8 ;
jack_h1 = 20 ;
jack_d2 = 6 ;
jack_h2 = 3 ;
jack_d3 = 8 ;
jack_h3 = 1;
jack_dx = -18 ;

led_nb = 2 ;
led_e = led_d2 + 1 ;
led_dx = 0 ;
led_dy = 9 ;

picot_d = 3 ;
picot_h = 3 ;
 
transperce = 2 ;
empreinte = 1 ;

module vis()
{
    rotate([0,180,0])
    {
        translate([0,0,-vis_l/2])
            cylinder(d = vis_d, h = vis_l, center = true);
        translate([0,0,-vis_l2/2])
            cylinder(d1 = vis_d, d2 = vis_d2 , h = vis_l2, center = true);
    }
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
    translate([0,0,- bouton_hboulon/2])
        boulon(bouton_dboulon,bouton_hboulon);
}
module s2()
{
    translate([0,0,0])
    {
        difference()
        {
                translate([0,0,0])
                    cube([s2_x,s2_y,s2_e],center = true);
            translate([s2_trou_dx,s2_trou_dy,0])
                cylinder(d = s2_trou_d, h =20 , center = true);
        }
        translate([0,s2_y/2-s2_y2/2,s2_e + s2_z2/2])
            cube([s2_x,s2_y2,s2_z2],center = true);
    }
}
module teensy()
{
    cube([teensy_x,teensy_y,teensy_e],center = true);

    // USB plug
    translate([0,teensy_y/2+teensy_y2/2,teensy_e/2+2.5/2])
        cube([teensy_x2,teensy_y2,teensy_z2],center = true);

    // led 
    translate([teensy_led_dx, teensy_led_dy, teensy_led_h / 2])
        cube([3,3,teensy_led_h],center = true);
    // pattes
    
    for(i=[-3 : 1 : 3 ])
    {
    translate([teensy_dp * i ,- teensy_y/2 + 1,-teensy_pz/2])
        cube([teensy_p,teensy_p,teensy_pz],center=true);
    }
    for(i=[1 : 1 : 12 ])
    {
    translate([teensy_dp*3,- teensy_y/2 + 1 + i * teensy_dp ,-teensy_pz/2])
        cube([teensy_p,teensy_p,teensy_pz],center=true);
    }
    for(i=[1 : 1 : 12 ])
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
module potarr()
{
    translate([0,0,potarr_h/2])
        cylinder(d=potarr_d,h = potarr_h, center = true);
    translate([0,0,-potarr_h2/2])
        cube([potarr_d2,potarr_d2,potarr_h2], center = true);
    translate([0,-potarr_y/2,-potarr_h2/2])
        cube([potarr_x, potarr_y,3],center = true);
    translate([0,0,potarr_bh/2+potarr_h])
        cylinder(d=potarr_bd,h = potarr_bh, center = true);
}
module potarrg()
{
    translate([0,0,potarrg_h/2])
        cylinder(d=potarrg_d,h = potarrg_h, center = true);
    translate([0,0,-potarrg_h2/2])
        cylinder(d=potarrg_d2,h=potarrg_h2, center = true);
    translate([0,-potarrg_y/2,-potarrg_h2/2])
        cube([potarrg_x, potarrg_y,3],center = true);
    translate([0,0,potarrg_bh/2+potarrg_h])
        cylinder(d=potarrg_bd,h = potarrg_bh, center = true);
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
module led_boitier()
{
    // corps optique
    translate([0,0,-led_h/2])
        cylinder(d = led_d, h = led_h, center = true);
    // embase
    translate([0,0,-led_h2/2 -led_h])
        cylinder(d = led_d2, h = led_h2, center = true);
}
module led()
{
    led_boitier() ;
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
    led_boitier() ;
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
        //rotate([0,rot,0])
            rotate([-90,0,0])
                led();
    // recepteur
    translate([0,captage_y / 2 ,0])
        //rotate([0,rot,0])
            rotate([90,180,0])
                led();
    // tube optique
    rotate([90,0,0])
        cylinder(d=led_d , h = captage_y + transperce ,center= true); 

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
                translate([-boitier_x/2 ,bouton_dy + i * bouton_e - bouton_e * (bouton_nb - 1 )/ 2,bouton_dz])
                {
                    rotate([0,-90,0])
                        bouton();
                }
            }
            // potar
            translate([potarl_dx ,potarl_dy   ,boitier_z/3  - e -1 ])
                rotate([90,0,90])
                    potarl();
            translate([potarl_dx ,potarl_dy  ,-boitier_z/3  + e +1 ])
                rotate([90,0,90])
                    potarl();

            // leds
            for(i=[0 : 1 : led_nb - 1 ])
            {
                translate([i * led_e - led_e * (led_nb - 1 )/ 2 +led_dx,-boitier_y/2 + e + led_dy ,boitier_z/2])
                    rotate([0,0,0])
                        cylinder(d=led_support_d,h=12,center= true);
            }

            // teensy
            translate([teensy_dx,teensy_dy,teensy_dz ])
                rotate([0,180,0])
                    teensy();
            // s2
            *translate([s2_dx ,s2_dy  ,s2_dz])
                rotate([0,0,180])
                s2();
            // jack
            translate([jack_dx,boitier_y/2 ,-5])
                rotate([-90,0,0])
                    jack();
            
        }
}

module support_vis(l,d)
{
    translate([0,0,l/2])
    difference()
    {
        cube([2*e, 2*e,l],center= true);
        translate([0,0,0])
            cylinder(d=d,h = l +1,center=true);
    }
}
module clavier()
{
    difference()
    {
        cube([clavier_x,clavier_y,clavier_z],center = true);
        // capteurs
        translate([0,0,0])
            captages();
        // rigole de cablage
        translate([0,clavier_y/2-e/2,-clavier_z/2])
            cube([clavier_x,e,clavier_z],center=true);
        translate([0,-clavier_y/2+e/2,-clavier_z/2])
            cube([clavier_x,e,clavier_z],center=true);
    }
    // vis
    translate([boitier_x /2  - 2*e, clavier_y/2 - clavier_vis_dy , -clavier_z/2 -vis_l])
        support_vis(vis_l,vis_d);
    translate([boitier_x /2  - 2*e, -clavier_y/2 + clavier_vis_dy , -clavier_z/2 -vis_l])
        support_vis(vis_l,vis_d);
    translate([-boitier_x /2  + 2*e, clavier_y/2 - clavier_vis_dy , -clavier_z/2 -vis_l])
        support_vis(vis_l,vis_d);
    translate([-boitier_x /2  + 2*e, -clavier_y/2 + clavier_vis_dy , -clavier_z/2 -vis_l])
        support_vis(vis_l,vis_d);
    // support s2
    *translate([s2_dx-s2_trou_dx,s2_dy - s2_trou_dy - clavier_dy ,-clavier_z/2 -vis_l+2.5])
        support_vis(vis_l,vis_d);
    // support teensy
    translate([teensy_dx ,teensy_dy - clavier_dy ,-clavier_z/2 -1 ])
        cube([teensy_x+2,teensy_y+2,2],center = true);
}
module boitier()
{
    difference()
    {
        union()
        {
            // boitier global
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
            // enrobage clavier
            translate([0,clavier_dy,clavier_dz])
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
                translate([clavier_x/2+e/2,clavier_y/2+e/2,-clavier_z/2-e/2])
                    sphere(d=e);
                translate([-clavier_x/2-e/2,clavier_y/2+e/2,-clavier_z/2-e/2])
                    sphere(d=e);
                translate([clavier_x/2+e/2,-clavier_y/2-e/2,-clavier_z/2-e/2])
                    sphere(d=e);
                translate([-clavier_x/2-e/2,-clavier_y/2-e/2,-clavier_z/2-e/2])
                    sphere(d=e);
                
            }
        }
        // cavité dans boitier
        cube([boitier_x-2*e,boitier_y-2*e,boitier_z-2*e ],center = true);
        // clavier
        translate([0,clavier_dy,clavier_dz])
        {
            cube([clavier_x + jeu , clavier_y + jeu , clavier_z + jeu ],center= true);
        }
        // trou couvercle de fond
        translate([0,0,-50])
            cube([boitier_x-2*e,boitier_y-2*e,100],center = true);
       
    }
    // support vis
    translate([boitier_x /2  - 2*e, -boitier_y /2  + 2*e , -boitier_z/2+e])
            support_vis(vis_l,vis_d);
    translate([-boitier_x /2  + 2*e, -boitier_y /2  + 2*e , -boitier_z/2+e])
        support_vis(vis_l,vis_d);

    translate([boitier_x /2  - 2*e, -clavier_y/2 + clavier_vis_dy + clavier_dy , -boitier_z/2+e])
            support_vis(boitier_z - clavier_z - vis_l - e,vis_d+1.2);
    translate([boitier_x /2  - 2*e, +clavier_y/2 - clavier_vis_dy + clavier_dy , -boitier_z/2+e])
            support_vis(boitier_z - clavier_z - vis_l - e,vis_d+1.2);
    translate([-boitier_x /2  + 2*e, -clavier_y/2 + clavier_vis_dy + clavier_dy , -boitier_z/2+e])
            support_vis(boitier_z - clavier_z - vis_l - e,vis_d+1.2);
    translate([-boitier_x /2  + 2*e, +clavier_y/2 - clavier_vis_dy + clavier_dy , -boitier_z/2+e])
            support_vis(boitier_z - clavier_z - vis_l - e,vis_d+1.2);

    // protection potar
    translate([boitier_x / 2  , potarl_dy, 0])
    {
        difference()
        {
            hull()
            {
            translate([0 ,  -potarl_x / 2 - e , boitier_z/2 - e ])
                {
                cube([1, e, e], center = true);
                translate([potarl_protect_z,0,0])
                    sphere(d= e);
                }
            translate([0 ,  +potarl_x / 2 + e , boitier_z/2 - e ])
                {
                cube([1, e, e], center = true);
                translate([potarl_protect_z,0,0])
                    sphere(d= e);
                }
            translate([0 ,  -potarl_x / 2 - e , -boitier_z/2 + e ])
                {
                cube([1, e, e], center = true);
                translate([potarl_protect_z,0,0])
                    sphere(d= e);
                }
            translate([0 ,  +potarl_x / 2 + e , -boitier_z/2 + e ])
                {
                cube([1, e, e], center = true);
                translate([potarl_protect_z,0,0])
                    sphere(d= e);
                }
            }
        translate([potarl_b_z/2,0,0])
            cube([potarl_b_z,potarl_x+e, boitier_z-2.5*e], center = true);
        }
    }
    //renflement potar 
    translate([boitier_x/2 - 4 -e, potarl_dy,0])
        cube([8,potarl_x + e,boitier_z -2*e ],center = true);


}
module couvercle()
{
    difference()
    {
        cube([boitier_x-2*e - jeu, boitier_y - 2*e - jeu , e],center=true);
        
        translate([boitier_x /2  - 2*e, -boitier_y /2  + 2*e , - e/2])
                vis();
        translate([-boitier_x /2  + 2*e, -boitier_y /2  + 2*e , - e/2])
            vis();
    
        translate([boitier_x /2  - 2*e, -clavier_y/2 + clavier_vis_dy + clavier_dy , - e/2])
            vis();
        translate([boitier_x /2  - 2*e, +clavier_y/2 - clavier_vis_dy + clavier_dy , - e/2])
            vis();
        translate([-boitier_x /2  + 2*e, -clavier_y/2 + clavier_vis_dy + clavier_dy , - e/2])
                vis();
        translate([-boitier_x /2  + 2*e, +clavier_y/2 - clavier_vis_dy + clavier_dy , - e/2])
                vis();        
    }
}
module tout(eclate)
{
    translate([0,clavier_dy,eclate*35 + clavier_dz ])
        clavier();
    union()
    {
        boitier();
        composants();
    }
    translate([0,0,-30*eclate - boitier_z/2 + e/2])
        rotate([0,180,0])
            couvercle() ;
}
*potarl() ;
*potarrg() ;
*vis();
*s2() ;
*teensy() ;
*boulon(10,2);
*bouton();
*jack();
*led();
*led_plie() ;
*capteur();
*captage(0);
*captages();
*support_vis();
*difference()
{
    translate([0,clavier_dy,clavier_dz ])
        clavier();
    composants();
}
difference()
{
    translate([0,0,- boitier_z / 2 ])
        couvercle() ;
    composants();
}
*difference()
{
    boitier();
    composants();
}
*difference()
{
    tout(0);
    *composants() ;
    *translate([0,170,0])
        cube([400,400,400],center=true);
}