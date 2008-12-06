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

//arxiu ocv.cpp

#include "ocv.h"

#ifdef WINDOWS
videoInput VI; //cream l'objecte VideoInput
int device1;   //qualsevol dispositiu de listDevices
int device2;   //qualsevol dispositiu de listDevices
unsigned char * buffer1;
unsigned char * buffer2;
#endif

#ifdef LINUX
CvCapture* cvdevice1;
CvCapture* cvdevice2;    
#endif

int counter;        //compta el nombre de disparitats calculades
int k;              //enter per a l'espera de tecla (coses d'OpenCV)
bool showdisp;

IplImage* leftGray; 
IplImage* rightGray;

clock_t oldtime;
clock_t newtime;
clock_t elapsed;

clock_t oldtimedisp;
clock_t newtimedisp;
clock_t elapseddisp;
    
int nframes;        //nombre de frames capturats

//constructor
Ocv::Ocv (Config* config)
{
    mConfig = config;
    //inicialitzam amb valors per defecte
    //fitxer de log
   char nombre[10] = "out.txt";
   f = fopen (nombre, "w");
   printf( "Arxiu: %s (per escriptura) -> ", nombre );
   if( f )
      printf( "Arxiu creat (OBERT)\n" );
   else
   {
      printf( "Error (NO OBERT)\n" );
      exit(0);
   }       
    //parametres (valors per defecte)
       
    //Birchfield/Tomasi
    disparam11 = 100;

    disparam12 = 20.0;
    disparam13 = 4.0;
    disparam14 = 12.0;
    disparam15 = 15.0;
    disparam16 = 25.0;

    //finestra + SAD
    disparam21 = 2;
    disparam22 = 2;
    disparam23 = 20;
    disparam24 = 2;
    disparam25 = 30;

    //RSR + CC
    disparam31 = 4;            //wwidth
    disparam32 = 4;            //wheight
    disparam33 = 1;            //threshold
    disparam34 = 10;           //limit
    disparam35 = 30;           //levels
       
    disalg = DA_BIRCHFIELD_TOMASI;

    //nivell de detall
    mImageStep = 2;
    counter = 0;
}

//destructor
Ocv::~Ocv ()
{
    //tancam fitxer de log
    fclose(f);
}

