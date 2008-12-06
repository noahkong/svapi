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

//arxiu config.cpp

#include "config.h"

//constructor
Config::Config ()
{
    mDefWidth = 0;
    mDefHeight = 0;

    mDefRefPointU = 0;
    mDefRefPointV = 0;
    
    mDefGridScale = 0;
    mDefGridMax   = 0;

    mDefRot.x = 0.0;
    mDefRot.y = 0.0;
    mDefRot.z = 0.0;

    mDefTr.x = 0.0;
    mDefTr.y = 0.0;
    mDefTr.z = 0.0;

    mDefFocalDist = 0.0;

    readfile();
}

//destructor
Config::~Config ()
{
}

//lectura de l'arxiu de configuraci�
void Config::readfile ()
{
    string line;
    ifstream myfile (DEF_CONFIG_FILE);
    if (myfile.is_open ()){
        while (!myfile.eof () ){
            getline (myfile,line);
            cout << line << endl;
            if (line.length ()>0){
                processLine (line);
            }
        }
        myfile.close ();
    }else{
        cout << "Arxiu de configuraci� no trobat" << endl; 
    }
}

//processament d'una linia
void Config::processLine (string l)
{
     string part1, part2;
     size_t found, comment;
     found=l.find ("=");
     if (found!=string::npos){         //si s'ha trobat
         part1 = l.substr (0, found);  //primera part
         part1.erase (part1.find_last_not_of (" \t\n")+1);
         part1 = part1.substr (part1.find_first_not_of (" \t\n"));
         
         part2 = l.substr (found+1, l.length ());   //fins al final    
         comment = part2.find ("#");
         if (comment!=string::npos){
             part2.erase (comment);    //esborram comentaris
         }
         part2.erase (part2.find_last_not_of (" \t\n")+1);
         part2 = part2.substr (part2.find_first_not_of (" \t\n"));
         
         setVar (part1, part2);             
     }
}

//assignaci� de valor a una variable de la configuraci�
int Config::setVar (string var, string val)
{
     std::istringstream i(val); //ara podem convertir al tipus que volguem
     if (var == "DEF_MODE"){
         i >> mDefMode;
         return 0;
     }               
     if (var == "DEF_WIDTH"){
         i >> mDefWidth;
         mDefRefPointU = mDefWidth/2;
         return 0;         
     }
     if (var == "DEF_HEIGHT"){
         i >> mDefHeight;
         mDefRefPointV = mDefHeight/2;
         return 0;
     }
     if (var == "DEF_GRID_SCALE"){
         i >> mDefGridScale;
         return 0;
     }     
     if (var == "DEF_GRID_MAX"){
         i >> mDefGridMax;
         return 0;
     }     
     if (var == "DEF_ROT_X"){
         i >> mDefRot.x;
         return 0;
     }          
     if (var == "DEF_ROT_Y"){
         i >> mDefRot.y;
         return 0;
     }          
     if (var == "DEF_ROT_Z"){
         i >> mDefRot.z;
         return 0;
     }          
     if (var == "DEF_TR_X"){
         i >> mDefTr.x;
         return 0;
     }          
     if (var == "DEF_TR_Y"){
         i >> mDefTr.y;
         return 0;
     }          
     if (var == "DEF_TR_Z"){
         i >> mDefTr.z;
         return 0;
     }          
     if (var == "DEF_FOCAL_DIST"){
         i >> mDefFocalDist;
         return 0;
     }     
     if (var == "DEF_CORR_TIME"){
         i >> mDefCorrTime;
         return 0;
     }     
     if (var == "DEF_TIMEOUT_3D"){
         i >> mDefTimeout3D;
         return 0;
     }          
     //sin�, retornam un error
     return -1;
}

//obtenci� del mode (1 = C�meres, 0 = Simulaci� amb imatges)
bool Config::getDefMode ()
{
    return (bool)mDefMode;
}

//obtenci� de l'mplada
int Config::getDefWidth ()
{
    return mDefWidth;
}

//obtenci� de l'al�ada
int Config::getDefHeight ()
{
    return mDefHeight;
}

//obtenci� de la coordenada horitzontal del punt de refer�ncia
int Config::getDefRefPointU ()
{
    return mDefRefPointU;
}

//obtenci� de la coordenada vertical del punt de refer�ncia
int Config::getDefRefPointV ()
{
    return mDefRefPointV;
}

//obtenci� del valor m�xim de l'al�ada de la malla 3D
float Config::getDefGridMax ()
{
    return mDefGridMax;
}

//obtenci� del valor d'escalat de la malla 3D
float Config::getDefGridScale ()
{
    return mDefGridScale;
}

//obtenci� de la rotaci�
t_point3d Config::getDefRotation () {
    return mDefRot;
}

//obtenci� de la translaci�
t_point3d Config::getDefTranslation () {
    return mDefTr;
}

//obtenci� de la dist�ncia focal
float Config::getDefFocalDist (){
    return mDefFocalDist;
}

//obtenci� del temps entre correspond�ncies
int Config::getDefCorrTime(){
    return mDefCorrTime;
}

//obtenci� del temps entre reconstruccions 3D
int Config::getDefTimeout3D(){
    return mDefTimeout3D;
}

