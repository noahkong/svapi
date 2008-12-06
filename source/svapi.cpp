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

//arxiu svapi.cpp

#include "svapi.h"

//constructor de classe
svLib::svLib (Config* config)
{    
    //dins config tendrem els 
    mConfig = config;
    
    //cream matrius
	A = cvCreateMat (4, 4, CV_64F);
	X = cvCreateMat (4, 1, CV_64F);

	E  = cvCreateMat (3, 3, CV_64F);
	K  = cvCreateMat (3, 3, CV_64F);
	F  = cvCreateMat (3, 3, CV_64F);
	PL = cvCreateMat  (3, 4, CV_64F);
	PR = cvCreateMat  (3, 4, CV_64F);

	mProjOk = false;

    //posam valors per defecte
	mUref = config->getDefRefPointU ();
	mVref = config->getDefRefPointV ();

	mScaleU = DEF_SF_U;
	mScaleV = DEF_SF_V;
	mTrans[0] = 1.0;
	mTrans[1] = 0.0;
	mTrans[2] = 0.0;

	mRot[0] = 0.0;
	mRot[1] = 0.0;
	mRot[2] = 0.0;
}

// destructor de classe
svLib::~svLib()
{
	cvReleaseMat (&A);
	cvReleaseMat (&X);
	if (K)   cvReleaseMat (&K);
	if (E)   cvReleaseMat (&E);
	if (PL)  cvReleaseMat (&PL);
	if (PR)  cvReleaseMat (&PR);
}

//inicialitza la classe amb els valors de la configuració
void svLib::init ()
{
    //agafam angle de la configuració
    t_point3d angle = mConfig->getDefRotation ();
    setRotation (angle.x, angle.y, angle.z); //0, 0, 0 per a un sistema epipolar
	
	//posam el vector, traslació entre càmeres
	t_point3d tr = mConfig->getDefTranslation ();
	setTranslation (tr.x, tr.y, tr.z);

    //posam factor d'escalat (píxels per metre = distància focal)	
	double scale = mConfig->getDefFocalDist ();
	setScale (scale, scale);             //dependrà de la distància focal
	
    f = scale;   //distancia focal
    b = tr.x;    //distància entre càmeres
    setReferencePoint (mConfig->getDefRefPointU(), mConfig->getDefRefPointV());
}    

//Triangulació i projecció senzilla (sense matrius)
//-------------------------------------------------

//transformació de coordenades del sistema a coordenades del món (2D a 3D)
//mètode simple
void svLib::stereoToWorld (CvPoint3D32f& p3d, 
                           CvPoint2D32f& ptL, CvPoint2D32f& ptR)
{     
    //important! primer referenciar els píxels respecte del punt de referència
    double xl2 = ptL.x-mUref;
    double xr2 = ptR.x-mUref;
    double y2  = ptL.y-mVref;

    //calculam coordenades del mon 
    //si no té sentit una profunditat negativa
    //p3d.z = fabs((b * f) / (xl2 - xr2));   
    p3d.z = ((b * f) / (xl2 - xr2));
    p3d.x = xl2 * p3d.z / f;
    p3d.y = y2 * p3d.z / f;
}


//pas de 3D a 2D (projecció) pel mètode senzill
void svLib::worldToStereo (CvPoint3D32f& p3d, 
                           CvPoint2D32f& ptL, CvPoint2D32f& ptR)
{   
    double xl2;
    double xr2;
    double y2;

    y2 = p3d.y / (p3d.z / f);
    xl2 = p3d.x / (p3d.z / f);
    xr2 = xl2 - ((b * f) / p3d.z);

    //tornam a referenciar respecte del centre de la imatge
    xl2 = xl2 + mUref;
    xr2 = xr2 + mUref;
    y2  = y2  + mVref;
    
    ptL.x = xl2;
    ptL.y = y2;
    ptR.x = xr2;
    ptR.y = y2;
}

//Triangulació i projecció complexa (Hartley i matrius de projecció)
//------------------------------------------------------------------

