/*
 * Copyright (c) 2017-2018, Arthur Hennequin, LIP6, UPMC, CNRS
 * Copyright (c) 2020-2020, Lionel Lacassagne, all rights reserved, LIP6 Sorbonne University, CNRS
 */

//#include <Ellipse.h>
#include <Features.h>
#include <DebugUtil.h>
#include <stdio.h>
#include <macro_debug.h>


/* ----------------------- */
void init_MeteorROI (MeteorROI *stats, int n)
/* ----------------------- */
{
        for (int i=0 ; i<n ; i++) {
                stats[i].ID   = 0;
                stats[i].S    = 0;
                stats[i].x    = 0;
                stats[i].y    = 0;
                stats[i].Sx   = 0;
                stats[i].Sy   = 0;
                stats[i].Sx2  = 0;
                stats[i].Sy2  = 0;
                stats[i].Sxy  = 0;
                stats[i].xmin = 0;
                stats[i].ymin = 0;
                stats[i].xmax = 0;
                stats[i].ymax = 0;
                stats[i].dx   = 0.0;
                stats[i].dy   = 0.0;
                stats[i].error= 0.0;
                stats[i].prev = 0;
                stats[i].next = 0;
                stats[i].time = 0;
                stats[i].motion = 0;
                stats[i].state = 0;

        }
}

// -----------------------------------------------------------------------------------------
void extract_features(uint32** img, int i0, int i1, int j0, int j1, MeteorROI* stats, int n)
// -----------------------------------------------------------------------------------------
{
    for (int i=1 ; i<=n ; i++) {
        stats[i].S    = 0;
        stats[i].ID   = 0;
        stats[i].x    = 0.0;
        stats[i].y    = 0.0;
        stats[i].dx   = 0.0;
        stats[i].dy   = 0.0;
        stats[i].error= 0.0;
        stats[i].Sx   = 0;
        stats[i].Sy   = 0;
        stats[i].Sx2  = 0;
        stats[i].Sy2  = 0;
        stats[i].Sxy  = 0;
        stats[i].xmin = j1;
        stats[i].xmax = j0;
        stats[i].ymin = i1;
        stats[i].ymax = i0;
        stats[i].prev = 0;
        stats[i].next = 0;
        stats[i].time = 0;
        stats[i].motion = 0;
        stats[i].state = 0;
    }
    
    for (int i=i0 ; i<=i1 ; i++) {
        for (int j=j0 ; j<=j1 ; j++) {
        
            uint32 e = img[i][j];
            if (e > 0) {
                stats[e].S += 1;

                stats[e].ID = e;

                stats[e].Sx += j;
                stats[e].Sy += i;
                
                stats[e].Sx2 += j*j;
                stats[e].Sy2 += i*i;
                stats[e].Sxy += j*i;
                
                if (j < stats[e].xmin) stats[e].xmin = j;
                if (j > stats[e].xmax) stats[e].xmax = j;
                if (i < stats[e].ymin) stats[e].ymin = i;
                if (i > stats[e].ymax) stats[e].ymax = i;
            }
            
        }
    }

    for(int i=1; i<=n; i++){
        stats[i].x = (double)stats[i].Sx/(double)stats[i].S;
        stats[i].y = (double)stats[i].Sy/(double)stats[i].S;
    }

}