//inicialització de la classe
void Ocv::ocvStart ()
{   
    int size= 0 ;
    int width = 0;
    int height = 0;
    CvSize sizecv;
        
    if (mConfig->getDefMode ()){ //mode REAL

#ifdef WINDOWS
        //Mode Real (Amb càmeres) [WINDOWS]

    	//imprimim una llista de dispositius de captura disponibles
    	int numDevices = VI.listDevices ();	
    
    	VI.setUseCallback (false); //aquesta crida accelera la captura d'imatges
    	
    	device1 = 1;  //qualsevol dispositiu de listDevices
    	device2 = 0;  //qualsevol dispositiu de listDevices
    	
    	//configuram els dispositius
    	
    	VI.setupDevice (device1, mConfig->getDefWidth(), 
                        mConfig->getDefHeight());				 
        VI.setupDevice (device2,
                        mConfig->getDefWidth(), mConfig->getDefHeight());				 
    
    	//asseguram el tamany    
    	width 	= VI.getWidth (device1);
    	height 	= VI.getHeight (device1);
    	size	= VI.getSize (device1);
    	
    	VI.showSettingsWindow (device1);
    	VI.showSettingsWindow (device2);

    	//establim propietats de les imatges a mostrar
    	sizecv.width=width;
    	sizecv.height=height;
         
        frameLeft = cvCreateImage (sizecv, IPL_DEPTH_8U, 3);
        frameRight = cvCreateImage (sizecv, IPL_DEPTH_8U, 3);
        
        //cream els buffers
    	buffer1 = new unsigned char[size];
    	buffer2 = new unsigned char[size];
#endif

#ifdef LINUX
        //Mode Real (Amb càmeres) [LINUX/UNIX]
        cvdevice1 = cvCaptureFromCAM (-1); //primera càmera
        cvdevice2 = cvCaptureFromCAM (-1); //segona càmera

        width  = (int)cvGetCaptureProperty (cvdevice1, CV_CAP_PROP_FRAME_WIDTH);
        height = (int)cvGetCaptureProperty (cvdevice1, CV_CAP_PROP_FRAME_HEIGHT);
        
        //establim propietats de les imatges a mostrar
        sizecv.width=width;
        sizecv.height=height;
        
        frameLeft = cvCreateImage  (sizecv, IPL_DEPTH_8U, 3);
        frameRight = cvCreateImage (sizecv, IPL_DEPTH_8U, 3);
#endif

    }else{          
        //MODE SIMULACIÓ [WINDOWS + LINUX]
        
        //carregam imatges per defecte
        frameLeft = cvLoadImage  ("data/left.png",  CV_LOAD_IMAGE_COLOR);
        frameRight = cvLoadImage ("data/right.png", CV_LOAD_IMAGE_COLOR);

        if ((frameLeft != NULL) && (frameRight != NULL)){
           printf("Imatges carregades OK\n");
        }
        
        //guardam les propietats de les imatges
        width = frameLeft->width;
        height = frameLeft->height;
        size = frameLeft->width * frameLeft ->height;

        //processam imatge esquerra
        //cream el pixbuf
        pixbufright = gdk_pixbuf_new_from_data ((guchar*)frameLeft->imageData,
               GDK_COLORSPACE_RGB, FALSE, 8, 
               frameLeft->width, 
               frameLeft->height, 
               frameLeft->width*3, 
               NULL, 
               NULL);

        //processam imatge dreta
        //cream el pixbuf
        pixbufleft = gdk_pixbuf_new_from_data ((guchar*)frameRight->imageData,
               GDK_COLORSPACE_RGB, FALSE, 8, 
               frameRight->width, 
               frameRight->height, 
               frameRight->width*3, 
               NULL, 
               NULL);
    }

    //creació d'imatges per la disparitat
    //calen dues imatges en escala de grisos
    
    leftGray = cvCreateImage   (cvGetSize(frameLeft),  IPL_DEPTH_8U, 1);
    rightGray = cvCreateImage  (cvGetSize(frameRight), IPL_DEPTH_8U, 1);
    depthImage = cvCreateImage (cvGetSize(frameLeft),  IPL_DEPTH_8U, 1);

    frameLeftRGB = cvCreateImage  (cvGetSize(frameLeft),  IPL_DEPTH_8U, 3);
    frameRightRGB = cvCreateImage (cvGetSize(frameRight), IPL_DEPTH_8U, 3);
    depthImageRGB = cvCreateImage (cvGetSize(frameLeft),  IPL_DEPTH_8U, 3);    

    //rellotge   
    oldtime=clock ();
    newtime=clock ();
    
    k=0;                  //key                 ???
    nframes=0;            //inicialitzam a 0 els frames
    showdisp = false;     //per controlar si s'ensenya la disparitat
    
    //calculam el bot màxim permisible
    mMaxStep = findMaxStep (depthImage->width, depthImage->height);
    //printf("maxtep = %i\n", mMaxStep);
    mImageStep = DEF_IMAGE_STEP;
}

