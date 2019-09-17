composant = 1 ; // pour voir les composants
audio =1 ; // pour inclure le S2 et son jack
x2 = 1; // pour inclure un x2 à la place du s2

// precision dessin (10=brouillon, 50=finition)
$fn = 10 ;

pouce=2.54;

largeurx2 = 40 ;
longueurx2 = 67 ;
largeurs2=34;
longueurs2 = 25 ;

largeur=30*pouce;
x_electronique = 20*pouce; 
x_doigt = 12*pouce;
x_led = 4*pouce;
e=pouce ;
h_doigt=4.5*pouce;
h_electronique = (audio==1)?24:15;
hauteur = h_doigt + h_electronique;

hled= 6.4; // hauteur led
lled = 5 ; // largeur led
eled = 3 ; // epaisseur led
dhled = 3.2 ; // distance top=>center of led
dled = 3.2 ; // dimension active led
pled = 20-6.4 -6.5 ; // dimension pattes led
iled = 9*pouce ; // intervale entre leds
dzporteled = hauteur/2 - e -e - 0.5 ; 
hporteled = hled + e ;

epcb = 1.5 ;
dyteensy = -iled/2 ;
dzusbteensy = 4.3 ;
dzteensy = 0 ;

dyjack = +3.5*pouce ;
hjack = 3 ;
dzjack = hjack + epcb / 2;

xpcbled = 2 * pouce;
dzpcbled = hauteur/2 - h_doigt - 2*epcb    ;
ypcbled = 2*iled+2* pouce ; 
xpcb = 18 * pouce;
ypcb = largeur - 2* pouce ; 
dzpcb = dzporteled - hporteled/2 -epcb/2   ;

dys2 = 12*pouce ;
dzs2=-12 ;

lplot = 2.5*pouce ;
lplotcouvercle = 3.5*pouce;
dxplot1= -x_electronique/2+ lplotcouvercle/2  + e/2 ;
dxplot2 = x_electronique/2+x_doigt+x_led-2*e-lplotcouvercle/2 -e/2 ;
dyplot1 = -largeur/2  + lplotcouvercle/2 + e/2  ;
dyplot2 = largeur/2 - lplotcouvercle/2 - e/ 2;

htetevis=1.65;
dtetevis=5.6;
dvis=3;
dtrouvis=2.2;
dzvis = e-htetevis - 0.25 ;

lpedale = x_electronique + x_doigt/2 ;
epedale = 1 ;
lboutonpedale = 5 ;
lcourse =6  ;

lcny70=7;
hcny70=6;
dzcny70= -3 ;
lportecny70=lcny70+e;