// -------------------------------------------------------------------------
void merge_HI_CCL_v2(uint32** HI, uint32** M, int i0, int i1, int j0, int j1, MeteorROI* stats, int n, int S_min, int S_max)
// -------------------------------------------------------------------------
{
    int x0, x1, y0, y1, id;
    MeteorROI cc;

    for(int i=1; i<=n; i++){
        cc = stats[i];
        if(cc.S){
            id = cc.ID;
            if (S_min > cc.S || cc.S > S_max ){
                stats[i].S = 0;
                /* JUSTE POUR DEBUG (Affichage frames)*/
                x0 = cc.ymin;
                x1 = cc.ymax;
                y0 = cc.xmin;
                y1 = cc.xmax;
                for(int k=x0; k<=x1; k++){
                    for(int l=y0; l<=y1; l++){
                        if (M[k][l] == id)
                            HI[k][l] = 0;
                    }
                }
                continue;
            }
            x0 = cc.ymin;
            x1 = cc.ymax;
            y0 = cc.xmin;
            y1 = cc.xmax;
            for(int k=x0; k<=x1; k++){
                for(int l=y0; l<=y1; l++){
                    if(HI[k][l]){
                        for(k=x0; k<x1; k++){
                            for(l=y0; l<y1; l++){
                                if (M[k][l] == id)
                                    HI[k][l] = i;
                            }
                        }
                        goto next;
                    }
                }
                    
            }
            stats[i].S=0;
next:;
        }
    }

}


// -------------------------------------------------------------------------
void filter_surface(MeteorROI* stats, int n, uint32** img, uint32 threshold_min, uint32 threshold_max)
// -------------------------------------------------------------------------
{
    // Doit on vraiment modifier l'image de départ? ou juste les stats.
    uint32 S, e;
    int i0, i1, j0, j1;
    uint16 id;

    for (int i=1 ; i<=n ; i++) {
        S = stats[i].S;
        id = stats[i].ID;

        if (S == 0) continue; // DEBUG

        if ( S < threshold_min || S > threshold_max) {
            stats[i].S = 0;
            
            // pour affichage debbug
            i0 = stats[i].ymin;
            i1 = stats[i].ymax;
            j0 = stats[i].xmin;
            j1 = stats[i].xmax;
            for (int i=i0 ; i<=i1 ; i++) {
                for (int j=j0 ; j<=j1 ; j++) {
                    e = img[i][j];
                    if (e == id) { 
                        img[i][j] = 0;
                    }
                }
            }
        }
    }
}


// -----------------------------------------------------
int shrink_stats(MeteorROI *stats_src, MeteorROI *stats_dest, int n)
// -----------------------------------------------------
{
    int cpt = 0;

    for (int i = 1; i<= n; i++){
        if(stats_src[i].S > 0){
            cpt++;
        
            stats_dest[cpt].S    = stats_src[i].S     ;
            stats_dest[cpt].ID   = cpt                ;
            stats_dest[cpt].x    = stats_src[i].x     ;
            stats_dest[cpt].y    = stats_src[i].y     ;
            stats_dest[cpt].dx   = stats_src[i].dx    ;
            stats_dest[cpt].dy   = stats_src[i].dy    ;
            stats_dest[cpt].error= stats_src[i].error ;
            stats_dest[cpt].Sx   = stats_src[i].Sx    ;
            stats_dest[cpt].Sy   = stats_src[i].Sy    ;
            stats_dest[cpt].Sx2  = stats_src[i].Sx2   ;
            stats_dest[cpt].Sy2  = stats_src[i].Sy2   ;
            stats_dest[cpt].Sxy  = stats_src[i].Sxy   ;
            stats_dest[cpt].xmin = stats_src[i].xmin  ;
            stats_dest[cpt].xmax = stats_src[i].xmax  ;
            stats_dest[cpt].ymin = stats_src[i].ymin  ;
            stats_dest[cpt].ymax = stats_src[i].ymax  ;
            stats_dest[cpt].prev = stats_src[i].prev  ;
            stats_dest[cpt].next = stats_src[i].next  ;
            stats_dest[cpt].time = stats_src[i].time  ;
            stats_dest[cpt].motion = stats_src[i].motion  ;
            stats_dest[cpt].state = stats_src[i].state  ;

        }
    }

    return cpt;
}

