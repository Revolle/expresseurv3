// pour voir les composants
voir_composant = 0 ; 
// pour voir les led
voir_led = 1 ; 
// pour vior la pedale
voir_pedale = 1 ; 
// pour lier les objets à l'impression (plot s2)
ligature=1; 
// pour lier le couvercle
ligature_couvercle = 0; 
// pour inclure le S2 et son jack
audio = 1 ; 
// pour inclure un x2 à la place du s2
x2 = 1; 
// pour inclure la pedale
pedale = 1 ;  
// pour inclure une capteur externe
capteur = 1 ;
// pour inclure le couvercle
couvercle = 0 ; 
// decalage couvercle pour l'impression
dzcouvercle = -10 ; 


// precision dessin (6=brouillon, 20=imprimable)
$fn = 15 ;

pouce=1*2.54;

largeurx2 = 40 ;
longueurx2 = 67 ;
largeurs2=34;
longueurs2 = 25 ;
epcb = 1.5 ;

// intervale entre leds 
iled_pouce = 9 ; 
iled = iled_pouce *pouce ;

ex2=1.2;
dxs2 = -2*pouce ;
dys2 = 10.5*pouce ;
dzs2=-12 ;
dyjack = iled / 2 ;
dxjack = -15.5*pouce;
hjack = 3 ;
dzjack = hjack + epcb / 2;
d_trou_jack = 10 ;
dxcapteur = -1.5*pouce ;


largeur=30*pouce;
x_electronique = 20*pouce; 
x_doigt = 12*pouce;
x_led = 4*pouce;
e=pouce ;
h_doigt=10;
h_electronique = (audio==1)?22:15;
hauteur = h_doigt + h_electronique;
d_empreinte = 9*pouce ;

// TT Electronics OP555 OP145
hled= 5.84; // hauteur led 
lled = 4.45 ; // largeur led
eled = 2.54 ; // epaisseur led
dhled = 1.27 ; // distance top=>center of led
dled1 = 1.8 ; // surface active led
pled = 20-6.4 -6 ; // dimension pattes led

hdouble_led = hled + dhled + dled1 / 2 ;
edouble_led = 2*eled ;

dled = 2*(dhled+dled1/2)+(dhled-dled1/2); // dimension trou lumiere


hporteled = dled + e ; 
dzporteled = hauteur/2 ; 
dzledporteled = - hporteled/2 ;
dzcentreoptique = dzporteled +dzledporteled  ;
lporteled = lled + e/2 ; // largeur porte led

dxteensy = -9.5*pouce ;
dyteensy = -iled/2 ;
dzusbteensy = 4.3 ;
dzteensy = 0 ;
l_trou_usb = 12 ;
h_trou_usb = 9 ;

xpcb = x_electronique + x_doigt + x_led - 4*pouce ; 
ypcb = largeur - 2* pouce ; 
dzpcb = hauteur / 2 - h_doigt - epcb/2 ; 

dxpcb = -x_electronique/2 + xpcb/2 + pouce /2 ;
dxpcbled = +x_electronique/2 + x_doigt + x_led/2 -e -pouce ;

lplot = 2.5*pouce ;
dyplotcouvercle = 3.5*pouce;
dxplot1= -x_electronique/2+ dyplotcouvercle/2  + e/2 ;
dxplot2 = x_electronique/2+x_doigt+x_led-2*e-dyplotcouvercle/2 -e/2 ;
dyplot1 = -largeur/2  + dyplotcouvercle/2 + e/2  ;
dyplot2 = largeur/2 - dyplotcouvercle/2 - e/ 2;

htetevis=1.65;
dtetevis=5.6;
dvis=2.5;
dtrouvis=1.9;
dzvis = e-htetevis - 0.25 ;

