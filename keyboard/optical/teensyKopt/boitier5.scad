composant = 1 ; // pour voir les composants
ligature=1; // pour lier les objets à l'impression
ligature_couvercle = 0;
audio = 1 ; // pour inclure le S2 et son jack
x2 = 1; // pour inclure un x2 à la place du s2
pedale = 1 ; // pour inclure la pedale 
couvercle = 0 ; 

// precision dessin (6=brouillon, 60=finition)
$fn = 6 ;

pouce=2.54;

largeurx2 = 40 ;
longueurx2 = 67 ;
largeurs2=34;
longueurs2 = 25 ;
ex2=1.2;
dys2 = 10.5*pouce ;
dzs2=-12 ;
dyjack = +3.5*pouce ;
hjack = 3 ;
dzjack = hjack + epcb / 2;


largeur=30*pouce;
x_electronique = 20*pouce; 
x_doigt = 12*pouce;
x_led = 4*pouce;
e=pouce ;
h_doigt=10;
h_electronique = (audio==1)?20:15;
hauteur = h_doigt + h_electronique;
d_empreinte = 9*pouce ;

// TT Electronics OP555 OP145
hled= 5.84; // hauteur led 
lled = 4.45 ; // largeur led
eled = 2.54 ; // epaisseur led
dhled = 1.27 ; // distance top=>center of led
dled1 = 1.8 ; // surface active led
pled = 20-6.4 -6 ; // dimension pattes led

dled = 2.8 ; // dimension trou lumiere

iled = 9*pouce ; // intervale entre leds = 22.86
hporteled = dhled + dled1/2 + 0.5 ;
dzporteled = hauteur/2 - hporteled/2 - e ; 
lporteled = lled + e ; // largeur porte led

epcb = 1.5 ;
dyteensy = -iled/2 ;
dzusbteensy = 4.3 ;
dzteensy = 0 ;


xpcbled = 2 * pouce;
ypcbled = 2*iled+2* pouce ; 
xpcb = 18 * pouce; // 45.72 mm
ypcb = largeur - 2* pouce ; // 71.12mm/2=35.56mm
dzpcb = dzporteled + hporteled/2 -hled  ;
dzpcbled = dzpcb    ;

lplot = 2.5*pouce ;
lplotcouvercle = 3.5*pouce;
dxplot1= -x_electronique/2+ lplotcouvercle/2  + e/2 ;
dxplot2 = x_electronique/2+x_doigt+x_led-2*e-lplotcouvercle/2 -e/2 ;
dyplot1 = -largeur/2  + lplotcouvercle/2 + e/2  ;
dyplot2 = largeur/2 - lplotcouvercle/2 - e/ 2;
dxplotc = x_electronique/2+x_doigt/2-8;
dyplotc = -10 ;

htetevis=1.65;
dtetevis=5.6;
dvis=2.5;
dtrouvis=1.5;
dzvis = e-htetevis - 0.25 ;

// TT Eletronics 404 series
h_tt404 = 8.51 ; 
l_tt404 = 12.7 ;
L_tt404 = 26.92 ;
dz_tt404 = -8.51/2+3.56 ;
d_tt404 = 4.85 ;
da_tt404 = -l_tt404/2 + 7.88;
dt_tt404 = 3.56 ;
x_tt404 = x_electronique/2 + x_doigt/2 + pouce ;
laxe_tt404 = 20.57 ;
dy_tt404 = laxe_tt404 - 12.7 - e  ;
dtrou_tt404 = 1.93 ;
daxe_tt404 = 24.13 ;