// -----------------------------------------------------
void rigid_registration(MeteorROI* stats0, MeteorROI* stats1, int n0, int n1, double* theta, double* tx, double* ty)
// -----------------------------------------------------
{
    double Sx, Sxp, Sy, Syp, Sx_xp, Sxp_y, Sx_yp, Sy_yp;
    MeteorROI cc0, cc1;
    double x0, y0, x1, y1; 
    double a, b;
    double xg, yg, xpg, ypg;
    int asso;
    int cpt;

    
    Sx    = 0;
    Sxp   = 0;
    Sy    = 0;
    Syp   = 0;
    Sx_xp = 0;
    Sxp_y = 0;
    Sx_yp = 0;
    Sy_yp = 0;
    cpt = 0;


    // parcours tab assos
    for(int i=1; i<=n0; i++){
        cc0 = stats0[i];
        asso = stats0[i].next; // assos[i];
        
        if (cc0.S>0 && asso){
            cpt++;
            cc1 = stats1[stats0[i].next];

            Sx    += cc0.x;
            Sy    += cc0.y;
            Sxp   += cc1.x;
            Syp   += cc1.y;

        }
    }
    
    xg   = Sx  / cpt;
    yg   = Sy  / cpt;
    xpg  = Sxp / cpt;
    ypg  = Syp / cpt;

    Sx    = 0;
    Sxp   = 0;
    Sy    = 0;
    Syp   = 0;


    // parcours tab assos
    for(int i=1; i<=n0; i++){
        cc0 = stats0[i];
        asso = stats0[i].next;


        
        if (cc0.S>0 && asso){
            // cpt++;
            cc1 = stats1[stats0[i].next];

            x0 = cc0.x - xg;
            y0 = cc0.y - yg;
            x1 = cc1.x - xpg;
            y1 = cc1.y - ypg;


            Sx    += x0;
            Sy    += y0;
            Sxp   += x1;
            Syp   += y1;
            Sx_xp += x0 * x1;
            Sxp_y += x1 * y0;
            Sx_yp += x0 * y1;
            Sy_yp += y0 * y1;

        }
    }
    a = cpt*cpt * (Sx_yp - Sxp_y) + (1 - 2 * cpt) * (Sx * Syp - Sxp * Sy);
    b = cpt*cpt * (Sx_xp + Sy_yp) + (1 - 2 * cpt) * (Sx * Sxp + Syp * Sy);
    

    *theta = atan2(a,b); 
    *tx = xpg - cos(*theta) * xg + sin(*theta) * yg;
    *ty = ypg - sin(*theta) * xg - cos(*theta) * yg;
 
}