//Càlcul de la matriu de rotació a partir dels ànguls donats en graus
void svLib::calcRotationMatrix (CvMat *R, double a_x, double a_y, double a_z)
{
    //cream matriu temporal
	CvMat *temp = cvCreateMat (3, 3, CV_64F);     //R ha de ser 3x3
	
    // construcció del vector i creació de les 3 matrius de rotació
	CvMat *rot[3];
	for( int i=0; i<3; i++ )
	{
		rot[i] = cvCreateMat (3, 3, CV_64F); // cream les matrius per x, y i z
		cvSetIdentity (rot[i]);              // inicialitzam matriu
	}

    //construim les matrius de rotació
	double co, si; // cos i sin de l'angle

	co = cos (a_x * DEG2RAD);        //*Pitch (eix X):
	si = sin (a_x * DEG2RAD);        
	cvmSet (rot[0] , 1, 1 ,  co);    //     | 1     0       0    |
	cvmSet (rot[0] , 1, 2 ,  si);    //Rx = | 0   cos(x)  sin(x) |
	cvmSet (rot[0] , 2, 1 , -si);    //     | 0  -sin(x)  cos(x) |
	cvmSet (rot[0] , 2, 2 ,  co);    //

	co = cos (a_y * DEG2RAD);        //*Yaw (eix Y):
	si = sin (a_y * DEG2RAD);    
	cvmSet (rot[1] , 0, 0 ,  co);    //     | cos(y)  0  -sin(y) |
	cvmSet (rot[1] , 0, 2 , -si);    //Ry = |   0     1    0     |
	cvmSet (rot[1] , 2, 0 ,  si);    //     | sin(y)  0  cos(y)  |
	cvmSet (rot[1] , 2, 2 ,  co);    //


	co = cos (a_z * DEG2RAD);        //*Roll (eix Z):
	si = sin (a_z * DEG2RAD);    
	cvmSet (rot[2] , 0, 0 ,  co);    //     |  cos(z)  sin(z)   0 |
	cvmSet (rot[2] , 0, 1 ,  si);    //Rz = | -sin(z)  cos(z)   0 |
	cvmSet (rot[2] , 1, 0 , -si);    //     |    0       0      1 |    
	cvmSet (rot[2] , 1, 1 ,  co);    //

    //multiplicam les matrius
	cvMatMul (rot[0], rot[1], temp);
	cvMatMul (temp, rot[2], R);
	
	//alliberam les matrius temporals
	for (int i=0; i<3; i++)
		cvReleaseMat (&rot[i]);
	cvReleaseMat (&temp);
}

//posam els angles entre càmeres, geometria RT (angles en graus!)
void svLib::setRotation (double ax, double ay, double az)
{
	mProjOk = false;
	
	mRot[0] = ax;
	mRot[1] = ay;
	mRot[2] = az;
}

//estableix traslació necessària d'una càmara a una altra
void svLib::setTranslation (double vx, double vy, double vz)
{
	mProjOk = false;

	mTrans[0] = vx;
	mTrans[1] = vy;
	mTrans[2] = vz;
}

//posa factor d'escala o Alfa, que indica la distància focal
//un punt de la imatge té les seves coordenades de pixel descrites per:
//fórmula : u=alfa*x/y per a l'eix horitzontal.
void svLib::setScale (double alpha_u, double alpha_v)
{		
	mProjOk = false;

	mScaleU = alpha_u;
	mScaleV = alpha_v;
}

//càlcul de la matriu fonamental a partir de la matriu d'intrinsecs, 
//rotacio i trans.
//E (Matriu essencial) per defecte és nul·la (compte)
void svLib::calcF (CvMat *A, CvMat *Rot, double *vector, CvMat *F, CvMat *E)
{
	bool EWasCreatedHere = false;  //indicam si s'ha creat E
	if( E == NULL )
	{
		E = cvCreateMat (3, 3, CV_64F);
		EWasCreatedHere = true;
	}
	//crea una matriu E un sol cop

    //matriu temporal
	CvMat *temp = cvCreateMat (3, 3, CV_64F);

    //matriu AI
	CvMat *AI = cvCreateMat (3, 3, CV_64F);

    //invertim A i la posam dins AI (A invertida)
	cvInvert (A, AI);
	
    //cream un altre matriu mat_vec
	CvMat *mat_vec = cvCreateMat (3, 3, CV_64F);
	
    //cream la antisimètrica amb els vectors (translació)
    setAntiSymetricMatrix (mat_vec, vector[0], vector[1], vector[2]);
	
	//calcul de E a partir del vector i de la rotació

    //multiplicam la matriu de translació amb ROT i la posam a E  (mirar)
	cvMatMul (mat_vec, Rot, E);

    //calcul de F a partir de E	
    //multiplicació generalitzada de matrius
	cvGEMM (AI, E, 1.0, NULL, 0.0, temp, CV_GEMM_A_T);
	
	//multiplica la matriu temp amb AI i la posa a F. Així obtenim la F final
	cvMatMul (temp, AI, F);

    //alliberam les matrius temporals
	cvReleaseMat (&temp);
	cvReleaseMat (&AI);

    //si E s'ha creat l'alliberam
	if (EWasCreatedHere)
		cvReleaseMat (&E);
}

