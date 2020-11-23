$fn = 30;

e=2; // epaisseur paroi
e_feutre = 1 ; // epaisseur feutre 
d_tube=10; //largeur tube interne
d_tore=100; // diametre du tore
d_axe=4;
h_axe=4;
l_bec = 13; // longueur total bec
l_embouchure = 15; // longueur embouchure
h_embouchure = 0.5 ; // hauteur embouchure intérieure
l_raccord_embouchure = 2 ; // racord embouchure
a_sifflet = 7 ; // angle siffelt fenetre
l_bouchon= 3;
d_vis = 3 ; // diametre vis axe
a_bec=40; // angle arc de cercle pour la sortie vers le bec

module axe()
{
	translate([0,0,e])
    union()
    {
            rotate([0,0,5])
                union()
                { 
					// bras de portage de l'axe
                    cube([d_tore-d_tube-e/2,d_axe+ 2*e,2*e],center=true);
                    translate([0,0,((d_tube-e)/2)+e])
                    difference()
                    {
                        cylinder(d=d_axe+ 2*e,h=d_tube-e,center=true);
                        cylinder(d=d_axe,h=50,center=true);
                    }
                }
        
    }
}
module tore(a)
{
	rotate_extrude(angle = a, convexity = 10)
	{
		translate([d_tore/2,0,0])
		difference()
		{
			circle(d=d_tube+2*e);
			circle(d=d_tube);
		}
	}
}
module tube()
{
    union()
    {
        tore(180);
        translate([-d_tore,0,0])
            rotate([0,0,-a_bec])
                tore(a_bec);
    }
}
module trou()
{
    difference()
    {
        cylinder(d=d_tore+d_tube/2 + 4*e,h=2*e,center=true);
        translate([d_tore+d_tube/2,0,0])
            cube([2*(d_tore+d_tube/2),2*(d_tore+d_tube/2),2*e],center=true);
        rotate([0,0,160])
            translate([d_tore+d_tube/2,0,0])
                cube([2*(d_tore+d_tube/2),2*(d_tore+d_tube/2),2*e],center = true);
       cylinder(d=d_tore-d_tube,h=2*e,center=true);
    }
}
module bord_bouchon(dd)
{
	rotate_extrude(angle = 360, convexity = 10)
		polygon(points = [ [d_tore/2+d_tube/2-(dd*e_feutre), -(1-dd)*e_feutre/2], [d_tore/2+d_tube/2-e/2-(dd*e_feutre), -(1-dd)*e_feutre/2],[d_tore/2+d_tube/2-e/2-(dd*e_feutre), e] ]);
}
module bouchon(a,dd)
{
    rotate([0,0,a])
	difference()
	{
		union()
		{
			difference()
			{
					union()
					{
						// plateau
						cylinder(d=d_tore+d_tube -e-2*(dd*e_feutre),h=e+(1-dd)*e_feutre,center=true);
						// bord bisauté
						translate([0,0,-e/2])
							bord_bouchon(dd);
						// axe
						translate([0,0,-h_axe/2-e/2])
							cylinder(d=d_axe-0.1,h=h_axe,center=true);
					}
					// trou pour regler la heuteur de son
					rotate([0,0,-10])
						trou();
					// evidements varies
					difference()
					{
						cylinder(d=d_tore-d_tube-2*e-1,h=e,center=true);
						cylinder(d=d_tore*2/3,h=e,center=true);
					}
					cylinder(d=d_tore*2/3 - 6*e,h=e,center=true);
			}
			// renforts axiaux
			for(i=[0:8])
			{
				rotate([0,0,360*i/8])
						cube([d_tore-d_tube,d_axe,e],center=true);
			}
		}
		// trou vis
		cylinder(d=d_vis,h=20,center=true);
	}
}
module sifflet()
{
    difference()
    {
        cube([l_bec,d_tube,d_tube],center=true);
        cube([l_bec,d_tube-e,d_tube-e],center=true);
        // sifflet
        translate([-l_bec/2+l_raccord_embouchure,-(d_tube-e)/2,(d_tube-e)/2-e/2])
            rotate([0,-a_sifflet,0])
                cube([2*d_tube,d_tube-e,2*d_tube],center=false);
        translate([-l_bec/2,-(d_tube-e)/2,(d_tube-e)/2-e/2])
            cube([l_raccord_embouchure,d_tube-e,2*d_tube],center=false);
    }
}
module embouchure()
{
	// conduit air
	difference()
	{
		cube([l_embouchure,d_tube-e,h_embouchure+e],center=true);
		cube([l_embouchure,d_tube-2*e,h_embouchure],center=true);
	}
	// bouchon
	translate([l_embouchure/2-l_bouchon/2,0,-(d_tube-e-e/2)/2-h_embouchure/2-e/2])
		cube([l_bouchon,d_tube-e,d_tube-e-e/2],center=true);
}
module bec()
{
	sifflet();
    translate([-l_bec/2-l_embouchure/2+3.5,0,d_tube/2-e/2])
		embouchure();
}
module instrument()
{
    difference()
    {
        tube();
		translate([0,0,d_tube/2+e/2])
			bouchon(0,0);
    }
	*translate([-d_tore,0,0])
		rotate([0,0,-a_bec])
			translate([d_tore/2,-l_bec/2,0])
				rotate([0,0,90])
					bec() ;
    translate([0,0,d_tube/2+e/2]) 
		bouchon(130,1);
    translate([0,0,-d_tube/2-e])
		axe();
}
*bouchon(0,1);
*axe();
*instrument();
*bec();
*difference()
{
	bec();
	translate([0,-40,0])
		cube([80,80,80],center=true);
}
tube();
*bord_bouchon();