//funció per a la captura d'imatges a partir de les càmeres
void Ocv::ocvCapture (IplImage* left, IplImage* right)
{
#ifdef WINDOWS
    if(VI.isFrameNew (device1)){
    	VI.getPixels (device1, buffer1, false, true);
    	frameLeft->imageData=(char*)buffer1;
        cvCvtColor (frameLeft, left, CV_BGR2RGB);  //convertim imatge a RGB
        
        nframes++;
        
        GdkPixbuf* pixbufleft = gdk_pixbuf_new_from_data (
            (guchar*)left->imageData ,GDK_COLORSPACE_RGB, FALSE, 8, 
            left->width, 
            left->height, 
            left->width*3, 
            NULL, 
            NULL);
    
    }
    
    if(VI.isFrameNew (device2)){
    	VI.getPixels (device2, buffer2, false, true);
    	frameRight->imageData=(char*)buffer2;
        cvCvtColor (frameRight, right, CV_BGR2RGB);  //convertim imatge a RGB
        
        GdkPixbuf* pixbufright = gdk_pixbuf_new_from_data (
            (guchar*)right->imageData ,GDK_COLORSPACE_RGB, FALSE, 8, 
            right->width, 
            right->height, 
            right->width*3, 
            NULL, 
            NULL);
    }
#endif

#ifdef LINUX
    //càmera esquerra
    cvGrabFrame (cvdevice1);
    frameLeft = cvRetrieveFrame (cvdevice1);
    cvCvtColor (frameLeft, left, CV_BGR2RGB);  //convertim imatge
    
    nframes++;
    
    GdkPixbuf* pixbufleft = gdk_pixbuf_new_from_data ((guchar*)left->imageData,
        GDK_COLORSPACE_RGB, FALSE, 8, 
        left->width, 
        left->height, 
        left->width*3, 
        NULL, 
        NULL);
       
    //càmera dreta
    cvGrabFrame (cvdevice2);
    frameRight = cvRetrieveFrame (cvdevice2);
    cvCvtColor (frameRight, right, CV_BGR2RGB);  //convertim imatge
    
    GdkPixbuf* pixbufright = gdk_pixbuf_new_from_data (
        (guchar*)right->imageData, GDK_COLORSPACE_RGB, FALSE, 8, 
        right->width, right->height, right->width*3, NULL, NULL);     
#endif
}     

//bucle principal de processament i captura d'imatges
void Ocv::ocvLoop (bool freeze)
{
    //Pas 1: captura d'imatges
    
    //si no està congelada la imatge, realitzam una nova captura
    if (!freeze){        

        //esborram els antics pixbuf
        if (pixbufleft){
           g_object_unref (pixbufleft);      //l'elimina
        }
        if (pixbufright){
           g_object_unref (pixbufright);     //l'elimina
        }
        
        //si no som al mode simulació, agafam noves imatges
        if (mConfig->getDefMode ()){ //mode REAL amb càmeres
            //MODE REAL
            ocvCapture (frameLeftRGB, frameRightRGB);
        }else{  
            //MODE SIMULACIÓ

            //passam les imatges de BGR a RGB
            cvCvtColor (frameLeft, frameLeftRGB, CV_BGR2RGB);   //convertim a RGB        
            cvCvtColor (frameRight, frameRightRGB, CV_BGR2RGB); //convertim a RGB    
               
            //tornam a crear els pixbuf
            pixbufright = gdk_pixbuf_new_from_data (
                (guchar*)frameLeftRGB->imageData ,GDK_COLORSPACE_RGB, FALSE, 8, 
                frameLeftRGB->width,
                frameLeftRGB->height, 
                frameLeftRGB->width*3, 
                NULL, 
                NULL);

            pixbufleft = gdk_pixbuf_new_from_data (
                (guchar*)frameRightRGB->imageData,
                GDK_COLORSPACE_RGB, FALSE, 8, 
                frameRightRGB->width, 
                frameRightRGB->height, 
                frameRightRGB->width*3, 
                NULL, 
                NULL);
        }
    }

	//actualitzam sempre els frames
    newtime=clock ();
    
    //calculam el temps passat pel càlcul de la disparitat
    elapsed = newtime-oldtime;
    
    //calculam el nombre de segons passats des del darrer ocvloop
    float secs = (float)elapsed/(float)CLOCKS_PER_SEC; 

    //float secs=(float)elapsed/(float)CLOCKS_PER_SEC;
    float secsdisp = (float)elapseddisp/(float)CLOCKS_PER_SEC;
    
    //secs sempre serà major que secsdisp

    //si ha el temps necessari entre diferents càlculs de disparitat
    if (secs>secsdisp + ((float)mConfig->getDefCorrTime()/1000.0)){
        //mostram fps
        //printf("fps: %f\n", (float)nframes/(float)secs); 
        
        //actualitzam els dos temps
        oldtime = newtime;
        
        nframes = 0;          //resetejam el nombre de frames
        showdisp = true;      //mostrarem la disparitat un cop per segon
    }


    //Pas 2: Càlcul de la disparitat
    if(frameLeft!=NULL && showdisp == true){
        //convertim a escala de grisos
        cvCvtColor (frameLeft, leftGray, CV_BGR2GRAY);
        cvCvtColor (frameRight, rightGray, CV_BGR2GRAY);

        //calculam el que es tarda en realitzar la correspondència
        oldtimedisp = clock();
        
        switch(disalg){
            case DA_BIRCHFIELD_TOMASI:{ //crida al primer algoritme de disp.
                cvFindStereoCorrespondence (leftGray, rightGray, 
                     CV_DISPARITY_BIRCHFIELD, depthImage, 
                     disparam11, disparam12, disparam13, 
                     disparam14, disparam15, disparam16);    
                break;
            }
            
            case DA_SAD:{ //crida al segon algoritme de disparitat
                ocvFindStereoCorrespondenceWindow_SAD ( 
                    leftGray, rightGray, depthImage, 
                    disparam21, disparam22, disparam23, disparam24, disparam25);   
                break;
            }
            case DA_RSR:{ //crida al tercer algoritme de disparitat

                ocvFindStereoCorrespondenceRSR_CC (
                    leftGray, rightGray, depthImage, 
                    disparam31, disparam32, disparam33, disparam34, disparam35, 
                    NULL, NULL); 
                break; 
            }
        }
        
        newtimedisp = clock ();
        
        //calculam la diferència
        elapseddisp = newtimedisp-oldtimedisp;

        //descomentar per calcular el temps d'execució
        //fprintf(f, "%i\n", elapseddisp);
        
        //actualitzam el pixbuf
        counter++;    //incrementam el comptador       
        
        //equalitzam l'histograma per incrementar el contrast
        cvEqualizeHist (depthImage, depthImage );

        depthImageRGB = cvCreateImage(cvGetSize(depthImage), IPL_DEPTH_8U, 3);
        cvCvtColor (depthImage, depthImageRGB, CV_GRAY2BGR);
        
        if (pixbufdepth){
           g_object_unref (pixbufdepth);      //l'elimina
        }
        pixbufdepth = gdk_pixbuf_new_from_data(
            (guchar*)depthImageRGB->imageData,
            GDK_COLORSPACE_RGB, FALSE, 8, 
            depthImage->width,
            depthImage->height, 
            depthImage->width*3, 
            NULL, 
            NULL);
        
        showdisp = false;
    }  

    k = cvWaitKey (1);
}