// -----------------------------------------------------
void rigid_registration_corrected(MeteorROI* stats0, MeteorROI* stats1, int n0, int n1, double* theta, double* tx, double* ty, double errMoy, double eType)
// -----------------------------------------------------
{
    double Sx, Sxp, Sy, Syp, Sx_xp, Sxp_y, Sx_yp, Sy_yp;
    MeteorROI cc0, cc1;
    double x0, y0, x1, y1; 
    double a, b;
    double xg, yg, xpg, ypg;
    int asso;
    int cpt;

    
    Sx    = 0;
    Sxp   = 0;
    Sy    = 0;
    Syp   = 0;
    Sx_xp = 0;
    Sxp_y = 0;
    Sx_yp = 0;
    Sy_yp = 0;
    cpt = 0;

    int cpt1 = 0;
    // parcours tab assos
    for(int i=1; i<=n0; i++){
        cc0 = stats0[i];

        if (fabs(stats0[i].error-errMoy) >  eType) {
            stats0[i].motion = 1; 
            // fdisp(stats0[i].error);
            cpt1++;
            continue; 
        }
        asso = stats0[i].next; // assos[i];
        
        if (cc0.S>0 && asso){
            cpt++;
            cc1 = stats1[stats0[i].next];

            Sx    += cc0.x;
            Sy    += cc0.y;
            Sxp   += cc1.x;
            Syp   += cc1.y;

        }
    }
    
    xg   = Sx  / cpt;
    yg   = Sy  / cpt;
    xpg  = Sxp / cpt;
    ypg  = Syp / cpt;

    Sx    = 0;
    Sxp   = 0;
    Sy    = 0;
    Syp   = 0;

    idisp(cpt1);
    // parcours tab assos
    for(int i=1; i<=n0; i++){
        cc0 = stats0[i];

        if (fabs(stats0[i].error-errMoy) >  eType) continue; 

        asso = stats0[i].next;

        
        if (cc0.S>0 && asso){
            // cpt++;
            cc1 = stats1[stats0[i].next];

            x0 = cc0.x - xg;
            y0 = cc0.y - yg;
            x1 = cc1.x - xpg;
            y1 = cc1.y - ypg;


            Sx    += x0;
            Sy    += y0;
            Sxp   += x1;
            Syp   += y1;
            Sx_xp += x0 * x1;
            Sxp_y += x1 * y0;
            Sx_yp += x0 * y1;
            Sy_yp += y0 * y1;

        }
    }
    a = cpt*cpt * (Sx_yp - Sxp_y) + (1 - 2 * cpt) * (Sx * Syp - Sxp * Sy);
    b = cpt*cpt * (Sx_xp + Sy_yp) + (1 - 2 * cpt) * (Sx * Sxp + Syp * Sy);
    

    *theta = atan2(a,b); 
    *tx = xpg - cos(*theta) * xg + sin(*theta) * yg;
    *ty = ypg - sin(*theta) * xg - cos(*theta) * yg;
 
}
/*
// -----------------------------------------------------
void rigid_registration(MeteorROI* stats0, MeteorROI* stats1, int n0, int n1, double* theta, double* tx, double* ty)
// -----------------------------------------------------
{
    double Sx, Sxp, Sy, Syp, Sx_xp, Sxp_y, Sx_yp, Sy_yp;
    MeteorROI cc0, cc1;
    double x0, y0, x1, y1; 
    double a, b;
    double xg, yg, xpg, ypg;
    int asso;
    int cpt;

    
    Sx    = 0;
    Sxp   = 0;
    Sy    = 0;
    Syp   = 0;
    Sx_xp = 0;
    Sxp_y = 0;
    Sx_yp = 0;
    Sy_yp = 0;
    cpt = 0;


    // parcours tab assos
    for(int i=1; i<=n0; i++){
        cc0 = stats0[i];
        asso = stats0[i].next; // assos[i];
        
        if (cc0.S>0 && asso){
            cpt++;
            cc1 = stats1[stats0[i].next];

            Sx    += cc0.x;
            Sy    += cc0.y;
            Sxp   += cc1.x;
            Syp   += cc1.y;

        }
    }
    
    xg   = Sx  / cpt;
    yg   = Sy  / cpt;
    xpg  = Sxp / cpt;
    ypg  = Syp / cpt;

    Sx    = 0;
    Sxp   = 0;
    Sy    = 0;
    Syp   = 0;


    // parcours tab assos
    for(int i=1; i<=n0; i++){
        cc0 = stats0[i];
        asso = stats0[i].next;


        
        if (cc0.S>0 && asso){
            cc1 = stats1[stats0[i].next];

            x0 = cc0.x - xg;
            y0 = cc0.y - yg;
            x1 = cc1.x - xpg;
            y1 = cc1.y - ypg;


            Sx    += x0;
            Sy    += y0;
            Sxp   += x1;
            Syp   += y1;
            Sx_xp += x0 * x1;
            Sxp_y += x1 * y0;
            Sx_yp += x0 * y1;
            Sy_yp += y0 * y1;

        }
    }
    a = cpt*cpt * (Sx_yp - Sxp_y) + (1 - 2 * cpt) * (Sx * Syp - Sxp * Sy);
    b = cpt*cpt * (Sx_xp + Sy_yp) + (1 - 2 * cpt) * (Sx * Sxp + Syp * Sy);
    
    // *theta = atan2(a,b); 
    *theta = 0; 
    *tx = xpg - cos(*theta) * xg + sin(*theta) * yg;
    *ty = ypg - sin(*theta) * xg - cos(*theta) * yg;
 
}

// -----------------------------------------------------
void rigid_registration_corrected(MeteorROI* stats0, MeteorROI* stats1, int n0, int n1, double* theta, double* tx, double* ty, double errMoy, double eType)
// -----------------------------------------------------
{
    double Sx, Sxp, Sy, Syp;
    MeteorROI cc0, cc1;
    double x0, y0, x1, y1; 
    double xg, yg, xpg, ypg;
    int asso;
    int cpt;

    
    Sx    = 0;
    Sxp   = 0;
    Sy    = 0;
    Syp   = 0;
    cpt = 0;


    // parcours tab assos
    for(int i=1; i<=n0; i++){
        cc0 = stats0[i];

        if (fabs(stats0[i].error-errMoy) >  eType) continue; 
        // -----------------------------------------------------
void rigid_registration(MeteorROI* stats0, MeteorROI* stats1, int n0, int n1, double* theta, double* tx, double* ty)
// -----------------------------------------------------
{
    double Sx, Sxp, Sy, Syp, Sx_xp, Sxp_y, Sx_yp, Sy_yp;
    MeteorROI cc0, cc1;
    double x0, y0, x1, y1; 
    double a, b;
    double xg, yg, xpg, ypg;
    int asso;
    int cpt;

    
    Sx    = 0;
    Sxp   = 0;
    Sy    = 0;
    Syp   = 0;
    Sx_xp = 0;
    Sxp_y = 0;
    Sx_yp = 0;
    Sy_yp = 0;
    cpt = 0;


    // parcours tab assos
    for(int i=1; i<=n0; i++){
        cc0 = stats0[i];
        asso = stats0[i].next; // assos[i];
        
        if (cc0.S>0 && asso){
            cpt++;
            cc1 = stats1[stats0[i].next];

            Sx    += cc0.x;
            Sy    += cc0.y;
            Sxp   += cc1.x;
            Syp   += cc1.y;

        }
    }
    
    xg   = Sx  / cpt;
    yg   = Sy  / cpt;
    xpg  = Sxp / cpt;
    ypg  = Syp / cpt;

    Sx    = 0;
    Sxp   = 0;
    Sy    = 0;
    Syp   = 0;


    // parcours tab assos
    for(int i=1; i<=n0; i++){
        cc0 = stats0[i];
        asso = stats0[i].next;


        
        if (cc0.S>0 && asso){
            // cpt++;
            cc1 = stats1[stats0[i].next];

            x0 = cc0.x - xg;
            y0 = cc0.y - yg;
            x1 = cc1.x - xpg;
            y1 = cc1.y - ypg;


            Sx    += x0;
            Sy    += y0;
            Sxp   += x1;
            Syp   += y1;
            Sx_xp += x0 * x1;
            Sxp_y += x1 * y0;
            Sx_yp += x0 * y1;
            Sy_yp += y0 * y1;

        }
    }
    a = cpt*cpt * (Sx_yp - Sxp_y) + (1 - 2 * cpt) * (Sx * Syp - Sxp * Sy);
    b = cpt*cpt * (Sx_xp + Sy_yp) + (1 - 2 * cpt) * (Sx * Sxp + Syp * Sy);
    

    *theta = atan2(a,b); 
    *tx = xpg - cos(*theta) * xg + sin(*theta) * yg;
    *ty = ypg - sin(*theta) * xg - cos(*theta) * yg;
 
}

// -----------------------------------------------------
void rigid_registration_corrected(MeteorROI* stats0, MeteorROI* stats1, int n0, int n1, double* theta, double* tx, double* ty, double errMoy, double eType)
// -----------------------------------------------------
{
    double Sx, Sxp, Sy, Syp, Sx_xp, Sxp_y, Sx_yp, Sy_yp;
    MeteorROI cc0, cc1;
    double x0, y0, x1, y1; 
    double a, b;
    double xg, yg, xpg, ypg;
    int asso;
    int cpt;

    
    Sx    = 0;
    Sxp   = 0;
    Sy    = 0;
    Syp   = 0;
    Sx_xp = 0;
    Sxp_y = 0;
    Sx_yp = 0;
    Sy_yp = 0;
    cpt = 0;


    // parcours tab assos
    for(int i=1; i<=n0; i++){
        cc0 = stats0[i];

        if (fabs(stats0[i].error-errMoy) >  eType) continue; 
        
        asso = stats0[i].next; // assos[i];
        
        if (cc0.S>0 && asso){
            cpt++;
            cc1 = stats1[stats0[i].next];

            Sx    += cc0.x;
            Sy    += cc0.y;
            Sxp   += cc1.x;
            Syp   += cc1.y;

        }
    }
    
    xg   = Sx  / cpt;
    yg   = Sy  / cpt;
    xpg  = Sxp / cpt;
    ypg  = Syp / cpt;

    Sx    = 0;
    Sxp   = 0;
    Sy    = 0;
    Syp   = 0;


    // parcours tab assos
    for(int i=1; i<=n0; i++){
        cc0 = stats0[i];

        if (fabs(stats0[i].error-errMoy) >  eType) continue; 

        asso = stats0[i].next;

        
        if (cc0.S>0 && asso){
            // cpt++;
            cc1 = stats1[stats0[i].next];

            x0 = cc0.x - xg;
            y0 = cc0.y - yg;
            x1 = cc1.x - xpg;
            y1 = cc1.y - ypg;


            Sx    += x0;
            Sy    += y0;
            Sxp   += x1;
            Syp   += y1;
            Sx_xp += x0 * x1;
            Sxp_y += x1 * y0;
            Sx_yp += x0 * y1;
            Sy_yp += y0 * y1;

        }
    }
    a = cpt*cpt * (Sx_yp - Sxp_y) + (1 - 2 * cpt) * (Sx * Syp - Sxp * Sy);
    b = cpt*cpt * (Sx_xp + Sy_yp) + (1 - 2 * cpt) * (Sx * Sxp + Syp * Sy);
    

    *theta = atan2(a,b); 
    *tx = xpg - cos(*theta) * xg + sin(*theta) * yg;
    *ty = ypg - sin(*theta) * xg - cos(*theta) * yg;
 
}
        asso = stats0[i].next; // assos[i];
        
        if (cc0.S>0 && asso){
            cpt++;
            cc1 = stats1[stats0[i].next];

            Sx    += cc0.x;
            Sy    += cc0.y;
            Sxp   += cc1.x;
            Syp   += cc1.y;

        }
    }
    
    xg   = Sx  / cpt;
    yg   = Sy  / cpt;
    xpg  = Sxp / cpt;
    ypg  = Syp / cpt;

    // *theta = 0; 
    *tx = xpg - cos(*theta) * xg + sin(*theta) * yg;
    *ty = ypg - sin(*theta) * xg - cos(*theta) * yg;
}

*/