// TT Eletronics 404 series
h_tt404 = 8.51 ; 
l_tt404 = 12.7 ;
L_tt404 = 26.92 ;
d_tt404 = 4.85 ;
da_tt404 = -l_tt404/2 + 7.88;
dt_tt404 = 3.56 ;
laxe_tt404 = 20.57 ;
dtrou_tt404 = 1.93 ;
daxe_tt404 = 24.13 ;
hpicot_tt404 = 3 ;
dz_tt404 = -8.51/2+3.56 ;
dxtrou_tt404 = dt_tt404-l_tt404/2 ;
dztt404 = h_tt404/2 + hpicot_tt404 - dz_tt404 ;
dxtt404 = 9.5 * pouce ;
dz_trou_pedale = - dztt404 + dzpcb - epcb/2 ;

dcuberaccord=e/4;

dcubecoupe=200;
// vue en coupe 
//coupe= [500,0,0]; // pas de coupe
// longueur
//coupe= [-120,00,0]; // phototransistor
//coupe= [-116,00,0]; // pcb
//coupe= [-100,00,0]; // tunnel
//coupe= [-67,00,0];// pedale
//coupe= [-45,00,0]; // led
//coupe= [83,00,0]; // 1/2 phototransistor
//coupe= [85,00,0]; // phototransistor
//coupe= [123,00,0]; // pcb
//coupe= [153,00,0]; // sans led
//coupe= [155,00,0]; // avec led
//largeur
//coupe=[0,77,0]; // led coté
//coupe=[0,-100,0]; // milieu
//coupe=[0,100,0]; // milieu
//coupe=[0,-135,0]; // presque tout
//hauteur
//coupe=[0,0,(dcubecoupe/2-dz_trou_pedale)];
coupe=[0,0,-(dcubecoupe/2-dz_trou_pedale)];

