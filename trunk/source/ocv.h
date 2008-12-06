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

//arxiu ocv.h

#ifndef OCV_H
#define OCV_H

#include "videoInput.h"
#include "cv.h"
#include "highgui.h"
#include "cvaux.h"
#include "svapi.h"
#include <time.h>
#include <stdlib.h>
#include "config.h"
#include <gtk/gtk.h>

#define DEF_IMAGE_STEP 2
#define INF 100000.0;   //infinit
#define MAX_COORD 1000  //m�xim de coordenades
    
//algoritmes de disparitat
typedef enum {DA_BIRCHFIELD_TOMASI, DA_SAD, DA_RSR} t_disp_algorithm;

//estructura de dades per a les subregions
typedef struct{
    int x;                      //coordenada x
    int y;                      //coordenada y
    int width;                  //amplada
    int height;                 //altura
    unsigned char maxd;         //disparitat m�xima
    unsigned char mind;         //disparitat m�nima
    int sum;                    //suma de les intensitats
}subregion;

//Classe Ocv
//controla esdeveniments OpenCV i el c�lcul de la correspond�ncia

class Ocv{
    public:
        Ocv  (Config* config);
        ~Ocv ();
        
        //incialitzaci� de la classe
        void ocvStart ();
        
        //finalitzaci� de la classe, elimina elements en mem�ria
        void ocvEnd ();
        
        void ocvCapture (IplImage* left, IplImage* right);
     
        //bucle principal del processament OpenCV
        void ocvLoop (bool freeze);
       
        //algoritme de correspond�ncia est�reo de finestra + SAD
        int ocvFindStereoCorrespondenceWindow_SAD (IplImage* left, 
                                                   IplImage* right, 
                                                   IplImage* depth, 
                                                   int w, int h, 
                                                   int limit, int detail, 
                                                   int levels);
                                                  
        //algoritme de correspond�ncia est�reo RSR + Correlaci� Creuada
        void ocvFindStereoCorrespondenceRSR_CC (IplImage* left, IplImage* right, 
                                                IplImage* depth, 
                                                int wwidth, int wheight, 
                                                int threshold, int limit, 
                                                int levels, 
                                                IplImage* debug1, 
                                                IplImage* debug2);
    
        //obtenci� de la textura per a la projecci� 3D
        IplImage* getTexture ();
        
        //obtenci� de la disparitat per a un p�xel (x,y)
        float getDepth (int x, int y);
            
        //par�metres de l'algoritme de disparitat (6 per BIRCHFIELD)
        int disparam11;              //indica m�xima disparitat a birchfield
        double disparam12;
        double disparam13;
        double disparam14;
        double disparam15;
        double disparam16;
        
        //par�metres de l'algoritme de finestra + SAD
        int disparam21;
        int disparam22;
        int disparam23;
        int disparam24;
        int disparam25;
        
        //par�metres de l'algoritme RSR + CC
        int disparam31;
        int disparam32;
        int disparam33;
        int disparam34;
        int disparam35;
        
        //l�mit m�xim del nivell de detall de la representaci�
        int mMaxStep;
        
        //nivell de detall actual
        int mImageStep;
        
        //algoritme de disparitat actual
        t_disp_algorithm disalg;
            
        //imatges capturades
        IplImage* frameLeft;
        IplImage* frameRight;
        IplImage* depthImage;
    
        //mapa de profunditat en mode RGB (OpenCV fa servir BGR)
        IplImage* depthImageRGB;
        
        //pixel buffers per a la representaci� Gtk+
        GdkPixbuf* pixbufleft;      //pixel buffer de la imatge esquerra
        GdkPixbuf* pixbufright;     //idem dreta
        GdkPixbuf* pixbufdepth;     //per la profunditat
            
    private:
        
        //obtenci� del nivell m�xim d'intensitat dels p�xels d'una imatge
        int getMax (IplImage* image, int step);

        //obtenci� del l�mit del nivell de detall a partir de les dimensions
        int findMaxStep (int a, int b);
                                
        //creaci� de bandes verticals a una imatge tractada amb RSR + CC
        void createVerticalStripes (subregion* sub, IplImage* image, int width, 
                                    IplImage* debug, int threshold, 
                                    int &numsub);

        //creaci� de bandes horitzontals a una imatge tractada amb RSR + CC
        void createHorizontalStripes (subregion* sub, IplImage* image, 
                                      int height, IplImage* debug, 
                                      int threshold);
                
        FILE *f;    //fitxer de log
        
        int numlin; //n� de linies trobades (l�mit superior, perque les ajuntam)
                    //les no v�lides tenen hstripex[i] = -1
        
        //estructures de mem�ria necess�ries per a l'algoritme RSR + CC
        int hstripex[MAX_COORD];
        int hstripey[MAX_COORD];
        int hstripeheight[MAX_COORD];
        
        //intensitat m�xima i m�nima d'una banda o "stripe"
        unsigned char hstripemaxd[MAX_COORD];
        unsigned char hstripemind[MAX_COORD];
        
        //llistat de subregions
        subregion* subl;
        
        //punter a la configuraci�
        Config* mConfig;            

        //imatges esquerra i dreta en format RGB (no BGR d'OpenCV)
        IplImage* frameLeftRGB;
        IplImage* frameRightRGB;
};

#endif