//crea la matriu mat, com a antisimètrica corresponent al vector src = (x, y, z)
//la matriu antisimètrica d'un vector (x, y, z) és:
// |   0   v_y  -v_z |
// |  v_z   0   -v_x |
// | -v_y  v_x    0  |
void svLib::setAntiSymetricMatrix (CvMat *mat, double x, double y, double z)
{
	cvZero( mat );
	cvmSet( mat, 2, 1,  x );
	cvmSet( mat, 1, 2, -x );
	cvmSet( mat, 0, 2,  y );
	cvmSet( mat, 2, 0, -y );
	cvmSet( mat, 1, 0,  z );
	cvmSet( mat, 0, 1, -z );
}

//Reconstrueix un punt 3D a partir de dues projeccions 2D del mateix punt. 
//Retorna un booleà que indica si s'ha pogut reconstruir correctament
bool svLib::stereoToWorldH (CvPoint3D32f& p3d, 
                            CvPoint2D32f& ptL, CvPoint2D32f& ptR)
{
	bool ok = true;
	
	if (mProjOk == false){
        //Computa les 2 matrius de projecció (L i R) 
        //en cas de no haver estat computades
		calc ();    
    }
    
    //reconstrucció 3D homogenia
    p3d = reconstructTo3DH (PR, ptL.x, ptL.y, ptR.x, ptR.y); 

    //en cas de z negatiu podriem retornar un ok fals
	return ok;
}

//reconstrueix un punt 3D a partir de la matriu de projecció dreta 
//i les coordenades 2D mitjançant el mètode homogeni de Hartley, 
//descrit al seu llibre, secció 12.2, plana 313
CvPoint3D32f svLib::reconstructTo3DH (const CvMat *RightPM, 
    double xL, double yL, double xR, double yR) 
{
	CvPoint3D32f M;

    //omplim la matriu A
	for (int i=0; i<4; i++)
	{
		cvmSet (A, 0, i, xL * cvmGet (PL, 2, i) - cvmGet (PL, 0, i));
		cvmSet (A, 1, i, yL * cvmGet (PL, 2, i) - cvmGet (PL, 1, i));
		cvmSet (A, 2, i, xR * cvmGet (RightPM, 2, i) - cvmGet (RightPM, 0, i ));
		cvmSet (A, 3, i, yR * cvmGet (RightPM, 2, i) - cvmGet (RightPM, 1, i ));
	}

    //resolem A · X = 0
	solve ();

    //agafam el valor v (escalat)
	double v = cvmGet (X, 3, 0);
    
    //agafam components de la matriu. X = 4x1 vector, 
    //fet servir per a la reconstrucció (una CvMat)
    M.x = (cvmGet (X, 0, 0) / v);
	M.y = (cvmGet (X, 1, 0) / v);
	M.z = (cvmGet (X, 2, 0) / v);

	return M;
}

//Resol la equació matricial A M = 0 en un sentit "least square"
//retorna el mòdul del vector A.M computat després de trobar una solució
//Llibre de Hartley , p.592
double svLib::solve ()
{
	int rows = 4;
	int cols = 4;

	CvMat* W = cvCreateMat( rows, cols, CV_64F );
	CvMat* U = cvCreateMat( rows, cols, CV_64F );
	CvMat* V = cvCreateMat( rows, cols, CV_64F );

	cvSVD (A, W, U, V);
	
    //agafam darrera columna de V
	for( int i=0; i<rows; i++ )
		cvmSet (X, i, 0, cvmGet (V, i, cols-1 ));

	cvReleaseMat (&W);
	cvReleaseMat (&U);
	cvReleaseMat (&V);

    //verificam el mòdul del producte A.M
	CvMat* ShouldBeNullVector = cvCreateMat (rows, 1, CV_64F);
	cvMatMul (A, X, ShouldBeNullVector);
	double v, sum=0.0;
	for(int i=0; i<rows; i++)
	{
		v = cvmGet( ShouldBeNullVector, i, 0 );
		sum += v * v;
	}

	return sqrt (sum);    //quadrat de la suma = mòdul (norma)
}

