$fn = 10 ;
pouce=2.54;

largeur=26*pouce;
longueur_electronique = 12*pouce;
longueur_doigt = 12*pouce;
longueur_led = 4*pouce;
e=pouce ;
hauteur_doigt=4.5*pouce;
hauteur_electronique = 5*pouce ;
hauteur=hauteur_doigt + hauteur_electronique  ;

hled= 6.4; // hauteur led
lled = 5 ; // largeur led
eled = 3 ; // epaisseur led
dhled = 3.2 ; // distance top=>center of led
dled = 3.2 ; // dimension active led
pled = 20-6.4 -5; // dimension pattes led
iled = 9*pouce ; // intervale entre leds
dzporteled = hauteur/2 - e -e - 0.5 ; 


epcb = 1.5 ;
dyteensy = 4.5*pouce ;
dzusbteensy = 1.8 ;
dzteensy = - 0.5 ;

dyjack = -4.5*pouce ;
hjack = 3 ;
dzjack = hjack + epcb / 2;

dzpcb = 8 ;

composant = 0 ; // pour voir les composants
audio=1 ; // pour inclure le s2 et le jack

xpcb = longueur_doigt+longueur_electronique+longueur_led - 5*pouce ;
ypcb = largeur - 6*pouce ; 

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
    union()
    {
        // connecteur
        translate([-27,0,4])
            cube([5,33,8],true);
        // circuit
        translate([0,0,8+1])
            cube([67,40,2],true);
        // jack
        translate([-20,20-7,8+1+2.5])
            jack();
        // usb
        translate([20,20-5,8+1+2])
            cube([8,10,4],true);
        // cpu
        translate([0,-10,8+1+1.5])
            cube([13,13,3],true);        
   }
}
module s2pin()
{
    union()
    {
        for(i=[-7,-6,-4,3,4])
        {
            translate([-3*pouce,(i+0.5)* pouce ,2.7]) cube([0.7,0.7,4.5],true) ;
        }
        for(i=[-2,2])
        {
            translate([-4*pouce,(i+0.5)* pouce ,2.7]) cube([0.7,0.7,4.5],true) ;
        }
        // circuit
        difference()
        {
            translate([0,-0.5,4])
                cube([25,34,1.2],true);
            translate([4*pouce,5*pouce,4])
                cube([3,3,2],true);
        }
    }
}
module s2connecteur()
{
    union()
    {
        // connecteur
        translate([-9.5,0,4.5])
            cube([5,13*pouce,9],true);
        // circuit
        difference()
        {
            translate([0,0,4])
                cube([25,34,1.2],true);
            translate([4*pouce,5*pouce,4])
                cube([3,3,2],true);
        }
   }
}
module teensy2()
{
     union()
    {
        for(i=[-4,-3,-2,-1,0,1,2,3,5])
        {
            translate([-3*pouce,(i+0.5)* pouce ,2.5]) cube([0.7,0.7,5],center=true) ;
            translate([3*pouce,(i+0.5)* pouce ,2.5]) cube([0.7,0.7,5],center=true) ;
        }
        // circuit
        translate([0,0,4])
            cube([7.5*pouce,12*pouce,1.2],center=true);
        // usb
        translate([0,10.5,dzusbteensy])
            difference()
            {
                cube([7,9,4],true);
                cube([5,10,2],true);
            }
        // bouton
        translate([0,-4.5*pouce,2])
            cube([3,3,3],true);
   }
}
module pcb()
{
    difference()
    {
        cube([xpcb ,ypcb  ,epcb],true);
        // trou usb teensy
        translate([-9.5*pouce,dyteensy,]) rotate([0,0,90])cube([11,13,5],center=true);
        // trou bouton teensy
        translate([-1*pouce,dyteensy,]) rotate([0,0,90])cube([5,5,5],center=true);
        // grile pouces
        for(i = [-12,-11,-10,-7,0,2,9,10,11])
        {
            for ( j = [-9,-8,-7,-6,-2,0,1,6,7,8,9,10] )
            {
                translate([i*pouce,(j-0.5)*pouce,]) cube([0.5,0.5,5],center=true);
            }
        }
    }
    translate([-5.5*pouce,dyteensy,dzteensy]) rotate([0,180,90]) teensy2() ;
    if ( audio == 1)
    { 
        translate([2*pouce,-4.5*pouce,pouce/2]) rotate([0,180,-90]) s2pin() ;
        translate([-11.5*pouce,dyjack,epcb/2]) rotate([0,0,90]) jack();
    }
}
module contour(x,y,z)
{
    minkowski() {
        cube([x-e,y-e,z-e],true);
        // coins es
        sphere(e);
    }
}
module dedans(x,y,z)
{
    minkowski() {
        cube([x-e-2*e,y-e-2*e,z-e-2*e],true);
        // coins es
        sphere(e);
    }
}
module demichanfrein(longueur,largeur,epaisseur)
{
    del=2;
    rotate([-90,0,0])
    translate([0,del/2,0])
    difference()
    {
        hull()
        {
            cylinder(h=longueur,d=epaisseur+del,center=true);
            translate([largeur,0,0])
                cylinder(h=longueur,d=del,center=true);
        }
        translate([0,25-del/2,0])
            cube([50,50,50],center=true) ;
    }
}
module chanfrein(longueur,largeur,epaisseur)
{
    rotate([0,0,90])
    union()
    {
        translate([-largeur/2,0,0])
            demichanfrein(longueur,largeur/3,epaisseur);
        translate([largeur/2,0,0])
            rotate([0,0,180])
                demichanfrein(longueur,largeur/3,epaisseur);
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
                cube([eled + e + l + eled ,lled + e ,hled + e ],center=true);
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
module boite ()
{
    union()
    {
        difference()
        {
            // boitier plein
            union()
            {
                contour(longueur_electronique,largeur,hauteur);
                translate([longueur_doigt/2 + longueur_electronique/2 -e  , 0,-hauteur_doigt/2])
                    contour(longueur_doigt,largeur,hauteur_electronique);
                translate([longueur_led/2 + longueur_doigt + longueur_electronique/2 - 2* e ,0,0])
                    contour(longueur_led,largeur,hauteur);
            }
            // evidement boitier
            union()
            {
                dedans(longueur_electronique,largeur,hauteur);
                translate([longueur_doigt/2 + longueur_electronique/2 -e , 0,-hauteur_doigt/2])
                    dedans(longueur_doigt+4*e,largeur,hauteur_electronique);
                translate([longueur_led/2 + longueur_doigt + longueur_electronique/2 - 2 * e ,0,0])
                    dedans(longueur_led,largeur,hauteur);
            }
            // trous pour les chemins optiques
            union()
            {
                
                for(nrled = [-1:1] )
                {
                    translate([longueur_doigt/2 + longueur_electronique/2 -pouce ,nrled * iled,dzporteled ])
                       cube([longueur_doigt+4*pouce,dled,dled],center=true);
                }
            }
            // couvercle_fond
            *translate([longueur_electronique/2+e,0,-hauteur/2])
                cube([longueur_doigt+longueur_electronique+longueur_led - 4*e,largeur - 2*e,e*2],true);
            // trou fiche usb
            translate([-longueur_electronique+6*pouce,dyteensy,-dzusbteensy + dzteensy -hauteur/2 + dzpcb ])
                cube([2*pouce,13,7],center=true);
            if ( audio == 1)
            {
                // trou fiche jack
                translate([-longueur_electronique+5*pouce,dyjack,dzjack -hauteur/2 + dzpcb]) 
                    rotate([0,90,0]) 
                        cylinder(h=2*pouce,d=8);
            }
        }    
    }
    // chemins optiques
    union()
    {
        for(nrled = [-1:1] )
        {
            // recepteur
            translate([-(longueur_electronique -3*e )/2 +  e,nrled * iled,dzporteled  ])
                   porteled(longueur_electronique - 3*e );
            // emetteur
            translate([longueur_doigt + longueur_electronique/2 -pouce/2 ,nrled * iled,dzporteled ])
                    rotate([0,0,180])
                        porteled(pouce);
            // guide doigt
            *translate([longueur_doigt / 2 + longueur_electronique/2 - 2* e + e  ,nrled * iled,hauteur/2 - hauteur_doigt+e/4])
                chanfrein(longueur_doigt-2*e,iled *0.6,4);
        }
    }
    // composants
    if (composant == 1)
    {
        // pcb
        translate([longueur_electronique/2 + 0.5*pouce,0,-hauteur/2 + dzpcb ] ) 
            pcb() ;
     }

}
*pcb();
*s2pin();
*jack();
*teensy2();
*porteled(5);
*chanfrein(10,10,3);
difference()
{
    boite() ;
    translate([0,-125,0])
        cube([200,200,200], center = true);
}
echo("largeur=",largeur," longueur=",longueur_electronique+longueur_doigt+longueur_led, " hauteur=",hauteur);
echo ("xpcb=",xpcb, " ypcb=",ypcb);