module led()
{
    union()
    {
        difference()
        {
            cube([eled,lled,hled],center=true);
            translate([eled/2,0,0])
                cube([1,dled,dled],center=true);
        }
        translate([0,pouce /2,-hled/2-pled/2])
            cube([0.4,0.45,pled],center = true );
        translate([0,- pouce /2,,-hled/2-pled/2])
            cube([0.4,0.45,pled],center = true) ;
    }
}
module cny70()
{
    union()
    {
        translate([0,0,hcny70/2])
            cube([lcny70,lcny70,hcny70],center=true);
        translate([pouce/2,2.8/2,-2])
            cube([0.4,0.45,4],center = true );
        translate([-pouce/2,2.8/2,-2])
            cube([0.4,0.45,4],center = true );
        translate([pouce/2,-2.8/2,-2])
            cube([0.4,0.45,4],center = true );
        translate([-pouce/2,-2.8/2,-2])
            cube([0.4,0.45,4],center = true );
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
module x2()
{
    translate([longueurx2/2-4.5,0,1.2/2])
    union()
    {
        // connecteur
        translate([-28,0,4.5])
            cube([5,13*pouce,9],true);
        // circuit 
        difference()
        {
            translate([0,0,0])
                cube([longueurx2,largeurx2,1.5],true);
            translate([29,-14,0])
                cylinder(h=3,d=4,center=true);
        }
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
}
module s2()
{
    translate([longueurs2/2-2,0,1.2/2])
    union()
    {
        // connecteur
        translate([-9.5,0,4.5])
            cube([5,13*pouce,9],true);
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
        for(i=[-4,-3,-2,-1,0,1,2,3,5])
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
module pcbled()
{
    difference()
    {
        translate([0,0,0]) cube([xpcbled ,ypcbled  ,epcb],true);
        // grile pouces
        for ( j = [-6,-5,-1,8,9,12] )
        {
            translate([0,(j-0.5)*pouce,0]) cube([0.5,0.5,5],center=true);
        }
    }
}
module pcb()
{
    difference()
    {
        translate([0.5*pouce,0,0]) cube([xpcb ,ypcb  ,epcb],true);
        // grile pouces
        for(i = [-7,-6,-2,0,1,4,5,7])
        {
            for ( j = [-11,-6,-5,-1,8,9,12] )
            {
                translate([i*pouce,(j-0.5)*pouce,0]) cube([0.5,0.5,5],center=true);
            }
        }
    }
    translate([-2.5*pouce,dyteensy,dzteensy]) rotate([180,0,-90]) teensy2() ;
    *translate([-6.5*pouce,-11*pouce,dzcny70]) rotate([180,0,0]) cny70();
    if ( audio == 1 )
    {
        // jack
        translate([-8.5*pouce,dyjack,-epcb / 2]) rotate([180,0,-90]) jack() ;
        // pins
        translate([3*pouce,dys2 ,-4]) cube([0.7,0.7,8],center=true) ;
        translate([-2*pouce,dys2 ,-4]) cube([0.7,0.7,8],center=true) ;
        translate([3*pouce,dys2-pouce ,-4]) cube([0.7,0.7,8],center=true) ;
        translate([-2*pouce,dys2-pouce ,-4]) cube([0.7,0.7,8],center=true) ;
        // expander
        translate([3*pouce,dys2,dzs2  ] ) 
                rotate([0,0,-90])
                    if ( x2 == 1 )
                        x2() ;
                    else
                        s2() ;
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
    minkowski() {
        cube([x-e-2*e,y-e-2*e,z-e-2*e],true);
        // coins arrondis
        sphere(r=e);
    }
}
module porteled(l)
{
    union()
    {
        difference()
        {
            // tunnel
            translate([(l+eled)/2,0,0])
                cube([eled + e + l + eled ,lled + e ,hporteled ],center=true);
            // moins logement led
            scale([1.2,1.15,1.05]) led() ;
            // moins degagement led
            translate([(l+eled)/2,0,0.1])
                cube([eled  + l + eled ,lled -0.2 ,hled - 0.2 ],center=true);
            // ouverture tunnel extrémité
            translate([10 + (l+eled/2),0,0])
                cube([20 ,20,20],center=true);
            // moins degageent pour inserer la led
            translate([-e/2,0,-10-hled/2])
                cube([eled + 1.5* e ,20,20],center=true);
        }
        if (composant == 1)
            led();
    }
}
module portecny70()
{
    hportecny70 = hcny70+e/2+lcourse+3*epedale ;
    difference()
    {
        translate([0,0,-hportecny70/2])
            cube([lportecny70,lportecny70,hportecny70], center = true);
        translate([0,0,-hportecny70/2 + e/2]) scale(1.05,1.05,1.1)
            cube([lcny70,lcny70,hportecny70], center = true);
        translate([-lcny70/2,0,(-lcourse-e - hcny70)/2 + e]) scale(1.05,1.05,1.1)
            cube([lcny70,lcny70,hcny70+lcourse], center = true);
        translate([0,0,0]) scale(0.8,0.8,1)
            cube([lcny70,lcny70,100], center = true);
    }
    if ( composant == 1)
    {
        translate([0,0,-hportecny70+e/2]) cny70();
    }
}
module plot(h,l,vis)
{
    translate([0,0,h/2])
        difference()
        {
            cube([l,l,h],center=true);
            cylinder(h=h,d=dtrouvis,center=true);
        }
    if (vis)
    {
        translate([0,0,6/2-epcb])
            cylinder(h=6,d=1.5,center=true);
        translate([0,0,2/2-2.1*epcb])
            cylinder(h=2,d=4.5,center=true);
    }
}
module fond(zscale)
{
    lfond = x_doigt+x_electronique+x_led - 3*e ;
    translate([lfond/2-x_electronique/2 +e/2,0,0])
         cube([lfond,largeur - e,zscale*e],true);
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
module pedale()
{
    union()
    {
        // lame ressort
        difference()
        {
        cube([lpedale,epedale,lboutonpedale],center=true);
        }
        // bouton utilisateur
        translate([lpedale/2-lboutonpedale/2,-(e+lcourse+2*epedale)/2,0])
            bouton(e+lcourse+2*epedale,lboutonpedale);
    }
}
module plot_couvercle(dx,xy)
{
    translate([dx,xy,-hauteur/2 + e/2 ])
        plot(1.5*lplot,lplotcouvercle,false);
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
                contour(x_electronique,largeur,hauteur);
                translate([x_doigt/2 + x_electronique/2 -e  , 0,-h_doigt/2])
                    contour(x_doigt,largeur,h_electronique);
                translate([x_led/2 + x_doigt + x_electronique/2 - 2* e ,0,0])
                    contour(x_led,largeur,hauteur);
            }
            // evidement boitier
            union()
            {
                dedans(x_electronique,largeur,hauteur);
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
                    translate([x_doigt/2 + x_electronique/2 -pouce ,nrled * iled,dzporteled ])
                       cube([x_doigt+4*pouce,dled,dled],center=true);
                }
            }
            // couvercle_fond
            translate([0,0,-hauteur/2]) fond(4) ;
            // trou fiche usb
            translate([-x_electronique/2,dyteensy,-dzusbteensy - dzteensy + dzpcb ])
                cube([2*pouce,13,7],center=true);
            
