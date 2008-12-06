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
#define MAX_COORD 1000  //màxim de coordenades
    
//algoritmes de disparitat
typedef enum {DA_BIRCHFIELD_TOMASI, DA_SAD, DA_RSR} t_disp_algorithm;

//estructura de dades per a les subregions
typedef struct{
    int x;                      //coordenada x
    int y;                      //coordenada y
    int width;                  //amplada
    int height;                 //altura
    unsigned char maxd;         //disparitat màxima
    unsigned char mind;         //disparitat mínima
    int sum;                    //suma de les intensitats
}subregion;

//Classe Ocv
//controla esdeveniments OpenCV i el càlcul de la correspondència

class Ocv{
    public:
        Ocv  (Config* config);
        ~Ocv ();
        
        //incialització de la classe
        void ocvStart ();
        
        //finalització de la classe, elimina elements en memòria
        void ocvEnd ();
        
        void ocvCapture (IplImage* left, IplImage* right);
     
        //bucle principal del processament OpenCV
        void ocvLoop (bool freeze);
       
        //algoritme de correspondència estèreo de finestra + SAD
        int ocvFindStereoCorrespondenceWindow_SAD (IplImage* left, 
                                                   IplImage* right, 
                                                   IplImage* depth, 
                                                   int w, int h, 
                                                   int limit, int detail, 
                                                   int levels);
                                                  
        //algoritme de correspondència estèreo RSR + Correlació Creuada
        void ocvFindStereoCorrespondenceRSR_CC (IplImage* left, IplImage* right, 
                                                IplImage* depth, 
                                                int wwidth, int wheight, 
                                                int threshold, int limit, 
                                                int levels, 
                                                IplImage* debug1, 
                                                IplImage* debug2);
    
        //obtenció de la textura per a la projecció 3D
        IplImage* getTexture ();
        
        //obtenció de la disparitat per a un píxel (x,y)
        float getDepth (int x, int y);
            
        //paràmetres de l'algoritme de disparitat (6 per BIRCHFIELD)
        int disparam11;              //indica màxima disparitat a birchfield
        double disparam12;
        double disparam13;
        double disparam14;
        double disparam15;
        double disparam16;
        
        //paràmetres de l'algoritme de finestra + SAD
        int disparam21;
        int disparam22;
        int disparam23;
        int disparam24;
        int disparam25;
        
        //paràmetres de l'algoritme RSR + CC
        int disparam31;
        int disparam32;
        int disparam33;
        int disparam34;
        int disparam35;
        
        //límit màxim del nivell de detall de la representació
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
        
        //pixel buffers per a la representació Gtk+
        GdkPixbuf* pixbufleft;      //pixel buffer de la imatge esquerra
        GdkPixbuf* pixbufright;     //idem dreta
        GdkPixbuf* pixbufdepth;     //per la profunditat
            
    private:
        
        //obtenció del nivell màxim d'intensitat dels píxels d'una imatge
        int getMax (IplImage* image, int step);

        //obtenció del límit del nivell de detall a partir de les dimensions
        int findMaxStep (int a, int b);
                                
        //creació de bandes verticals a una imatge tractada amb RSR + CC
        void createVerticalStripes (subregion* sub, IplImage* image, int width, 
                                    IplImage* debug, int threshold, 
                                    int &numsub);

        //creació de bandes horitzontals a una imatge tractada amb RSR + CC
        void createHorizontalStripes (subregion* sub, IplImage* image, 
                                      int height, IplImage* debug, 
                                      int threshold);
                
        FILE *f;    //fitxer de log
        
        int numlin; //nº de linies trobades (límit superior, perque les ajuntam)
                    //les no vàlides tenen hstripex[i] = -1
        
        //estructures de memòria necessàries per a l'algoritme RSR + CC
        int hstripex[MAX_COORD];
        int hstripey[MAX_COORD];
        int hstripeheight[MAX_COORD];
        
        //intensitat màxima i mínima d'una banda o "stripe"
        unsigned char hstripemaxd[MAX_COORD];
        unsigned char hstripemind[MAX_COORD];
        
        //llistat de subregions
        subregion* subl;
        
        //punter a la configuració
        Config* mConfig;            

        //imatges esquerra i dreta en format RGB (no BGR d'OpenCV)
        IplImage* frameLeftRGB;
        IplImage* frameRightRGB;
};

#endif