//computa les dues matrius de projecció esquerra i dreta
void svLib::calc ()
{
    //Primer : Matriu de paràmetres intrínsecs
    if(mRefPointOk == true){
    	cvSetIdentity (K);
    	cvmSet (K, 0, 2, mUref);
    	cvmSet (K, 1, 2, mVref);
    
    	cvmSet (K, 0, 0, mScaleU);
    	cvmSet (K, 1, 1, mScaleV);
    	
        //Segon: Matriu de projecció esquerra
    	cvSetIdentity (PL);
    	cvMatMul (K, PL, PL);
    
        //calculam la matriu de projecció dreta a partir de la rotació 
        //i la translació
    	calcFromRT ();
    
        //marcam computat
    	mProjOk = true;
     }
}

//Calcula la matriu de projecció dreta 
//a partir d'una rotació i translació donades
void svLib::calcFromRT ()
{
    //cream matriu de rotació
	CvMat* RotM = cvCreateMat (3, 3, CV_64F);
    
    //invertim la rotació (és la de la càmera esquerra respecte de la dreta)
	calcRotationMatrix (RotM, -mRot[0], -mRot[1], -mRot[2]); 

    //calculam matriu fonamental
	calcF (K, RotM, mTrans, F);

    //inicialitzam matriu de projecció dreta
	cvSetIdentity (PR);
	
	//per cada linia
	for (int li=0; li<3; li++) // linies
	{
		//cvmSet (PR, li, 3, trans[li]);     //abans
        //copiam el vector de translació (de l'esquerra repecte de la dreta!!)
		cvmSet (PR, li, 3, -mTrans[li]);     
        //copiam els valors de les 3 primeres files i columnes
		for (int col=0; col<3; col++)        
        {		
			cvmSet (PR, li, col, cvmGet (RotM, li, col));
        }
	}

	cvMatMul (K, PR, PR);

	cvReleaseMat (&RotM);
}

//posa el punt de referencia (pel calcul de la mat. par. intrinsecs)
//normalment es posa el punt mig de la imatge. 
void svLib::setReferencePoint (float u, float v)
{
	mProjOk = false;
    mRefPointOk = true;
	mUref = u;
	mVref = v;
}

//Projecció d'un punt 3D per P (matriu de projecció) dins d'un punt 2D
//matriu de projecció P : CvMat 4x3
void svLib::projectTo2D (CvPoint3D32f& src, CvMat *P, CvPoint2D32f& point2d)
{
	CvMat *p2d = cvCreateMat (3, 1, CV_64F);
	
	//1. projecció amb les dues matrius
	//cream matriu mat amb el punt 3D per multiplicar-lo
    CvMat* mat = cvCreateMat (4, 1, CV_64F);
	cvmSet (mat, 0, 0, src.x);
	cvmSet (mat, 1, 0, src.y);
	cvmSet (mat, 2, 0, fabs(src.z));  //z negatiu no té sentit al nostre sistema
	cvmSet (mat, 3, 0, 1.0);
	
    //multiplicam matrius (P per mat)
	cvMatMul (P, mat, p2d);
	cvReleaseMat (&mat);              //alliberam mat (matriu del punt 3D)
	
	//2. recuperam coordenades
	point2d.x = ((float)(cvmGet (p2d, 0, 0) / cvmGet (p2d, 2, 0 )));
	point2d.y = ((float)(cvmGet (p2d, 1, 0) / cvmGet (p2d, 2, 0 )));
	cvReleaseMat (&p2d);              //alliberam la matriu amb el punt 2D
}

//projecta el punt 3D donat src als punts esquerre i dret (2D)
void svLib::worldToStereoH (CvPoint3D32f& src, 
                            CvPoint2D32f& ptL, CvPoint2D32f& ptR)
{
	if(mProjOk == false )
    	calc (); //calculam les matrius de projecció
    projectTo2D (src, PL, ptL);  //projectam punt 3D a 2D, esquerre
   	projectTo2D (src, PR, ptR);  //projectam punt 3D a 2D, dret
}
