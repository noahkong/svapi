//    **************************************************************************
//    This file is part of SVAPI.
//
//    SVAPI is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    SVAPI is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with SVAPI.  If not, see <http://www.gnu.org/licenses/>.
//    **************************************************************************

//arxiu svapi.h

#ifndef SVAPI_H
#define SVAPI_H

#include <math.h>
#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <ctype.h>
#include <cvaux.h>
#include "config.h"

//factors d'escalat
#define DEF_SF_U  160.0     //horitzontal
#define DEF_SF_V  120.0     //vertical
#define DEG2RAD M_PI/180.0  //canvi de graus a radians

//Classe svLib
//dedicada a la transformació de coordenades 3D a 2D i viceversa

class svLib{
    public:
        svLib(Config* config);      
        ~svLib();
                
        //inicialització de les estructures de dades de la llibreria
        void init();

        //establiment dels paràmetres
        void setRotation       (double ax, double ay, double az);
        void setTranslation    (double vx, double vy, double vz);
        void setScale          (double alpha_u, double alpha_v);
        void setReferencePoint (float u, float v);
        
        //transformació simple    
        //--------------------
        
        void worldToStereo     (CvPoint3D32f& p3d, 
                                CvPoint2D32f& ptL, CvPoint2D32f& ptR);
        
        //reconstrueix el punt en 3D a partir de les dues projeccions
        void stereoToWorld     (CvPoint3D32f& p3d, 
                                CvPoint2D32f& ptL, CvPoint2D32f& ptR );
           
        //transformació complexa (mètode homogeni de Hartley + projecció)
        //---------------------------------------------------------------
        
        //projecta el punt 3D a 2D sobre els dispositius de captura
        //mitjançant les matrius de projecció esquerra i dreta
        void worldToStereoH     (CvPoint3D32f& src, 
                                 CvPoint2D32f& ptL, CvPoint2D32f& ptR);
        
        //reconstrueix el punt en 3D a partir de les dues projeccions
        bool stereoToWorldH (CvPoint3D32f& p3d, 
                             CvPoint2D32f& ptL, CvPoint2D32f& ptR );

    	
    private:
    	
        //projecta un punt 3D a 2D a partir de les seves coordenades i la matriu
        void projectTo2D (CvPoint3D32f& src, CvMat *P, 
                               CvPoint2D32f& point2d);

        //reconstrueix un punt de 2D a 3D amb el mètode homogeni
        CvPoint3D32f reconstructTo3DH (const CvMat *RightPM, 
                                       double xL, double yL, 
                                       double xR, double yR);

        //assignació de matrius
        void setAntiSymetricMatrix (CvMat *mat, double x, double y, double z);
        
        //computa matriu fonamental
        void calcF (CvMat *A, CvMat *Rot, double *vector, CvMat *F, 
                    CvMat *E=NULL);

        //resolem A · X = 0 , necessari per la reconstrucció 3D
        double solve ();
                                                                     
        //computació
        void calc ();         //computa matrius de projecció L, R
        void calcFromRT ();   //computa a partir de la rotació i traslació
        //computa matriu de rotació
        void calcRotationMatrix (CvMat *R, double a_x, double a_y, double a_z);
                                                                                                                                
        Config* mConfig;    //punter a la configuració

    	CvMat *E;	//matriu essencial
    	CvMat *F;	//matriu fonamental calculada a partir de R i T
    	CvMat *K;	//matriu K de paràmetres intrínsecs (3x3)
    	CvMat *A;	//matriu A utilitzada en reconstrucció (4x4)
    	CvMat *X;   //vector utilitzat per a la reconstrucció (4x1)
        	
    	bool mProjOk;	  //indica si les matrius han estat calculades
    
        bool mRefPointOk; //indica si s'ha guardat el punt de referència
        		
    	double mTrans[3];	 //vector de la càmera dreta a l'esquerra
    	double mRot[3];      //rotació de la càmara dreta respecte de l'esquerra
    
    	double mScaleU;	     //factor d'escala horitzontal de les càmeres, 
                             //píxels per metre                             
    	double mScaleV;	     //factor d'escala vertical de les càmeres, 
                             //píxels per metre
                             
    	float  mUref, mVref;   //punt de referència de les càmeres

        //atributs necessaris
        float f;            //distància focal (en mm)
        float b;            //distància entre càmeres (en mm)
            
    	CvMat *PL;	        //matriu de projecció esquerra
    	CvMat *PR;	        //matriu de projecció dreta
};

#endif
