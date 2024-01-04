e = 4 ; // epaisseur du socle
led_d = 5 ; // diametre led
led_h = 8 ; // hauteur led
photo_d = led_d ; // diametre phototransistor
photo_d2 = photo_d + 1  ; // diametre encombrement phototransistor
photo_h = led_h ; // hauteur phototransistor ;
corde_r = 800 ; // rayon d'une corde
corde_jeu2 = 2 * corde_r / 3 ; // aire du jeu
corde_jeu1 = corde_jeu2 - 80 ; // aire du jeu
corde_a = asin(25/corde_jeu2) ; // angle entre les cordes
corde_tunnel = 60 ; // longueur tunnel noir
corde_led = corde_jeu1 - corde_tunnel ; // position debut led emitrice
corde_photo = corde_jeu2 + corde_tunnel  ; // position fin phototransistors
corde_lumiere = corde_photo - corde_led - led_h - photo_h ; // longueur tunne de lumiere
corde_lumiere_dx = corde_lumiere/2 + corde_led + led_h; // position tunnel de lumiere
photo_nb = 3 ;
photo_z = photo_d2 * photo_nb ; // hauteur tunnel phottransistors
corde_h = photo_z + e ; // hauteur d'une corde
fil_d = 0.5 ; // diametre du fil tactile
fil_nb = 5 ;
fil_dy = ((corde_jeu2 - corde_jeu1 ) / fil_nb ) / 2 ; // decalage des deux fils

corde_nb = 4;

module corde()
{

    difference() 
    {

       // cylindre complet
       translate([0,0,-e/2])
             cylinder(r=corde_r,h=corde_h + e, center = true);
       // couper pour faire un camenbert
       rotate([0,0,corde_a/2])
             translate([0,corde_r/2,0])
                 cube([2*corde_r,corde_r,corde_h + 2*e],center = true);
       rotate([0,0,-corde_a/2])
             translate([0,-corde_r/2,0])
                 cube([2*corde_r,corde_r,corde_h + 2*e],center = true);
    
        
       // couper les pointes pour avoir la section qui contient les leds et photo
       translate([-corde_r + corde_led,0 ,0])
           cube([2*corde_r,2*corde_r,corde_h+2*e],center = true);
       translate([corde_r + corde_photo,0])
           cube([2*corde_r,2*corde_r,corde_h+2*e],center = true);
   
    
       // couper pour avoir la section de jeu
       translate([(corde_jeu2 - corde_jeu1)/2 + corde_jeu1  ,0,0])
           cube([corde_jeu2 - corde_jeu1,2*corde_r,corde_h],center = true);

       // percer pour le tunnel de lumiere
       translate([corde_lumiere_dx,0,0])
           cube([corde_lumiere ,photo_d,photo_z],center = true);
           
       // percer led
       translate([corde_led + led_h / 2, 0,0])
           rotate([0,90,0])
                cylinder(d=led_d,h=led_h,center = true);

       // percer phototransistor
       for(i = [1:photo_nb])
       {
           si = i - (photo_nb + 1)/ 2 ;
           echo("si=",si);
           dz =  photo_d2 * si ;
           echo("dz=",dz);
           da = atan(dz / corde_lumiere ) * si * ((si < 0)?1:-1) ;
           echo("da=",da);
           translate([corde_photo - photo_h / 2 , 0 , dz ])
               rotate([0,90 + da ,0])
                   cylinder(d=photo_d,h=photo_h + 5,center = true);
       }
    }

       // fils tactile
    for(f=[1:fil_nb])
    {
       translate([corde_jeu1 + f * (corde_jeu2 - corde_jeu1 )/ (fil_nb + 1) ,0 ,-e])
           cylinder(d=fil_d , h=photo_z + e  ,center = true);
    }
}
module cordes() {
    for(a = [0 : corde_a : (corde_a * ( corde_nb - 1) ) ]) 
    {
        echo ( a ) ;
            rotate([0,0,a])
                corde();
    }
}

difference()
{
    corde();
    *translate([0,200,0])
        cube([4000,400,400],center = true);
}