//finalització de la classe Ocv
void Ocv::ocvEnd ()
{   
#ifdef WINDOWS
	//Apagam els dispositius
	VI.stopDevice (device1);
	VI.stopDevice (device2);
#endif
#ifdef LINUX
    cvReleaseCapture (&cvdevice1);
    cvReleaseCapture (&cvdevice2);
#endif
}

//retorna el valor màxim de la intensitat d'una imatge
int Ocv::getMax (IplImage* image, int step)
{
     //el factor step accelera el procés per agafar el màxim
     char val;
     int max  = 0;
     for(int j = 0; j < image->height; j = j + step){
         for(int i= 0; i<image->width; i = i + step){
             val = image->imageData[j*image->width + i];
             if (val>max){
                 max = (unsigned char)val;
             }
         }
     }
     return (int)max;
}

//càlcul del bot màxim que es pot donar dins una imatge
int Ocv::findMaxStep (int a, int b) 
{
    //agafam màxim
    int min = a;
    if (b<a) min = b;

    //anam dividint i acumulam els divisors 
    //(el nombre de cops que podem dividir-lo per 2)    
    int maxstep = 1;
    while((min%2) == 0){
        maxstep = maxstep * 2;
        min = min/2;
    }
    return maxstep;
} 

//obtenció de la textura per posar-la sobre els polígons de la reconstrucció 3D
IplImage* Ocv::getTexture(){
    return frameLeftRGB;
}

bool draw;

//Funcions de Cross Correlation (Correlació creuada)
//--------------------------------------------------

