$fn =10 ;
e = 3 ;

touche_x = 25 ;
touche_y = 25 ;
touche_z = 5 ;
touche_n= 4 ;
touche_s = 2 ;
touche_dd = 1 ;

touches_dy = 20 ; 

boitier_x = (touche_n + 1)*touche_x ;
boitier_y = touche_y + 45  ;
boitier_z = 30 ;
boitier_s = 15 ;
boitier_dx = -5 ;

// bouton ckn10362 digikey  C&K HS-2206-A1 NS
bye = 24.2 ;
by = 16.4 ;
byl = bye-by ;
bxe = 17.1 ;
bx = 14.5 ;
bz = 8.2 ;
bxr = 13.7 ;
byr = 3.3 ;
bzr=2 ;
bdr = 3.5 ;
bdre = 2.1 ;
bxb= 12.3 ;

bdxv = 3 ;
bdyv = 5 ;

bp1x= 1.8 ;
bp1y=1.6 ;
bp1dx = bxb/2 - bp1x/2 ;
bp1dy = 8.25 - bp1y/2 ;

bp2x=1.4 ;
bp2y=1.8 ;
bp2dx = bxb/2 - bp2x /2 ;
bp2dy = -(8.25- bp2y / 2) ;

bpz = 2.6 - 1 ;
bpze= 2.6 ;

be = 2;

module bouton()
{
    // plateau mobile
    difference()
    {
        translate([0,by/2+(bye-by)/2,bz -1])
            cube([bx,bye-by,2],center = true);
        // trou de vis
        translate([bdxv,bdyv+by/2,bz -1])
            cylinder(d=2, h = 5 , center = true);
        translate([-bdxv,bdyv+by/2,bz -1])
            cylinder(d=2, h = 5 , center = true);
    }
    
    // boitier
    difference()
    {
        union()
        {
            //boitier corps dessus
            translate([0,0,bz/2 + 1])
                cube([bx,by,bz -2 ],center = true);
            //boitier corps dessous
            translate([0,0, 1])
                cube([bxb,by,2 ],center = true);
            // cylindres de positionnement
            translate([bxr/2,byr,bzr/2])
                cylinder(d=bdr , h = bzr , center = true );
            translate([-bxr/2,byr,bzr/2])
                cylinder(d=bdr , h = bzr , center = true );
        }
        // trous dans les cylindres de positionnement
        translate([bxr/2,byr,bzr/2])
            cylinder(d=bdre , h = bzr+2 , center = true );
        translate([-bxr/2,byr,bzr/2])
            cylinder(d=bdre , h = bzr+2 , center = true );
    }
    
    // pattes de fixation
    translate([bp1dx,bp1dy,-bpze/2])
        cube([bp1x,bp1y,bpze],center = true);
    translate([bp1dx,bp1dy+0.25,-be - (bpze -be)/2])
        cube([bp1x,bp1y+0.25,bpze -be],center = true);
    
    translate([-bp1dx,bp1dy,-bpze/2])
        cube([bp1x,bp1y,bpze],center = true);
    translate([-bp1dx,bp1dy+0.25,-be - (bpze -be)/2])
        cube([bp1x,bp1y+0.25,bpze -be],center = true);

    translate([bp2dx,bp2dy,-bpze/2])
        cube([bp2x,bp2y,bpze],center = true);
    translate([bp2dx+0.25,bp2dy,-be - (bpze -be)/2])
        cube([bp1x+0.25,bp1y,bpze -be],center = true);

    translate([-bp2dx,bp2dy,-bpze/2])
        cube([bp2x,bp2y,bpze],center = true);
    translate([-bp2dx-0.25,bp2dy,-be - (bpze -be)/2])
        cube([bp1x+0.25,bp1y,bpze -be],center = true);
    
    // pattes connecteurs
    translate([2.5,0,-2.6/2])
        cube([1,1,2.6],center = true);
    translate([-2.5,0,-2.6/2])
        cube([1,1,2.6],center = true);
    translate([2.5,5,-2.6/2])
        cube([1,1,2.6],center = true);
    translate([-2.5,5,-2.6/2])
        cube([1,1,2.6],center = true);
    translate([2.5,-5,-2.6/2])
        cube([1,1,2.6],center = true);
    translate([-2.5,-5,-2.6/2])
        cube([1,1,2.6],center = true);
}
module bouton_empreinte()
{
    bouton() ;
    translate([0,0,-be/2])
        cube([bxb, by, be], center = true);
    translate([bxr/2,byr,-be/2])
        cylinder(d=bdre,h=be,center = true);
    translate([-bxr/2,byr,-be/2])
        cylinder(d=bdre,h=be,center = true);
}