module tt404()
{
    translate([da_tt404,0,h_tt404/2 + hpicot_tt404])
    rotate([0,180,0])
    union()
    {
        if ( voir_composant == 1)
        {
            difference()
            {
                // corps
                translate([0,-L_tt404/2,0])
                    cube([l_tt404,L_tt404,h_tt404],center=true);
               // trou vis
                translate([dxtrou_tt404,-L_tt404/2+daxe_tt404/2,0])
                    cylinder(h=10,d=dtrou_tt404,center=true);
                translate([dtrou_tt404,-L_tt404/2+daxe_tt404/2+dtrou_tt404/2,0])
                    cube([dxtrou_tt404,dtrou_tt404+0.2,10],center=true);
               // trou vis
                translate([dxtrou_tt404,-L_tt404/2-daxe_tt404/2,0])
                    cylinder(h=10,d=dtrou_tt404,center=true);
                translate([dtrou_tt404,-L_tt404/2-daxe_tt404/2-dtrou_tt404/2,0])
                    cube([dxtrou_tt404,dtrou_tt404+0.5,10],center=true);
            }
            // bouton utilisateur
            translate([da_tt404,laxe_tt404/2,dz_tt404])
                rotate([90,0,0])
                    cylinder(h=laxe_tt404,d=d_tt404,center=true);
            // picots
            translate([3.5,-6.8,8.51/2+1.5])
                cube([1,1,hpicot_tt404], center=true);
            translate([3.5,-15,8.51/2+1.5])
                cube([1,1,hpicot_tt404], center=true);
            translate([3.5,-20,8.51/2+1.5])
                cube([1,1,hpicot_tt404], center=true);
        }
        translate([dxtrou_tt404,-L_tt404/2+daxe_tt404/2,(hpicot_tt404 + epcb + e/2)/2])
                    cylinder(h=h_tt404 + hpicot_tt404 + epcb + e/2 ,
                            d=dtrou_tt404-0.1,center=true);
        translate([dxtrou_tt404,-L_tt404/2+daxe_tt404/2,(hpicot_tt404 + epcb + e/2)/2 + h_tt404 / 2])
                    cylinder(h=hpicot_tt404 + epcb + e/2 ,
                            d=dtrou_tt404+2,center=true);
        translate([dxtrou_tt404,-L_tt404/2-daxe_tt404/2,(hpicot_tt404 + epcb + e/2)/2])
                    cylinder(h=h_tt404 + hpicot_tt404 + epcb + e/2 ,
                            d=dtrou_tt404-0.1,center=true);
         translate([dxtrou_tt404,-L_tt404/2-daxe_tt404/2,(hpicot_tt404 + epcb + e/2)/2 + h_tt404 / 2])
                    cylinder(h=hpicot_tt404 + epcb + e/2 ,
                            d=dtrou_tt404+2,center=true);
    }
}
module led(sc)
{
    translate([0,0,-hled/2 + dhled])
    scale([sc,sc,sc])
    union()
    {
        cube([eled,lled,hled],center=true);
        translate([eled/2,0,hled/2 - dhled])
                cube([0.5,dled1,dled1],center=true);
        translate([0,pouce /2,-hled/2-pled/2])
            cube([0.4,0.45,pled],center = true );
        translate([0,- pouce /2,,-hled/2-pled/2])
            cube([0.4,0.45,pled],center = true) ;
    }
}
module double_led(sc)
{
    translate([0,0,dled1/2 + (dhled-dled1/2)/2])
    union()
    {
        led(sc);
        translate([eled,0,-dhled - dled1/ 2])
            led(sc);
    }
}
module jack()
{
    union()
    {
        translate([0,-8,3])
        difference()
        {
            cube([6,16,2*hjack],true);
            translate([0,2,0]) rotate([90,0,0]) cylinder(h=15,d=4,center=true);
        }
        translate([-1.5*pouce,-5.5*pouce ,0.3]) cube([2,1,0.6],true) ;
        translate([-1.5*pouce,-1.5*pouce ,0.3]) cube([2,1,0.6],true) ;
        translate([1.5*pouce,-2.0*pouce ,0.3]) cube([2,1,0.6],true) ;
    }
}
module connecteur13x2()
{
    union()
    {
       cube([5,13*pouce,9],true);
        for(i = [-6:6])
        {
            for ( j = [-0.5,0.5] )
            {
                translate([j*pouce,i*pouce,5]) cube([0.5,0.5,3],center=true);
            }
        }
    }
}
module x2()
{
    difference()
    {
        translate([longueurx2/2-4.5,0,ex2/2])
        union()
        {
            // connecteur
            translate([-28,0,4.5])
                connecteur13x2();
            // circuit 
            cube([longueurx2,largeurx2,1.5],true);
            // jack
            translate([-20,-20,-0.5])
                rotate([0,180,180])
                    jack();
            // usb
            translate([20,-20+5,-3.0])
                cube([8,10,4],true);
            // cpu
            translate([0,10,-2])
                cube([13,13,3],true); 
       }
        translate([23*pouce,-14,0])
            cylinder(h=3,d=4,center=true);
   }
}
module s2()
{
    translate([longueurs2/2-2,0,ex2/2])
    union()
    {
        // connecteur
        translate([-9.5,0,4.5])
                connecteur13x2();
        // circuit
        difference()
        {
            translate([0,0,0])
                cube([longueurs2,largeurs2,1.2],true);
            translate([9,-14,0])
                cylinder(h=3,d=4,center=true);
        }
        //composants
        translate([10.5,-9.7,1.2])
            cube([2,3,1.8],true);
        translate([5.5,-5.7,1.2])
            cube([2,3,1.8],true);
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
module pcb()
{
    if ( voir_composant == 1)
    {
        difference()
        {
            translate([0.5*pouce,0,0]) cube([xpcb ,ypcb  ,epcb],true);
            // grile pouces
            for(i = [-8,-6,-2,0,1,4,5,7])
            {
                for ( j = [-12,-6,-5,-1,8,9,12] )
                {
                    translate([i*pouce,(j-0.5)*pouce,0]) cube([0.5,0.5,5],center=true);
                }
            }
        }
        translate([dxteensy,dyteensy,dzteensy]) rotate([180,0,-90]) teensy2() ;
        if ( audio == 1 )
        {
            // jack
            translate([dxjack,dyjack,-epcb / 2]) rotate([180,0,-90]) jack() ;
            // pins
            translate([3*pouce,dys2 ,-4]) cube([0.7,0.7,8],center=true) ;
            translate([-2*pouce,dys2 ,-4]) cube([0.7,0.7,8],center=true) ;
            translate([3*pouce,dys2-pouce ,-4]) cube([0.7,0.7,8],center=true) ;
            translate([-2*pouce,dys2-pouce ,-4]) cube([0.7,0.7,8],center=true) ;
            // expander
            translate([dxs2,dys2,dzs2  ] ) 
                    rotate([0,0,-90])
                        if ( x2 == 1 )
                            x2() ;
                        else
                            s2() ;
        }
        if ( capteur == 1 )
        {
            // jack        
            translate([dxcapteur,-ypcb/2,-epcb / 2]) 
                rotate([180,0,0])
                    jack() ;
        }
    }
    if ( pedale == 1 )
    {
        translate([dxtt404,-ypcb/2 + pouce,-epcb / 2]) 
            rotate([180,0,0])
                tt404() ;
    }
}
module contour(x,y,z)
{
    minkowski() {
        cube([x-e,y-e,z-e],true);
        // coins arrondis
        sphere(r=e);
    }
}
module dedans(x,y,z)
{
    scale([(x-e)/x,(y-e)/y,(z-e)/z]) contour(x,y,z);
    /* 
    minkowski() {
        cube([x-e-2*e,y-e-2*e,z-e-2*e],true);
        // coins arrondis
        sphere(r=e);
    }
    */
}
module echancrure_empreinte(x,y,z,dy)
{
     translate([x/2+d_empreinte/5,dy,z/2+d_empreinte/6]) rotate([0,-35,0]) cylinder(d=d_empreinte,h=2*z,center=true);

}
module contour_electronique(x,y,z)
{
    minkowski()
    {
        difference()
        {
            cube([x-e,y-e,z-e],center=true);
            for(nrled = [-1:1] )
            {
                echancrure_empreinte(x,y,z,nrled*iled);
            }
        }
        sphere(r=e);
    }

 }
module dedans_electronique(x,y,z)
{
    scale([(x-e)/x,(y-e)/y,(z-e)/z])
        contour_electronique(x,y,z);    
}
module porteled(l, doubleled)
{
    translate([0,0,dzledporteled])
    union()
    {
        difference()
        {
            // tunnel
                translate([(l+eled)/2,0,0])
                    cube([eled + e + l + eled ,
                            lporteled,
                            hporteled  ],center=true);
            // trou lumiere
                translate([(l+eled)/2,0,0])
                    cube([eled  + l + eled ,
                            lporteled - e ,
                            hporteled - e ],center=true);
            // moins logement led
            if (doubleled == 1)
                led(1.05) ;
            else
                double_led(1.05) ;
            // ouverture tunnel extrémité
            translate([10 + (l+eled/2),0,0])
                cube([20 ,20,20],center=true);
            // moins degagement pour inserer la led
            *translate([-e/2,0,-10-hled/2])
                cube([eled*doubleled + 1.5* e ,20,20],center=true);
        }
        if ((voir_composant == 1) && (voir_led == 1))
        {
            if (doubleled == 1)
                led(1);
            else
                double_led(1) ;
        }
    }
}
module plot(h,vis)
{
    translate([0,0,h/2])
        difference()
        {
            cube([lplot,lplot,h],center=true);
            cylinder(h=h,d=dtrouvis,center=true);
        }
    if ( voir_composant)
    {
        if (vis)
        {
            translate([0,0,6/2-epcb])
                cylinder(h=6,d=1.5,center=true);
            translate([0,0,2/2-2.1*epcb])
                cylinder(h=2,d=4.5,center=true);
        }
    }
}
module fond(zplus,xyminus)
{
    lfond = x_doigt+x_electronique+x_led - 3*e + xyminus ;
    translate([lfond/2-x_electronique/2 +e/2 - xyminus/2 ,0,0])
         cube([lfond,largeur - e +xyminus ,zplus+e],true);
}
module bouton(lbouton,dbouton)
{
    rotate([90,0,0])
    union()
    {
        translate([0,0,-dbouton/8])
            cylinder(h=lbouton-dbouton/4,d=dbouton,center=true);
        translate([0,0,lbouton/2-dbouton/4])
            sphere(d=dbouton,center=true);
    }

}
module plot_couvercle(dx,dy)
{
    translate([dx,dy,-hauteur/2 + e/2 ])
    {
        union()
        {
            plot(1.5*lplot,false);
            if (dy > 0)
                translate([0,lplot/2+(largeur/2-abs(dy)- lplot/2)/2,1.5*lplot/2])
                    cube([lplot,largeur/2-abs(dy)- lplot/2,1.5*lplot],center=true);
            else
                translate([0,-(lplot/2+(largeur/2-abs(dy)- lplot/2)/2),1.5*lplot/2])
                    cube([lplot,largeur/2-abs(dy)- lplot/2,1.5*lplot],center=true);
        }
    }
}
module trou_couvercle(dx,dy)
{
    translate([dx,dy,0])
        cylinder(h=2*e,d=dvis,center=true);            
    translate([dx,dy,-dzvis/2 - e/2 +dzvis ])
        cylinder(h=dzvis,d=dtetevis,center=true);            
    translate([dx,dy,-htetevis/2 -e/2 +dzvis +htetevis ])
        cylinder(h=htetevis,d1=dtetevis,d2=dvis,center=true);            
}
module boite ()
{
    union()
    {
        difference()
        {
            // boitier plein
            union()
            {
                contour_electronique(x_electronique,largeur,hauteur);
                translate([x_doigt/2 + x_electronique/2 -e  , 0,-h_doigt/2])
                    contour(x_doigt,largeur,h_electronique);
                translate([x_led/2 + x_doigt + x_electronique/2 - 2* e ,0,0])
                    contour(x_led,largeur,hauteur);
            }
            // evidement boitier
            union()
            {
                dedans_electronique(x_electronique,largeur,hauteur);
                translate([x_doigt/2 + x_electronique/2 -e , 0,-h_doigt/2])
                    dedans(x_doigt+4*e,largeur,h_electronique);
                translate([x_led/2 + x_doigt + x_electronique/2 - 2 * e ,0,0])
                    dedans(x_led,largeur,hauteur);
            }
            // trous pour les chemins optiques
            union()
            {           
                for(nrled = [-1:1] )
                {
                    translate([x_doigt/2 + x_electronique/2 -pouce ,nrled * iled,dzcentreoptique  ])
                       cube([x_doigt+4*pouce,
                                lporteled - e ,
                                hporteled - e ],center=true);
                }
            }
            // trou fiche usb
            translate([-x_electronique/2,dyteensy,-dzusbteensy - dzteensy + dzpcb  - epcb/2])
                cube([2*pouce,l_trou_usb ,h_trou_usb ],center=true);
            
            if ( audio == 1 )
            {
                // trou fiche jack audio
                translate([-x_electronique/2,dyjack,-dzjack + dzpcb - epcb/2 ]) 
                    rotate([0,90,0]) 
                        cylinder(h=2*pouce,d=d_trou_jack,center = true);
            }
            if ( capteur == 1 )
            {
                // trou fiche jack capteur
                translate([dxpcb+ dxcapteur, -largeur/2,-dzjack + dzpcb - epcb/2 ]) 
                    rotate([0,90,90]) 
                        cylinder(h=2*pouce,d=d_trou_jack,center = true);
            }
            if ( pedale == 1)
            {
                // trou pedale
                translate([dxpcb+ dxtt404 ,-largeur/2, dz_trou_pedale ]) 
                {
                    rotate([90,0,0])
                        cylinder(h=10,d=d_tt404  + 1 ,center = true);
                    *translate([0,0,-10])
                        cube([d_tt404  + 1, 6*e,20],center=true);
                }
            }
       } 
    }
    // chemins optiques   
    *for(nrled = [-1:1] )
    {
        // recepteur
        difference()
        {
            translate([-(x_electronique -3*e )/2 +  e,nrled * iled,dzporteled  ])
               porteled(x_electronique - 3*e , 2 );
            translate([e,0,0])
                echancrure_empreinte(x_electronique,largeur,hauteur,nrled * iled);
        }
        // emetteur
        translate([x_doigt + x_electronique/2 -pouce/2 ,nrled * iled,dzporteled ])
                rotate([0,0,180])
                    porteled(pouce , 1);
    }
        