// Pour l'optimisation : faire une version errorMoy_corrected()
// ---------------------------------------------------------------------------------------------------
double errorMoy(MeteorROI *stats, int n)
// ---------------------------------------------------------------------------------------------------
{
    double S = 0.0;
    int cpt = 0;

    for(int i=1; i<=n; i++){

        if (stats[i].motion || !stats[i].next) continue;

        S+=stats[i].error;
        cpt++;
    }
    return S/cpt;
}

// Pour l'optimisation : faire une version ecartType_corrected()
// -----------------------------------------------------
double ecartType(MeteorROI *stats, int n, double errMoy)
// -----------------------------------------------------
{   
    double S = 0.0;
    int cpt = 0;
    float32 e;

    for (int i = 1; i <= n; i++){

        if (stats[i].motion || !stats[i].next) continue;

        e = stats[i].error;
        S += ((e-errMoy)*(e-errMoy));
        cpt++;
    }
    return sqrt(S/cpt);
}

// -----------------------------------------------------
void motion_extraction(MeteorROI *stats0, MeteorROI *stats1, int nc0, double theta, double tx, double ty)
// -----------------------------------------------------
{
    int cc1;
    double x, y,xp, yp;
    float32 dx, dy;
    float32 e;

    for(int i=1; i<=nc0; i++){
        cc1 = stats0[i].next; //assos[i];
        if(cc1){
            //coordonees du point dans l'image I+1
            xp = stats1[cc1].x;
            yp = stats1[cc1].y;
            //calcul de (x,y) pour l'image I
            x = cos(theta) * (xp - tx) + sin(theta) * (yp - ty);
            y = cos(theta) * (yp - ty) - sin(theta) * (xp - tx);

            // pas besoin de stocker dx et dy (juste pour l'affichage du debug)
            dx = x-stats0[i].x;
            dy = y-stats0[i].y;
            stats0[i].dx = dx;
            stats0[i].dy = dy;

            e = sqrt(dx*dx+dy*dy);
            stats0[i].error = e;           
        }
    }
}

