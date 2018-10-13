$fn=20;
mousse=2.5;
ztr=5+mousse;
xtr=5;
ytr=1.1 ;
ltr =11*2.54 ;
e=0.8;

xbase=3*2.54;
ybase = 11*2.54;
zpin=4;
dpin=1.9;

module pin()
{
    translate([0,0,-zpin/2+0.1])
            cylinder(h=zpin,d=dpin,center=true);
}
module base()
{
    union()
    {
    translate([0,0,dpin/2]) cube([dpin,dpin,dpin],center=true);
    pin();
    }
}
module tunnel()
{
    yt=ytr+ltr;
    translate([0,(yt+2*e)/2-ytr-e,(ztr+e)/2])
            difference()
            {
                cube([xtr+2*e,yt+2*e,ztr+e],center=true);
                translate([0,e,-e])
                    cube([xtr,yt+2*e,ztr+e],center=true);
            }
}
module lien()
{
    union()
    {
        translate([3*2.54/2+3/2,0,e/4]) cube([3,e/2,e/2],center=true);
        translate([3*2.54/2+3/2,ybase,e/4]) cube([3,e/2,e/2],center=true);
    }
}

module cache()
{
    union()
    {
        tunnel() ;
        translate([xbase/2,0,0]) base();
        translate([-xbase/2,0,0]) base();
        translate([xbase/2,ybase,0]) base();
        translate([-xbase/2,ybase,0]) base();
    }
}

union()
{    
    translate([-15,0,0]) 
        union()
        {
            cache();
            lien();
        }
    translate([-5,0,0]) 
        union()
        {
            cache();
            lien();
        }
    translate([5,0,0]) 
        union()
        {
            cache();
            lien();
        }
    translate([15,0,0]) 
        union()
        {
            cache();
        }
}