float means[2];
//mean[0] = esquerra
//mean[1] = dreta

//calcula mitjana
float calculateMean (int i, int j, IplImage* image, int k, int l)
{
     //càlcul del valor mig dels píxels de la finestra
     float mean = 0;
     for (int m=i-k; m<=i+k; m++){
          for (int n=j-l; n<=j+l; n++){
              mean = mean + image->imageData[m + n*image->width];
          }
     }     
     //dividim pel nombre de mostres
     mean = mean /(k * l);
     return mean;
}

//covariança
float cov (int i, int j, int d, IplImage* f, int indexf, 
           IplImage* g, int indexg, int k, int l)
{
     float val = 0.0;
     float meanf = calculateMean (i, j, f, k, l);
     means[indexf] = meanf;         //guardam la mitja per reutilitzar-la
     float meang = calculateMean (i + d, j, g, k, l); 
     means[indexg] = meang;         //guardam la mitja per reutilitzar-la
     for (int m = i - k; m <= i + k; m++){
          for (int n = j - l; n <= j + l; n++){
              val = val + (f->imageData[m + n * f->width] - meanf) * 
                          (g->imageData[m + d + n * f->width] - meang);
          }
     }
     return val;
}

//càlcul de la variació d'una finestra de píxels
//i, j son les coordenades
//image és l'imatge que conté la finestra
//k, l son les dimensions de la finestra
float var (int i, int j, IplImage* image, int index, int d, int k, int l)
{
     float val = 0.0;
     float mean = means[index];
     for (int m=i-k; m<=i+k; m++){
          for (int n= j - l; n <= j + l; n++){
              val = val + 
                  pow( (image->imageData[m + d + n * image->width] - mean), 2);
          }
     }
     return val;
}

//calcula la correlació creuada entre dues finestres de píxels
//i, j son les coordenades del centre de la finestra
//d és el desplaçament de la finestra dreta respecte de l'esquerra 
//(positiu = desplaçament cap a la dreta!)
//f, g son les imatges
//k, l son les dimensions de la finestra
float findCrossCorrelation (int i, int j , int d, IplImage* f, int indexf,
                            IplImage* g, int indexg, int k, int l)
{
     //evitam sortir dels rangs correctes
     if ((i+d<0) || (i+d>=g->width) || (i-k<0) || (i+k>f->width) || (j-l<0) || 
         (j+l>=f->height)) {
         return 0;         //retornam un valor no vàlid
     }else{
         return cov(i, j ,d, f, 0, g, 1, k, l) / 
                (0.000000001 + (sqrt(var(i, j, f, 0, 0, k, l)) * 
                sqrt(var(i, j, g, 1, d, k, l))));
     }
}

//Funcions de finestra + SAD
//--------------------------

//ajustam el paràmetre a les dimensions de la finestra
void adjustParameter (int &p, int r)
{
    if(p<0){
        p=0;
    }else{
        if (p>=r){
            p = r-1;      //p.e. amplada = 256 -> 255
        }
    }
}

//implementació del càlcul del valor absolut per un numero en coma flotant
float absf (float f)
{
    if(f<0.0){
        return -f;
    }
    return f;
}

//càlcul del SAD (Sum of Absolute Differences) per a dues finestres de píxels
int findSAD (IplImage* left, IplImage* right, 
             int xl, int yl, int xr, int yr, int w, int h, int d)
{
      //d = desplaçament cap a l'esquerra de la finestra dreta 
      //(elements de la dreta sempre a la mateixa pos o a l'esquerra )
      //es a dir, desplaçam la finestra dreta sobre l'esquerra cap a l'esquerra.
      int sad = 0;
      int vl = 0;
      int vr = 0;
      for(int i = 0; i < w; i++){
          for(int j = 0; j < h; j++){
              if( 
                  (xr - d + i>=0) && 
                  (xr - d + i < right->width) &&               
                  (xl + i     < left->width) && 
                  (j + yr     < right->height) && 
                  (j + yl     < left->height)  
              ){
                  vl = left->imageData[(xl+i) + (j+yl)*left->width];
                  vr = right->imageData[(xr - d + i) + (j+yr)*left->width];
                  sad = sad + abs(vl - vr);
              }else{
                  return (int)INF;  //error (millor retornar un nº gran)
              }
          }
      }
      return sad;
}