module tt404()
{
    difference()
    {
        // corps
        translate([0,-L_tt404/2,0])
            cube([l_tt404,L_tt404,h_tt404],center=true);
       // trou vis
        translate([dt_tt404-l_tt404/2,-L_tt404/2+daxe_tt404/2,0])
            cylinder(h=10,d=dtrou_tt404+0.5,center=true);
        translate([dt_tt404-l_tt404/2,-L_tt404/2+daxe_tt404/2+dtrou_tt404/2,0])
            cube([dtrou_tt404,dtrou_tt404+0.5,10],center=true);
       // trou vis
        translate([dt_tt404-l_tt404/2,-L_tt404/2-daxe_tt404/2,0])
            cylinder(h=10,d=dtrou_tt404+0.5,center=true);
        translate([dt_tt404-l_tt404/2,-L_tt404/2-daxe_tt404/2-dtrou_tt404/2,0])
            cube([dtrou_tt404,dtrou_tt404+0.5,10],center=true);
    }
    // bouton utilisateur
    translate([da_tt404,laxe_tt404/2,dz_tt404])
        rotate([90,0,0])
            cylinder(h=laxe_tt404,d=d_tt404,center=true);
    // picots
    translate([3.5,-6.8,8.51/2+1.5])
        cube([1,1,3], center=true);
    translate([3.5,-15,8.51/2+1.5])
        cube([1,1,3], center=true);
    translate([3.5,-20,8.51/2+1.5])
        cube([1,1,3], center=true);
}
module led()
{
    //translate([0,0,-hled/2 + dhled])
    union()
    {
        difference()
        {
            cube([eled,lled,hled],center=true);
            translate([eled/2,0,hled/2 - dhled])
                cube([1,dled1,dled1],center=true);
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
    difference()
    {
        translate([longueurx2/2-4.5,0,ex2/2])
        union()
        {
            // connecteur
            translate([-28,0,4.5])
                cube([5,13*pouce,9],true);
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
    union()
    {
        difference()
        {
            translate([0,0,0]) cube([xpcbled ,ypcbled  ,epcb],true);
            // grile pouces
            for ( i = [-1,0,1] )
            {
                for ( j = [-6,-5,-3,-2,-1,0,8,9,12] )
                {
                translate([(i+0.5)*pouce,(j+0.5)*pouce,0]) cube([0.5,0.5,5],center=true);
                }
            }
        }
    }
}
module pcb()
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
        sphere(r=e,center=true);
    }

 }
module dedans_electronique(x,y,z)
{
    scale([(x-e)/x,(y-e)/y,(z-e)/z])
        contour_electronique(x,y,z);    
}
module porteled(l)
{
    union()
    {
        difference()
        {
            // tunnel
            translate([(l+eled)/2,0,0])
                cube([eled + e + l + eled ,lporteled,hporteled ],center=true);
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
            translate([0,0,-hled/2+hporteled/2]) led();
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
    if ( composant)
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
module plot_couvercle(dx,xy)
{
    translate([dx,xy,-hauteur/2 + e/2 ])
        plot(1.5*lplot,lplotcouvercle,false);
}
module plot_tt404(hplot)
{
   translate([0,-L_tt404/2,-hplot/2])
        union()
        {
            // socle
             difference()
             {
                    cube([l_tt404,L_tt404,hplot  ],center=true);
                    // trous pour les connections
                    translate([l_tt404/4,0,0])
                        cube([l_tt404/2,L_tt404-6,hplot ],center=true);
             } 
             //cylindres pour le tenir
             translate([dt_tt404-l_tt404/2,daxe_tt404/2,-(4)/2-hplot/2])
                cylinder(h=4,d=dtrou_tt404-0.1,center=true);
              translate([dt_tt404-l_tt404/2,-daxe_tt404/2,-(4)/2-hplot/2])
                cylinder(h=4,d=dtrou_tt404-0.1,center=true);
        }
    *translate([0,0,-h_tt404/2 -hplot-1])
        tt404();
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
                    translate([x_doigt/2 + x_electronique/2 -pouce ,nrled * iled,dzporteled  ])
                       cube([x_doigt+4*pouce,dled,dled],center=true);
                }
            }
            // couvercle_fond
            translate([0,0,-hauteur/2]) fond(0.25,0) ;
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
            if ( pedale == 1)
            {
                // trou pedale
                translate([x_tt404 - da_tt404 ,-largeur/2,-hauteur/2 + e/2 + dz_tt404 + h_tt404 /2  ] )
                {
                    rotate([90,0,0])
                        cylinder(h=10,d=d_tt404  + 1 ,center = true);
                    translate([0,0,-4])
                        cube([d_tt404  + 1, 6*e,8],center=true);
                }
            }
       } 
       // chemins optiques   
       for(nrled = [-1:1] )
        {
            // recepteur
            difference()
            {
                translate([-(x_electronique -3*e )/2 +  e,nrled * iled,dzporteled  ])
                   porteled(x_electronique - 3*e );
                echancrure_empreinte(x_electronique,largeur,hauteur,nrled * iled);
            }
            // emetteur
            translate([x_doigt + x_electronique/2 -pouce/2 ,nrled * iled,dzporteled ])
                    rotate([0,0,180])
                        porteled(pouce);
        }
    }
    // couvercle avec trous de vis
    if ( couvercle == 1 )
    {
        translate([0,0,-hauteur/2 -4 ])
            union()
            {
                difference()
                {
                    fond(0,-0.5);
                    trou_couvercle(dxplot1,dyplot1);
                    trou_couvercle(dxplot2,dyplot1);
                    trou_couvercle(dxplot1,dyplot2);
                    trou_couvercle(dxplot2,dyplot2);
                    trou_couvercle(dxplotc,dyplotc);
                }
                //attaches pour l'impression
                if ( ligature_couvercle == 1)
                {
                    translate([dxplot1-2,dyplot1-2,3])cube([1,1,6],center=true);
                    translate([dxplot2+2,dyplot2+2,3])cube([1,1,6],center=true);
                }
            }
    }
        