    // fixation
    union()
    {
        // pcb
        *translate([-5.5*pouce,-12.5*pouce,dzpcb ])
              plot(hauteur/2 - dzpcb,true);
        *translate([-5.5*pouce,12.5*pouce,dzpcb ])
              plot(hauteur/2 - dzpcb,true);
        translate([-3.5*pouce,-12.5*pouce,dzpcb ])
              plot(hauteur/2 - dzpcb,true);
        translate([-3.5*pouce,12.5*pouce,dzpcb ])
              plot(hauteur/2 - dzpcb,true);
        translate([x_electronique/2+x_doigt,-lplot ,dzpcb])
                plot(hauteur/2 - dzpcb,true);
        // couvercle
        plot_couvercle(dxplot1,dyplot1);
        plot_couvercle(dxplot2,dyplot1);
        plot_couvercle(dxplot1,dyplot2);
        plot_couvercle(dxplot2,dyplot2);
        if ( x2 == 1 )
        {
            //x2
            translate([-1*pouce,-12.5*pouce,dzpcb+dzs2])
            *union()
            {
                plot(-dzs2 -epcb ,true);
                if ( ligature == 1)
                {
                    translate([0,-3.5,1])
                    rotate([90,0,0]) cube([1,1,5],center=true);
                }
            }
        }
    }
     // composants
    translate([dxpcb ,0,dzpcb - epcb/2 ] ) 
            pcb() ;
 }
 module boite_decoupe_haut()
 {
     union()
     {
         translate([0,0,-(dcubecoupe/2-dz_trou_pedale)])   
            cube([dcubecoupe,dcubecoupe,dcubecoupe], center = true);
         translate([(x_doigt+x_led-2*e)/2,largeur/2+e/5,dz_trou_pedale])
            rotate([45,0,0])
                cube([x_electronique+(x_doigt+x_led-2*e)-2*e,
                    dcuberaccord,dcuberaccord],center=true);
         translate([-x_electronique/4+2*e,-largeur/2-e/5,dz_trou_pedale])
            rotate([45,0,0])
                cube([x_electronique/2+2*e,
                dcuberaccord,dcuberaccord],center=true);
         translate([x_electronique/2+(x_doigt+x_led-4*e)/2,
                    -largeur/2-e/5,dz_trou_pedale])
            rotate([45,0,0])
                cube([x_doigt+x_led-2*e,
                dcuberaccord,dcuberaccord],center=true);
     }
 }
 module boite_decoupe_bas()
 {
    cube([dcubecoupe,dcubecoupe,dcubecoupe], center = true);
 }
 module boite_haut()
 {
     difference()
     {
        boite();
        boite_decoupe_bas() ;
     }
 }
 module boite_bas()
 {
      difference()
     {
        boite();
        boite_decoupe_haut() ;
     }
}
*tt404();
*plot(6,lplot,true);
*pcb();
*teensy2();
*porteled(5,2);
*led(1);
*double_led();
*chanfrein(10,10,3);
*s2(); 
*x2();
*jack();
*difference()
{
    boite() ;
    translate(coupe)   
        cube([dcubecoupe,dcubecoupe,dcubecoupe], center = true);
}
boite_bas();
echo("largeur=",largeur," longueur=",x_electronique+x_doigt+x_led, " hauteur=",hauteur);
echo ("xpcb=",xpcb, " ypcb=",ypcb);
echo ("dxtrou tt404=",dxtrou_tt404+da_tt404);