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

//arxiu config.h

#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

//indicam sistema operatiu (Windows o Linux/Unix)
#define WINDOWS 1
//#define LINUX 1

//arxiu de configuraci� per defecte
#define DEF_CONFIG_FILE "config.ini"

typedef struct{
    float x;
    float y;
    float z;
}t_point3d;

using namespace std; 

//Classe Config
//classe dedicada a la c�rrega de par�metres de configuraci� des d'un arxiu de 
//configuraci�
class Config{
    public:
        Config  ();
        ~Config ();

        //lectura de l'arxiu de configuraci�
        void readfile ();
        
        //processament d'una linia
        void processLine (string l);

        //assignaci� de valor a una variable de la configuraci�
        int setVar (string var, string val);
        
        //m�todes per a l'obtenci� dels valors de les variables de configuraci� 
        int   getDefWidth  ();
        int   getDefHeight ();
        
        int   getDefRefPointU ();
        int   getDefRefPointV ();
        
        float getDefGridMax   ();
        float getDefGridScale ();

        t_point3d getDefRotation ();
        t_point3d getDefTranslation ();
        
        bool  getDefMode();        
        float getDefFocalDist ();
        int   getDefCorrTime();
        int   getDefTimeout3D();

    private:
            
        //variables de la configuraci�
    
        int mDefMode;
        
        int mDefWidth;
        int mDefHeight;

        int mDefRefPointU;
        int mDefRefPointV;
        
        float mDefGridScale;
        float mDefGridMax;
        
        t_point3d mDefRot;
        t_point3d mDefTr;
        float mDefFocalDist;
        
        int mDefCorrTime;      //en ms
        int mDefTimeout3D;     //en ms
        
};

#endif