            if ( audio == 1 )
            {
                // trou fiche jack
                translate([-x_electronique/2,dyjack,-dzjack + dzpcb]) 
                    rotate([0,90,0]) 
                        cylinder(h=2*pouce,d=8,center = true);
            }
            // trou pedale
            translate([lpedale - x_electronique/2 - lboutonpedale/2,-largeur/2,lportecny70/2 - hauteur/2 + e/2] )
                rotate([90,0,0])
                    cylinder(h=10,d=lboutonpedale * 1.5 ,center = true);
       }    
    }
    // chemins optiques
    union()
    {
        for(nrled = [-1:1] )
        {
            // recepteur
            translate([-(x_electronique -3*e )/2 +  e,nrled * iled,dzporteled  ])
                   porteled(x_electronique - 3*e );
            // emetteur
            translate([x_doigt + x_electronique/2 -pouce/2 ,nrled * iled,dzporteled ])
                    rotate([0,0,180])
                        porteled(pouce);
        }
    }
    // couvercle avec trous de vis
    *translate([0,0,-hauteur/2 - 4  ])
    union()
    {
        difference()
        {
            fond(1);
            trou_couvercle(dxplot1,dyplot1);
            trou_couvercle(dxplot2,dyplot1);
            trou_couvercle(dxplot1,dyplot2);
            trou_couvercle(dxplot2,dyplot2);
        }
        //attaches pour l'impression
        *translate([dxplot1-2,dyplot1-2,3])cube([1,1,6],center=true);
        *translate([dxplot2+2,dyplot2+2,3])cube([1,1,6],center=true);
    }
    // fixation
    union()
    {
        // pcb
        for(j=[-3.5*pouce,5.5*pouce])
        {
            for(i=[-iled - lled/2 - e/2 - pouce ,iled - lled/2 - e/2 - pouce])
            {
                translate([j,i,dzpcb])
                    plot(hled + 1.5*e,lplot,true);
            }
        }
        // couvercle
        plot_couvercle(dxplot1,dyplot1);
        plot_couvercle(dxplot2,dyplot1);
        plot_couvercle(dxplot1,dyplot2);
        plot_couvercle(dxplot2,dyplot2);
        // x2
        translate([0,-largeur/2,0])
        {
            difference()
            {
                cube([10,40,5]);
            }
        }
    }
    // pedale
    translate([lpedale/2 - x_electronique/2  ,-largeur/2 + epedale + e ,lportecny70/2 - hauteur/2  + e/2 ])
        union()
        {
            pedale() ;
            translate([lpedale/2 - lcny70/3  ,-1*e ,0])
                rotate([90,0,0]) portecny70();
        }
    // composants
    if (composant == 1)
    {
        // pcb
        translate([-x_electronique/2 + xpcb/2 + pouce /2 ,0,dzpcb  ] ) 
            pcb() ;
        // pcbled
        *translate([+x_electronique/2 + x_doigt + x_led/2 -2*e -pouce/2 ,0 , dzpcbled  ] ) 
            pcbled() ;
     }

}
*cny70();
*portecny70();
*pedale();
*plot(6,lplot,true);
*pcb();
*pcbled();
*teensy2();
*porteled(5);
*chanfrein(10,10,3);
*s2(); 
*x2();
difference()
{
    boite() ;
    translate([-120,0,0])
        cube([200,200,200], center = true);
}
echo("largeur=",largeur," longueur=",x_electronique+x_doigt+x_led, " hauteur=",hauteur);
echo ("xpcb=",xpcb, " ypcb=",ypcb);