module boitier_cube(de)
{
    hull()
    {
        translate([boitier_x/2 - boitier_s/2 - de , boitier_y/2- boitier_s/2  - de, boitier_z/2- boitier_s/2 - de ])
            sphere(d=boitier_s);
        translate([-(boitier_x/2 - boitier_s/2 - de) , boitier_y/2- boitier_s/2  - de, boitier_z/2- boitier_s/2  - de])
            sphere(d=boitier_s);
        translate([boitier_x/2 - boitier_s/2 - de , -(boitier_y/2- boitier_s/2  - de) , boitier_z/2- boitier_s/2  - de])
            sphere(d=boitier_s);
        translate([-(boitier_x/2 - boitier_s/2 - de) , -(boitier_y/2- boitier_s/2 - de), boitier_z/2- boitier_s/2  - de])
            sphere(d=boitier_s);
        translate([boitier_x/2 - boitier_s/2 +boitier_dx  - de, boitier_y/2- boitier_s/2 - de , -(boitier_z/2- boitier_s/2 - de) ])
            sphere(d=boitier_s);
        translate([-(boitier_x/2 - boitier_s/2+boitier_dx  - de) , boitier_y/2- boitier_s/2 - de , - (boitier_z/2- boitier_s/2  - de)])
            sphere(d=boitier_s);
        translate([boitier_x/2 - boitier_s/2 +boitier_dx  - de, -(boitier_y/2- boitier_s/2 - de ) ,- (boitier_z/2- boitier_s/2 - de)])
            sphere(d=boitier_s);
        translate([-(boitier_x/2 - boitier_s/2+boitier_dx - de ) , -(boitier_y/2- boitier_s/2 - de), - (boitier_z/2- boitier_s/2 - de)])
            sphere(d=boitier_s);
    }
}
module touche()
{
    difference()
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
            translate([touche_x/2-touche_s/2, touche_y/2- touche_s/2,-(touche_z/2- touche_s/2)])
                sphere(d=touche_s);
            translate([-(touche_x/2-touche_s/2), touche_y/2- touche_s/2,-(touche_z/2- touche_s/2)])
                sphere(d=touche_s);
            translate([touche_x/2-touche_s/2, -(touche_y/2- touche_s/2),-(touche_z/2- touche_s/2)])
                sphere(d=touche_s);
            translate([-(touche_x/2-touche_s/2), -(touche_y/2- touche_s/2),-(touche_z/2- touche_s/2)])
                sphere(d=touche_s);
        }
        // trou de vis
        translate([bdxv,bdyv - touche_y/2,-1])
            cylinder(d=1.5, h = touche_z , center = true);
        translate([-bdxv,bdyv - touche_y/2,-1])
            cylinder(d=1.5, h = touche_z , center = true);
        // rainures
        for(i=[2:10])
        {
            translate([0,touche_y/2 - i * 2, touche_z/2])
                cube([touche_x-5,1,1],center = true);
        }
    }
}
 module touches()
{
    hull()
    {
        translate([2+((touche_n * touche_x + (touche_n + 1) * touche_dd )/2- touche_s/2) , 4+ touche_y/2 + 2 * touche_dd- touche_s/2 , 0 ])
            cylinder(d=touche_s , h = 6* touche_z , center = true ) ;
        translate([-2-((touche_n * touche_x + (touche_n + 1) * touche_dd)/2 - touche_s/2) , 4+ touche_y/2 + 2 * touche_dd- touche_s/2 , 0 ])
            cylinder(d=touche_s , h = 6* touche_z , center = true ) ;
        translate([2+((touche_n * touche_x + (touche_n + 1) * touche_dd) /2  - touche_s/2) , -(touche_y/2 + 2 * touche_dd- touche_s/2 ), 0 ])
            cylinder(d=touche_s , h = 6* touche_z , center = true ) ;
        translate([-2-((touche_n * touche_x + (touche_n + 1) * touche_dd )/2 - touche_s/2) , -(touche_y/2 + 2 * touche_dd- touche_s/2) , 0 ])
            cylinder(d=touche_s , h = 6* touche_z , center = true ) ;
    }
}
*bouton();
*bouton_empreinte();
*touche() ;
*touches();
difference()
{
    boitier_cube(0);
    boitier_cube(e);
    translate([0,touches_dy,boitier_z/2])
        touches();
    *translate([500,0,0])
        cube([1000,1000,1000],center=true);
}
for(i=[1:touche_n])
{
    translate([(i-(touche_n+1)/2) * ( touche_x + touche_dd ) , touches_dy, boitier_z/2])
        touche() ;
}
    