//troba la correspondència SAD de dues imatges, donada una finestra
int Ocv::ocvFindStereoCorrespondenceWindow_SAD (IplImage* left, IplImage* right, 
    IplImage* depth, int w, int h, int limit, int detail, int levels)
{
    
    //per a cada linia cercam el SAD de cada píxel de 
    //l'esquerra respecte de la dreta amb un desplaçament
    //levels serà els nivells de gris en els quals volem escalar [0-255]
    //creixent. Finalment agafam el millor candidat
    //no té sentit un limit 0, no permetrer-ho
    //factor d'escalat per mapejar la disparitat dins tot el rang de grisos
    float scale = (float)levels/(float)limit;   
    //antic candidat disparitat (x)
    int oldcd = depth->width; 
    for (int j=0; j<depth->height; j=j+detail){   //per a cada linia
        for(int i=0; i<depth->width; i=i+detail){ //avançam per les columnes
            //anam incrementant el desplaçament
            float sad = INF;      //inicialitzam a infinit el SAD
            float newsad = 0;
            int cd = 0;  //disparitat candidata
            for(int d=0; d<limit; d=d+1){
                //cercam el SAD i ens quedam amb el menor
                //perque funcioni com una finestra hem de modificar i i j
                //hem de posar el cantó adalt esquerra de la finestra
                int newi = i-(int)(w/2.0);    
                int newj = j-(int)(h/2.0);    
                //convé adaptar la finestra verticalment
                adjustParameter (newj, depth->height);
                newsad = findSAD (left, right, newi, newj, newi, newj, w, h, d);
                if (newsad < sad){
                   sad = newsad;
                   //també guardam coordenades del candidat
                   //sabem que el punt de l'esquerra és (i, j)
                   //el punt de la dreta serà (i-d, j);
                   cd = d;   //desplaçament del candidat
                }
            }
            //ara tenim el SAD mínim
            //aleshores sabem la disparitat, que és d. La guardam dins depth
            int mini = 0;
            int maxi = 0;
            int minj = 0;
            int maxj = 0;
            
            mini = i - detail;
            if (mini < 0) mini = 0;
            
            maxi = i + detail;
            if (maxi > depth->width) maxi = depth->width;

            minj = j - detail;
            if (minj < 0) minj = 0;
            
            maxj = j + detail;
            if (maxj > depth->height) maxj = depth->height;
            for (int l=minj; l<maxj; l++){
                for (int k=mini; k<maxi; k++){
                    depth->imageData[k + l*depth->width] = 
                        (unsigned char)((float)cd * scale);
                    //guardam el guanyador, serà el limit per a la nova cerca
                    oldcd=cd;        
                }
            }
        }
    }
}

//Funcions de Rectangular Subregioning + CC
//-----------------------------------------

//troba el màxim i el mínim valor de disparitat dins una regió
void findMaxMinDisp (IplImage* image, unsigned char &max, unsigned char &min, 
                     bool calcsum, int &sum, int x, int y, 
                     int width, int height)
{

    min = image->imageData[x+image->width*y];
    max = image->imageData[x+image->width*y];

    sum = 0;
    unsigned char val = 0;
    for(int i=x; i<x+width; i++){
        for(int j=y; j<y+height; j++){
            val = image->imageData[i+image->width*j];
            //actualitzam mínim i màxim
            if(calcsum == true){
                sum = sum + val;
            }
            if (val>max){
                //marcam màxim
                max = val;
            }else{
                if (val<min){ 
                    //marcam mínim
                    min = val;
                }
            }
        }
    }
}

