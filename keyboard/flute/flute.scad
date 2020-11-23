$fn = 40;

e=2; // epaisseur paroi
e_feutre = 0.5 ; // epaisseur feutre 

a_bec=30; // angle arc de cercle pour la sortie vers le bec

d_raccord_sifflet = 15 ; // diametre du sifflet interne
d_raccord_externe_sifflet = 18 ; // diamètre du sifflet externe
l_racord_male_sifflet=15;// longueur du tube male enquillant le sifflet
l_raccord_sifflet = 15 ; // longueur du transformateur de forme sifflet / tube

d_tube= sqrt(3.14159*d_raccord_sifflet*d_raccord_sifflet/4); //largeur tube interne
d_tore=150; // diametre du tore
d_axe=4;
h_axe=3;
l_bec = 25; // longueur total bec
l_embouchure = 20; // longueur bec
h_embouchure = 0.5 ; // hauteur embouchure intérieure
l_raccord_embouchure = 15 ; // racord embouchure
a_sifflet = atan(e/(l_bec/2)) ; // angle siffelt fenetre
l_bouchon= 2.5+3*e;
d_vis = 3 ; // diametre vis axe


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
			square(size=d_tube+2*e,center=true);
			square(size=d_tube,center=true);
		}
	}
}
module tube_notes()
{
	difference()
    {
        tore(180);
		translate([0,0,d_tube/2+e/2])
			bouchon(0,0);
    }
    translate([0,0,-d_tube/2-e])
		axe();
}
module raccord_bec()
{
	// raccord entre tube_bec et un sifflet standard
	difference()
	{
		hull()
		{
			rotate([0,90,0])
				cylinder(d=d_raccord_sifflet+2*e,h=1,center=true);
			translate([l_raccord_sifflet,0,0])
				cube([1,d_tube+2*e,d_tube+2*e],center=true);
		}
		hull()
		{
			rotate([0,90,0])
				cylinder(d=d_raccord_sifflet,h=1,center=true);
			translate([l_raccord_sifflet,0,0])
				cube([1,d_tube,d_tube],center=true);
		}
	}
	translate([-l_racord_male_sifflet/2-0.5,0,0])
		rotate([0,90,0])
			difference()
			{
				cylinder(d=d_raccord_externe_sifflet,h=l_racord_male_sifflet,center=true);
				cylinder(d=d_raccord_sifflet,h=l_racord_male_sifflet,center=true);
			}
}
module tube_bec()
{
	// sortie du tore
	difference()
    {
        translate([-d_tore,0,0])
            rotate([0,0,-a_bec])
				tore(a_bec);
		translate([0,0,d_tube/2+e/2])
			bouchon(0,0);
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
						cylinder(d=d_tore+d_tube -e-2*(dd*e_feutre)+0.1,h=e+(1-dd)*e_feutre,center=true);
						// bord bisauté
						translate([0,0,-e/2])
							bord_bouchon(dd);
						// axe
						translate([0,0,-h_axe/2-e/2])
							cylinder(d=d_axe-0.1,h=h_axe,center=true);
					}
					// trou pour regler la heuteur de son
					rotate([0,0,32])
						trou();
					// evidements varies
					for(i=[0:8])
						rotate([0,0,i*360/8])
							translate([20,0,0])
								cylinder(d=12,h=e,center=true);
					for(i=[0:8])
						rotate([0,0,(i+0.5)*360/8])
							translate([40,0,0])
								cylinder(d=27,h=e,center=true);
					for(i=[0:8])
						rotate([0,0,i*360/8])
							translate([55,0,0])
								cylinder(d=15,h=e,center=true);
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
        cube([l_bec,d_tube+2*e,d_tube+2*e],center=true);
        cube([l_bec,d_tube,d_tube],center=true);
        // sifflet
        translate([0,-(d_tube)/2,(d_tube)/2])
            rotate([0,-a_sifflet,0])
                cube([2*d_tube,d_tube,2*d_tube],center=false);
        translate([-l_bec/2,-d_tube/2,(d_tube-e)/2-e/2])
            cube([l_bec/2,d_tube,2*d_tube],center=false);
    }
}
module embouchure()
{
	// conduit air
	translate([0,0,d_tube/2])
		difference()
		{
			cube([l_embouchure,d_tube,h_embouchure+e],center=true);
			cube([l_embouchure,d_tube-e,h_embouchure],center=true);
		}
	// bouchon
	translate([l_embouchure/2-l_bouchon/2,0,-(h_embouchure+e)/4])
		difference()
		{
			cube([l_bouchon,d_tube,d_tube-(h_embouchure+e)/2],center=true);
			cylinder(d=2.5,h=d_tube-(h_embouchure+e)/2,center=true);
		}
}
module bec()
{
	sifflet();
    translate([-13,0,0])
		embouchure();
}
module instrument()
{
	tube_notes();
	tube_bec();
	
	translate([-d_tore,0,0])
		rotate([0,0,-a_bec])
			translate([d_tore/2,-l_bec/2,0])
				rotate([0,0,90])
					raccord_bec() ;
	*translate([-d_tore,0,0])
		rotate([0,0,-a_bec])
			translate([d_tore/2,-l_bec/2,0])
				rotate([0,0,90])
					bec() ;
    translate([0,0,d_tube/2+e/2]) 
		bouchon(130,1);
}


*instrument();
*bouchon(0,1);
axe();
*bec();
*difference()
{
	bec();
	translate([0,-40,0])
		cube([80,80,80],center=true);
}
*bord_bouchon();
*raccord_bec() ;

// a imprimer
*rotate([0,90,0])
	sifflet();
*translate([0,25,0])
	rotate([0,90,0])
		embouchure() ;
*tube_bec();
*tube_notes();
*bouchon(130,1);