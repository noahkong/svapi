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

//arxiu gui.h

#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>
#include "ocv.h"
#include "svapi.h"

//modes existents de visualització
#define NUM_MODES 4             
typedef enum {MDE_POINTS, MDE_WIREFRAME, MDE_POLYGON, MDE_POLYTEX}t_mode_3d;

//Classe Gui
//dedicada al control de la interfícide d'usuari Gtk+

class Gui{
    public:
        Gui  (Ocv* ocv, svLib* sv);
        ~Gui ();
    
        //esdeveniment de configuració
        gboolean configure_event_drawarea (GtkWidget         *widget,
                                           GdkEventConfigure *event,
                                           gpointer          data);
    
        //esdeveniment d'exposició: redibuixa
        gboolean expose_event_drawarea    (GtkWidget      *widget,
                                           GdkEventExpose *event,
                                           gpointer       data);
          
        //dibuixat de les línies allà on es fa click, per les imatges dreta i 
        //esquerra
        void draw_brush (GtkWidget *widget, gdouble x, gdouble y, 
                         gpointer  data);
        
        //dibuixa les linies verticals i horitzontals 
        void draw_brush_lines (GtkWidget *widget, gdouble x, gdouble y,
                               gpointer  data);
    
        //callback per a l'apretat de botó
        gboolean button_press_event (GtkWidget*      widget, 
                                     GdkEventButton* event,
                                     gpointer        data);
    
        //controla el moviment del ratolí sobre les imatges, i actualitza 
        //les linies si cal
        gboolean motion_notify_event (GtkWidget* widget,
                                           GdkEventMotion *event,
                                           gpointer data );
                                           
        //callback dels ajustos, farem servir el paràmetre per sebre 
        //de quin es tracta
        void adj_callback (GtkAdjustment* get, gpointer data);
    
        //funció d'inactivitat de la interficie
        gboolean idle (gpointer data, Ocv* ocv);
    
        //oculta els widgets (scales) del primer algoritme
        void hide_widgets_alg1 ();
        
        //oculta els widgets (scales) del segpm algoritme
        void hide_widgets_alg2 ();
        
        //oculta els widgets (scales) del tercer algoritme
        void hide_widgets_alg3 ();
        
        //mostra els widgets (scales) del primer algoritme
        void show_widgets_alg1 ();
        
        //mostra els widgets (scales) del segon algoritme
        void show_widgets_alg2 ();
        
        //mostra els widgets (scales) del tercer algoritme
        void show_widgets_alg3 ();
    
        //callback per als radio buttons
        void toggle_callback (GtkWidget *widget, gpointer data);
        
        //callback cridat en apretar enter dins un entry
        void enter_callback  (GtkWidget* widget, gpointer data);
                            
        //realitza la transformació de 2D a 3D
        gboolean transform2Dto3D (GtkWidget* widget, gpointer data);

        //realitza la transformació de 3D a 2D    
        gboolean transform3Dto2D (GtkWidget* widget, gpointer data);
    
        //controla la congelació de la captura
        gboolean freeze_callback (GtkWidget* widget, gpointer data);
    
        //controla el guardat del parell estereoscòpic
        gboolean save_callback   (GtkWidget* widget, gpointer data);
           
        //mode de renderització
        t_mode_3d mode3d;
        
        //entries per les coordenades 3D
        GtkWidget* entry_3d_x;
        GtkWidget* entry_3d_y;
        GtkWidget* entry_3d_z;
        
        //entries per les coordenades 2D imatge esquerra
        GtkWidget* entry_2d_u_l;
        GtkWidget* entry_2d_v_l;
        
        //entries per les coordenades 2D imatge dreta
        GtkWidget* entry_2d_u_r;
        GtkWidget* entry_2d_v_r;
        
        //drawing areas
        GtkWidget *drawing_area1;    
        GtkWidget *drawing_area2;
        GtkWidget *drawing_area3;
        GtkWidget *drawing_area_gl; 
                
        //primer algoritme
        GtkWidget* hscale11;   //també reutilitzarem aquest
        GtkWidget* hscale12;   
        GtkWidget* hscale13;   
        GtkWidget* hscale14;   
        GtkWidget* hscale15;   
        GtkWidget* hscale16;   
        
        //segon algoritme
        GtkWidget* hscale21;   
        GtkWidget* hscale22;   
        GtkWidget* hscale23;   
        GtkWidget* hscale24;   
        GtkWidget* hscale25;   
        GtkWidget* hscale26;   
        GtkWidget* hscale27;   
        
        //tercer algoritme
        GtkWidget* hscale31;   
        GtkWidget* hscale32;   
        GtkWidget* hscale33;   
        GtkWidget* hscale34;   
        GtkWidget* hscale35;   
        GtkWidget* hscale36;   
        GtkWidget* hscale37;   
               
        //botons de congelació i guardat de imatge
        GtkWidget* freezebutton;
        GtkWidget* savebutton;
        
    private:
        //radio button actiu ara mateix: "1" "2" o "3"
        char mRadioButtonAct;  
        
        //estat de la imatge: congelada o no
        bool freeze;     

        //pixmaps
        GdkPixmap *pixmapleft;    
        GdkPixmap *pixmapright;   
        GdkPixmap *pixmapdepth;  
                   
        //coordenades d'on s'ha fet click
        gdouble hline_x_left;
        gdouble hline_x_right;
        gdouble hline_y;
        
        //punters a objectes
        Ocv*   mOcv;
        svLib* mSv;
};

#endif