//divideix verticalment l'imatge, i ajunta horitzontalment subregions simil·lars
void Ocv::createVerticalStripes (subregion* sub, IplImage* image, int width, 
                                 IplImage* debug, int threshold, int &numsub)
{
     //problema: ho tenim dividit en z linies horitzontals
     //ara cada linia horitzontal es dividirà en image->width/width segments
     int dwidth = 0;
     int s = 0;   //subregió actual (índex de l'array)
     //tractam cada linia horitzontal
     for(int k=0; k<numlin; k++){
         //cream les subregions per a aquesta linia k
         dwidth = 0;
         if (hstripex[k]!= -1){ //si és vàlida
             //cream tantes sublinies com faci falta
             for (int l=0; l<image->width; l=l+width){
                 sub[s].y = hstripey[k]; //mateixa fila per a totes les subreg.
                 sub[s].x = l;           //l avança dins l'amplada
                 if (l + width > image->width){
                     sub[s].width = image->width - l;
                 }else{
                     sub[s].width = width;
                 }
                            
                 sub[s].height = hstripeheight[k]; //alçada de la linia
                 //només aquí ens cal calcular la suma
                 findMaxMinDisp (image, sub[s].maxd, sub[s].mind,
                     true, sub[s].sum, 
                     sub[s].x, sub[s].y, sub[s].width, sub[s].height);
                 
                 dwidth  = dwidth  + sub[s].width;
                 
                 s++;    //avançam de subregió
             }
         }
     }

     //ajuntam subregions amb la mateixa disparitat (horitzontalment)
     //z és el número de linies horitzontals
     //inicialitzam

     int s2=s;     //nou nombre de stripes
     int now = 0;
     int next = 1;
     
     for(int b=0; b<s; b++){  //tractam 2 stripes cada pic
        //si coincideixen min i max disp, les juntam!
        if (
           ( abs( (int)sub[now].mind - (int)sub[next].mind )<=threshold ) &&
           ( abs( (int)sub[now].maxd - (int)sub[next].maxd )<=threshold ) &&
           (sub[now].y == sub[next].y)  //perque les ajuntam horitzontalment
           ){
           //es junten
           sub[now].width = sub[now].width + sub[next].width;
           //eliminam la "next"
           sub[next].x = -1;   //indicam que ja no la necessitam
           next = next + 1;    //botam a la seguent next
           s2 = s2-1;          //nombre de subregions es decrementa
           //no faria falta ajustar el maxd i mind, perque son els mateixos
        }else{
            //aleshores son dues stripes diferents
            now = next;
            next = next+1;
        }
     }

     //guardam s, nombre total de subregions de la imatge
     numsub = s;
     
     if(draw==false && debug != NULL){
         for(int l=0; l<s; l++){      //per a cada subregió
             if(sub[l].x!=-1){        //si és vàlida
                 //pintam linia (limit dret)
                 for(int w=0; w<sub[l].width; w++){
                     debug->imageData[sub[l].y*debug->width+(sub[l].x+w)]=255;
                 }
                 //pintam linia(limit superior)
                 for(int h=0; h<sub[l].height; h++){
                     debug->imageData[(sub[l].y+h)*debug->width+sub[l].x]=255;         
                 }
             }
         }
     }     
}

//crea les subregions horitzontals i les ajunta
//threshold  = nivells d'intensitat que es toleren per a juntar les subregions
void Ocv::createHorizontalStripes (subregion* sub, IplImage* image, int height, 
                                   IplImage* debug, int threshold)
{
     int z=0;
     int theight = 0;
     int sum;    //no usat, però manté al compilador content
     for(int k=0; k<image->height; k=k+height){
         hstripex[z] = 0;
         hstripey[z] = k;
         if(k+height>=image->height){             //si a la pròxima ens sortim
             findMaxMinDisp (image, hstripemaxd[z], hstripemind[z], 
                 false, sum, hstripex[z], hstripey[z], 
                 image->width, image->height-(z*height));
                 hstripeheight[z] = image->height-(z*height);
         }else{
             findMaxMinDisp (image, hstripemaxd[z], hstripemind[z], 
                 false, sum, hstripex[z], hstripey[z], image->width, height);
             hstripeheight[z] = height;    //posam la altura de la linia
                                           //a la darrera linia no serà així
         }
         theight = theight + hstripeheight[z];    //acumulam altures
         z++;
     }    
     
     //ara hem d'ajuntar les linies horitzontals!
     //z és el número de linies horitzontals
     //inicialitzam
     int z2=z;     //nou nombre de stripes
     int now = 0;
     int next = 1;
     for(int h=0; h<z; h=h+1){  //tractam 2 stripes cada pic
        //si coincideixen min i max disp, les ajuntam
        if (
           (abs( (int)hstripemind[now] - (int)hstripemind[next] )<=threshold) &&
           (abs( (int)hstripemaxd[now] - (int)hstripemaxd[next] )<=threshold)
           ){
           //simplement incrementam el tamany de la 1a i eliminam la 2a 
           //(valor negatiu)
           hstripeheight[now] = hstripeheight[now] + hstripeheight[next];           
           //eliminam la "next"
           hstripex[next] = -1; //indica que no la necessitam
           next = next + 1;     //botam a la seguent next
           z2 = z2-1;
        }else{
            //aleshores son dues stripes diferents
            now = next;
            next = next+1;
        }
     }
     //guardam z a la variable global
     numlin = z;      
}

