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
//dedicada a la transformaci� de coordenades 3D a 2D i viceversa

class svLib{
    public:
        svLib(Config* config);      
        ~svLib();
                
        //inicialitzaci� de les estructures de dades de la llibreria
        void init();

        //establiment dels par�metres
        void setRotation       (double ax, double ay, double az);
        void setTranslation    (double vx, double vy, double vz);
        void setScale          (double alpha_u, double alpha_v);
        void setReferencePoint (float u, float v);
        
        //transformaci� simple    
        //--------------------
        
        void worldToStereo     (CvPoint3D32f& p3d, 
                                CvPoint2D32f& ptL, CvPoint2D32f& ptR);
        
        //reconstrueix el punt en 3D a partir de les dues projeccions
        void stereoToWorld     (CvPoint3D32f& p3d, 
                                CvPoint2D32f& ptL, CvPoint2D32f& ptR );
           
        //transformaci� complexa (m�tode homogeni de Hartley + projecci�)
        //---------------------------------------------------------------
        
        //projecta el punt 3D a 2D sobre els dispositius de captura
        //mitjan�ant les matrius de projecci� esquerra i dreta
        void worldToStereoH     (CvPoint3D32f& src, 
                                 CvPoint2D32f& ptL, CvPoint2D32f& ptR);
        
        //reconstrueix el punt en 3D a partir de les dues projeccions
        bool stereoToWorldH (CvPoint3D32f& p3d, 
                             CvPoint2D32f& ptL, CvPoint2D32f& ptR );

    	
    private:
    	
        //projecta un punt 3D a 2D a partir de les seves coordenades i la matriu
        void projectTo2D (CvPoint3D32f& src, CvMat *P, 
                               CvPoint2D32f& point2d);

        //reconstrueix un punt de 2D a 3D amb el m�tode homogeni
        CvPoint3D32f reconstructTo3DH (const CvMat *RightPM, 
                                       double xL, double yL, 
                                       double xR, double yR);

        //assignaci� de matrius
        void setAntiSymetricMatrix (CvMat *mat, double x, double y, double z);
        
        //computa matriu fonamental
        void calcF (CvMat *A, CvMat *Rot, double *vector, CvMat *F, 
                    CvMat *E=NULL);

        //resolem A � X = 0 , necessari per la reconstrucci� 3D
        double solve ();
                                                                     
        //computaci�
        void calc ();         //computa matrius de projecci� L, R
        void calcFromRT ();   //computa a partir de la rotaci� i traslaci�
        //computa matriu de rotaci�
        void calcRotationMatrix (CvMat *R, double a_x, double a_y, double a_z);
                                                                                                                                
        Config* mConfig;    //punter a la configuraci�

    	CvMat *E;	//matriu essencial
    	CvMat *F;	//matriu fonamental calculada a partir de R i T
    	CvMat *K;	//matriu K de par�metres intr�nsecs (3x3)
    	CvMat *A;	//matriu A utilitzada en reconstrucci� (4x4)
    	CvMat *X;   //vector utilitzat per a la reconstrucci� (4x1)
        	
    	bool mProjOk;	  //indica si les matrius han estat calculades
    
        bool mRefPointOk; //indica si s'ha guardat el punt de refer�ncia
        		
    	double mTrans[3];	 //vector de la c�mera dreta a l'esquerra
    	double mRot[3];      //rotaci� de la c�mara dreta respecte de l'esquerra
    
    	double mScaleU;	     //factor d'escala horitzontal de les c�meres, 
                             //p�xels per metre                             
    	double mScaleV;	     //factor d'escala vertical de les c�meres, 
                             //p�xels per metre
                             
    	float  mUref, mVref;   //punt de refer�ncia de les c�meres

        //atributs necessaris
        float f;            //dist�ncia focal (en mm)
        float b;            //dist�ncia entre c�meres (en mm)
            
    	CvMat *PL;	        //matriu de projecci� esquerra
    	CvMat *PR;	        //matriu de projecci� dreta
};

#endif