    // fixation
    union()
    {
        // pcb
        for(j=[-5.5*pouce,6.5*pouce]) 
        {
            for(i=[-12.5*pouce,12.5*pouce ])
            {
                echo ("dplotpcb x,y= ", j, i);
                translate([j,i,dzpcb + epcb/2 ])
                    plot(hled + e,lplot,true);
            }
        }
        // pcbled
        translate([x_electronique/2+x_doigt,-lplot ,dzpcbled])
                plot(hled + e,lplot,true);
        // couvercle
        plot_couvercle(dxplot1,dyplot1);
        plot_couvercle(dxplot2,dyplot1);
        plot_couvercle(dxplot1,dyplot2);
        plot_couvercle(dxplot2,dyplot2);
        translate([dxplotc,dyplotc,-hauteur/2 +e/2 ])
            plot( hauteur - h_doigt -e ,lplot,false);
        //tt404
        if ( pedale == 1 )
        {
            translate([x_tt404,-largeur/2+e/2+dy_tt404,hauteur/2 - h_doigt -e/2]) 
                rotate([0,0,180])
                    plot_tt404(hauteur/2 -h_tt404 + hauteur/2 - h_doigt -e - 0.25 );
        }
        //x2
        translate([-3*pouce,-12.5*pouce,dzpcb+dzs2+epcb/2])
        union()
        {
            plot(-dzs2 -epcb  ,lplot,true);
            if ( ligature == 1)
            {
                translate([0,-3,3])
                rotate([90,0,0]) cube([1,1,5],center=true);
            }
        }
    }
     // composants
    if (composant == 1)
    {
        // pcb
        translate([-x_electronique/2 + xpcb/2 + pouce /2 ,0,dzpcb - epcb/2 -0.2 ] ) 
            pcb() ;
        // pcbled
        translate([+x_electronique/2 + x_doigt + x_led/2 -e -pouce ,0 , dzpcbled - epcb/2 -0.2  ] ) 
            pcbled() ;
        if ( pedale == 1 )
        {
            translate([x_tt404,-largeur/2+e/2+dy_tt404,-hauteur/2+h_tt404/2 + e / 2]) 
                rotate([0,0,180])
                    tt404();
        }
     }

}
*plot_tt404(10);
*tt404();
*plot(6,lplot,true);
*pcb();
*pcbled();
*teensy2();
*porteled(5);
*led();
*chanfrein(10,10,3);
*s2(); 
*x2();
difference()
{
    boite() ;
    //translate([123,00,0]) //bord pcb
    //translate([153,00,0]) //bord sans led
    translate([155,00,0]) //bord avec led
        cube([200,200,200], center = true);
}
echo("largeur=",largeur," longueur=",x_electronique+x_doigt+x_led, " hauteur=",hauteur);
echo ("xpcb=",xpcb, " ypcb=",ypcb);