void motion(MeteorROI *stats0, MeteorROI *stats1, int n0, int n1, double *theta, double *tx, double *ty){

    rigid_registration(stats0, stats1, n0, n1, theta, tx, ty);
    motion_extraction(stats0, stats1, n0, *theta, *tx, *ty);

    double errMoy = errorMoy(stats0, n0);
    double eType = ecartType(stats0, n0, errMoy);

    saveErrorMoy("first_error.txt", errMoy, eType);

    rigid_registration_corrected(stats0, stats1, n0, n1, theta, tx, ty, errMoy, eType);
    motion_extraction(stats0, stats1, n0, *theta, *tx, *ty);
}



// -------------------------------------------------------------
int analyse_features_ellipse(MeteorROI* stats, int n, float e_threshold)
// -------------------------------------------------------------
 {
    float32 S, Sx, Sy, Sx2, Sy2, Sxy;
    float32 x_avg, y_avg, m20, m02, m11;
    float32 a2, b2, a, b, e;
    
    int true_n = 0;
    
    for (int i=1 ; i<=n ; i++) {
        S   = stats[i].S;
        Sx  = stats[i].Sx;
        Sy  = stats[i].Sy;
        Sx2 = stats[i].Sx2;
        Sy2 = stats[i].Sy2;
        Sxy = stats[i].Sxy;
        
        if (S==0) continue;
        
        x_avg = Sx / S;
        y_avg = Sy / S;
        
        // Moments centrés
        m20 = Sx2 / S - x_avg * x_avg; // var(x)
        m02 = Sy2 / S - y_avg * y_avg; // var(y)
        m11 = Sxy / S - x_avg * y_avg; // cov(x,y) ?
        
        
        a2 = (m20 + m02 + sqrt((m20 - m02) * (m20 - m02) + 4.0 * m11 * m11)) / (2.0 * S);
        b2 = (m20 + m02 - sqrt((m20 - m02) * (m20 - m02) + 4.0 * m11 * m11)) / (2.0 * S);
        
        a = sqrt(a2);
        b = sqrt(b2);
        
        e = a / b; // garder cette ligne une fois que tout sera corrige LL 2022 
        // e = b / a; // FAUX car a grand rayon et b petit rayon par construction LL 2022
        //e = sqrt(a2 - b2) / a;
        //e = a / b;
        
        //float32 abs_diff = fabs(theta - stats[i].angle_UV);
        //float32 angle_diff = min(min(abs_diff, 360-abs_diff), fabs(180-abs_diff));
        
        // test inverse de ce qui est naturel de faire (coherent avec e = b/a, mais INVERSE !) LL 2020
        if (e > e_threshold /*&&  angle_diff > 10*/) {
            stats[i].S = 0;
            continue;
        }
        true_n++;
        /*#ifndef REAL_TIME_TEST
        printf("i = %2d | m20 = %8.1f | m02 = %8.1f | m11 = %8.1f | theta = %4.1f | a = %6.1f | b = %6.1f | e = %6.1f | |uv| = %6.1f | arg(uv) = %4.1f\n",
        i, m20, m02, m11, theta, a, b, e, stats[i].norm_UV, stats[i].angle_UV);
        #endif*/
    }
    return true_n;
}