//important comprovar el rang de les finestres (pixels) per evitar errors
void Ocv::ocvFindStereoCorrespondenceRSR_CC (IplImage* left, IplImage* right, 
     IplImage* depth, int wwidth, int wheight, int threshold, int limit, 
     int levels, IplImage* debug1, IplImage* debug2)
{

     int size = (1+(depth->width / wwidth)) * (1+(depth->height / wheight));

     //factor d'escalat per mapejar la disparitat dins tot el rang de grisos
     float scale = (float)levels/(float)limit;
    
     subl = new subregion[size];
     
     int numsubl;               //nombre de subregions imatge esquerra

     //cream bandes horitzontals     
     createHorizontalStripes (subl, left, wheight, debug1, threshold);

     //cream bandes verticals (subdivisió final)
     createVerticalStripes (subl, left, wwidth, debug1, threshold, numsubl);

     float cc = -1;
     //processam cada subregió
     for(int l=0; l<numsubl; l++){
         if(subl[l].x!=-1){
             float wincc = -1;
             float windisp = 0;   //disparitat (desplaçament)  
             for(int d = -limit; d <= 0; d++){ 
                  //d és el desplaçament de la finestra dreta, 
                  //negatiu = a l'esquerra
                  cc = findCrossCorrelation (subl[l].x, subl[l].y, d, left, 0, 
                           right, 1, subl[l].width, subl[l].height);
                  if (cc > wincc ){
                      //actualitzam el SAD guanyador (el menor)          
                      wincc = cc ;
                      //guardam el desplaçament candidat 
                      //(disparitat per aquell bloc esquerre)  
                      windisp = (float)-d;    
                  }
             }
             //pintam el mapa de disparitat
             for(int h=0; h<subl[l].height; h++){    //pintam linia (limit sup.)
                 for(int w=0; w<subl[l].width; w++){ //pintam linia (limit dret)
                     if(windisp<=255){               //consideram 255 un error
                         //quan més separats més disparitat
                         depth->imageData
                             [(subl[l].y+h)*depth->width+subl[l].x+w] =
                             (unsigned char) (windisp * scale);         
                     }else{
                         depth->imageData
                             [(subl[l].y+h)*depth->width+subl[l].x+w] = 
                             255;
                     }
                 }
             }
        }
     }
     //alliberam memoria
     delete[] subl;
}

//obtenció del nivell de disparitat d'un cert píxel
float Ocv::getDepth (int x, int y)
{
    int x2 = x;
    int y2 = y;
    if(x>= depthImageRGB->width){
        //retornam el darrer valor vàlid
        x2 = depthImageRGB->width-1;
    }
    if(y>=depthImageRGB->height){
        //retornam el darrer valor vàlid
        y2 = depthImageRGB->height-1;
    }
    //cas per defecte
    //multiplicam per 3 ja que agafam les dades de la imatge RGB (3 canals)
    unsigned char num = 
        depthImageRGB->imageData[3*(x2 + (y2 * depthImageRGB->width))];
    return (float)num;
}



