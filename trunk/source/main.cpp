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

//arxiu main.cpp

#include <unistd.h>
#include <gtk/gtk.h>

#include "config.h"
#include "ocv.h"
#include "svapi.h"
#include "gui.h"
#include "wrapper.h"
#include "ogl.h"

//objectes    
Config* config;     //objecte de configuració
svLib* sv;          //objecte d'estereovisió
Ocv* ocv;           //objecte Ocv pel tractament d'imatges i altres operacions
Gui* gui;           //objecte d'interficie

//funció del thread
void *idle_thread (void *args)
{
    for(;;){
        gdk_threads_enter ();
        gui->idle (NULL, ocv);
        gdk_flush ();
        gdk_threads_leave ();
    }
}

//inicialització
int init ()
{
    config = new Config ();
    sv  = new svLib (config);
    ocv = new Ocv (config);
    gui = new Gui (ocv, sv);    
}

//callback per sortir del programa
void quit ()
{
    timeout_remove (gui->drawing_area_gl);
    gdk_threads_enter ();
    ocv->ocvEnd ();     //finalitzam opencv
    freeTexture ();     //alliberam textura si escau

    //hem d'alliberar estructures de memoria
    delete config;
    delete sv;
    delete ocv;
    delete gui;
    gtk_main_quit ();  //finalitzam gtk+
}

//programa principal
int main (int argc, char *argv[])
{
    //carregam configuració
    init ();

    //inicialitzam classe sv
    sv->init();
      
    //declaracions locals de widgets
    GtkWidget *imagetable;    //taula per les imatges
    GtkWidget *controltable;  //taula pels controls
    GtkWidget *maintable;     //taula principal

    GtkWidget *button;        //punter a un botó
    GtkWidget *frame;         //punter a frame
             
    //inicialitzam threads
    g_thread_init (NULL);
    gdk_threads_init ();    
    
    gtk_init (&argc, &argv);  //inicialitzam gtk+

    GtkWidget *window;        //finestra principal

    //cream finestra
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW(window), "Demo Estereo");
    gtk_quit_add_destroy (1, GTK_OBJECT(window));

    //establim tamany per defecte
    gtk_window_set_default_size (GTK_WINDOW(window), 
                                 (int)(config->getDefWidth() * 2.2), 
                                 (int)(config->getDefHeight() * 2.4));
    
    //callback per destruir l'aplicació
    g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (quit), NULL);

    //connectam el senyal a la finestra
    g_signal_connect_swapped (G_OBJECT (window), "key_press_event",
                              G_CALLBACK (key_press_event), NULL);

    //inicialitzacio OpenGL
    //---------------------
    
    GdkGLConfig *glconfig;
    gint major, minor;

    //inicialitzam GtkGlExt   
    gtk_gl_init (&argc, &argv);
    
    //demanam la versió de la extensió OpenGL   
    gdk_gl_query_version (&major, &minor);
    g_print ("\nOpenGL extension version - %d.%d\n", major, minor);
        
    //configuram el framebuffer OpenGL
    glconfig = configure_gl ();
 
    //definició de la interficie
    //--------------------------
    
    //taula principal 1x2, imatges a l'esquerra, controls a la dreta
    maintable = gtk_table_new(1, 2, FALSE);
    gtk_container_add(GTK_CONTAINER(window), maintable);
    
    //cream taula per les imatges
    imagetable = gtk_table_new(2, 2, true); 
    
    //cream un frame per a la taula de les imatges
    frame = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME(frame), "Imatges" );
    gtk_frame_set_label_align (GTK_FRAME(frame), 0.5, 0.0);
    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_ETCHED_OUT);        

    //posam la taula dins el frame
    gtk_container_add (GTK_CONTAINER (frame), imagetable);
    
    //ficam el frame a l'esquerra de la taula
    gtk_table_attach_defaults (GTK_TABLE (maintable), frame, 0, 1, 0, 1);
    
    //cream la taula de controls
    controltable = gtk_table_new (7, 1, false);
    gtk_table_attach_defaults (GTK_TABLE (maintable), controltable, 1, 2, 0, 1);

    // DRAWING AREA 1
    //cream el drawing area 1 (imatge esquerre)
    gui->drawing_area1 = gtk_drawing_area_new ();
    //posam la mida
    gtk_widget_set_size_request (GTK_WIDGET (gui->drawing_area1), 
                                 config->getDefWidth(), 
                                 config->getDefHeight());
    
    //ficam el drawing area dins un frame, i el frame a la taula
    //cream un frame per a la taula de les imatges
    frame = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME(frame), "Esquerra" );    
    gtk_frame_set_label_align (GTK_FRAME(frame), 0.5, 0.0);
    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_ETCHED_OUT);        
    gtk_container_add (GTK_CONTAINER (frame), gui->drawing_area1);
    gtk_table_attach_defaults (GTK_TABLE (imagetable), frame, 0, 1, 0, 1);

    //connectam senyals
    g_signal_connect (G_OBJECT (gui->drawing_area1), "expose_event",
                      G_CALLBACK (expose_event), (gpointer)"left");
    
    //prova experimental!!!
    g_signal_connect (G_OBJECT (gui->drawing_area1),"configure_event",
                      G_CALLBACK (configure_event), (gpointer)"left");
    
    ///connectam senyals d'esdeveniments
    g_signal_connect (G_OBJECT (gui->drawing_area1), "motion_notify_event",
                      G_CALLBACK (motion_notify_event), (gpointer)"left");
    g_signal_connect (G_OBJECT (gui->drawing_area1), "button_press_event",
                      G_CALLBACK (button_press_event), (gpointer)"left");
    
    //indicam els esdeveniments que rebrà el widget
    gtk_widget_set_events (gui->drawing_area1, GDK_EXPOSURE_MASK
                           | GDK_LEAVE_NOTIFY_MASK
                           | GDK_BUTTON_PRESS_MASK
                           | GDK_POINTER_MOTION_MASK
                           | GDK_POINTER_MOTION_HINT_MASK);
    
    // DRAWING AREA 2
    //cream el drawing area 2 (imatge dreta)
    gui->drawing_area2 = gtk_drawing_area_new ();
    //posam la mida
    gtk_widget_set_size_request (GTK_WIDGET(gui->drawing_area2), 
                                 config->getDefWidth(), 
                                 config->getDefHeight());

    frame = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME(frame), "Dreta" );    
    gtk_frame_set_label_align (GTK_FRAME(frame), 0.5, 0.0);
    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_ETCHED_OUT);        
    gtk_container_add (GTK_CONTAINER (frame), gui->drawing_area2);
    gtk_table_attach_defaults (GTK_TABLE (imagetable), frame, 1, 2, 0, 1);
       
    //connectam senyals
    g_signal_connect (G_OBJECT (gui->drawing_area2), "expose_event",
                      G_CALLBACK (expose_event), (gpointer)"right");
    g_signal_connect (G_OBJECT (gui->drawing_area2),"configure_event",
                      G_CALLBACK (configure_event), (gpointer)"right");
                    
    //esdeveniments
    g_signal_connect (G_OBJECT (gui->drawing_area2), "motion_notify_event",
                      G_CALLBACK (motion_notify_event), (gpointer)"right");
    g_signal_connect (G_OBJECT (gui->drawing_area2), "button_press_event",
                      G_CALLBACK (button_press_event), (gpointer)"right");
    
    //indicam els esdeveniments que rebrà el widget
    gtk_widget_set_events (gui->drawing_area2, GDK_EXPOSURE_MASK
                           | GDK_LEAVE_NOTIFY_MASK
                           | GDK_BUTTON_PRESS_MASK
                           | GDK_POINTER_MOTION_MASK
                           | GDK_POINTER_MOTION_HINT_MASK);
    
    // DRAWING AREA 3
    //cream el drawing area 3 (imatge disparitat)
    gui->drawing_area3 = gtk_drawing_area_new ();
    //posam la mida
    gtk_widget_set_size_request (GTK_WIDGET (gui->drawing_area3), 
                                 config->getDefWidth(), 
                                 config->getDefHeight());

    frame = gtk_frame_new(NULL);
    gtk_frame_set_label( GTK_FRAME(frame), "Disparitat" );    
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.0);
    gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_ETCHED_OUT);        
    gtk_container_add (GTK_CONTAINER (frame), gui->drawing_area3);
    gtk_table_attach_defaults (GTK_TABLE (imagetable), frame, 0, 1, 1, 2);
      
    /* Signals used to handle backing pixmap */
    g_signal_connect (G_OBJECT (gui->drawing_area3), "expose_event",
                    G_CALLBACK (expose_event), (gpointer)"depth");
    g_signal_connect (G_OBJECT (gui->drawing_area3),"configure_event",
                    G_CALLBACK (configure_event), (gpointer)"depth");
    
    //indicam els events que rebrà el widget
    gtk_widget_set_events (gui->drawing_area3, GDK_EXPOSURE_MASK
                           | GDK_LEAVE_NOTIFY_MASK
                           | GDK_BUTTON_PRESS_MASK
                           | GDK_POINTER_MOTION_MASK
                           | GDK_POINTER_MOTION_HINT_MASK);
    
    //drawing area per a l'OpenGL    
    gui->drawing_area_gl = gtk_drawing_area_new ();
    gtk_widget_set_size_request (gui->drawing_area_gl, 
                                 config->getDefWidth(), 
                                 config->getDefHeight());

    //cream el frame
    frame = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME(frame), "Reconstruccio 3D");    
    gtk_frame_set_label_align (GTK_FRAME(frame), 0.5, 0.0);
    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_ETCHED_OUT);        
    gtk_container_add (GTK_CONTAINER (frame), gui->drawing_area_gl);
    gtk_table_attach_defaults (GTK_TABLE (imagetable), frame, 1, 2, 1, 2);
    
    //Donam capacitat OpenGL al widget
    gtk_widget_set_gl_capability (gui->drawing_area_gl,
                                  glconfig,
                                  NULL,
                                  TRUE,
                                  GDK_GL_RGBA_TYPE);
   
    
    //Donam capacitat OpenGL al widget
    gtk_widget_set_gl_capability (gui->drawing_area_gl,
    			                  glconfig,
   			                      NULL,
    			                  TRUE,
                    		      GDK_GL_RGBA_TYPE);
    
    gtk_widget_add_events (gui->drawing_area_gl,
    		               GDK_BUTTON1_MOTION_MASK    
                           | GDK_BUTTON2_MOTION_MASK    
                           | GDK_BUTTON_PRESS_MASK      
                           | GDK_VISIBILITY_NOTIFY_MASK);
    
    //connectam els callbacks dels signals
    g_signal_connect_after (G_OBJECT (gui->drawing_area_gl), "realize",
                            G_CALLBACK (realize_gl), NULL);
                          
    g_signal_connect (G_OBJECT (gui->drawing_area_gl), "configure_event",
    	              G_CALLBACK (configure_event_gl), NULL);
    	    
    g_signal_connect (G_OBJECT (gui->drawing_area_gl), "expose_event",
    	              G_CALLBACK (expose_event_gl), NULL);
    
    g_signal_connect (G_OBJECT (gui->drawing_area_gl), "motion_notify_event",
               	      G_CALLBACK (motion_notify_event_gl), NULL);
    	    
    g_signal_connect (G_OBJECT (gui->drawing_area_gl), "button_press_event",
    	              G_CALLBACK (button_press_event_gl), NULL);
    	       
    g_signal_connect (G_OBJECT (gui->drawing_area_gl), "unrealize",
    	              G_CALLBACK (unrealize_gl), NULL);
       
    /* For timeout function. */
    g_signal_connect (G_OBJECT (gui->drawing_area_gl), "map_event",
    	              G_CALLBACK (map_event_gl), NULL);

    g_signal_connect (G_OBJECT (gui->drawing_area_gl), "unmap_event",
    	              G_CALLBACK (unmap_event_gl), NULL);

    g_signal_connect (G_OBJECT (gui->drawing_area_gl), 
                      "visibility_notify_event",
    	              G_CALLBACK (visibility_notify_event_gl), NULL);
    
    
   //menú pop-up
    GtkWidget* menu;
    menu = create_popup_menu (gui->drawing_area_gl);
        
    g_signal_connect_swapped (G_OBJECT (gui->drawing_area_gl), 
                              "button_press_event",
                              G_CALLBACK (button_press_event_popup_menu), 
                              menu);

    //radio buttons
    //3 algoritmes de disparitat
    GtkWidget* controltablesub1 = gtk_table_new (3, 1, false); 

    //ficam la taula dins un frame
    frame = gtk_frame_new (NULL);
    gtk_frame_set_label (GTK_FRAME(frame), "Algoritmes de disparitat" );
    gtk_frame_set_label_align (GTK_FRAME(frame), 0.5, 0.0);
    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_ETCHED_OUT);        
    gtk_container_add (GTK_CONTAINER (frame), controltablesub1);
    //ficam el frame dins la taula de controls
    gtk_table_attach_defaults (GTK_TABLE (controltable), frame, 0, 1, 0, 1);

    //cream els RadioButton i els ficam a la subtaula
    GSList* group;
    
    //primer botó
    button = gtk_radio_button_new_with_label (NULL, 
                                             "Algoritme Birchfield-Tomasi");
    g_signal_connect (G_OBJECT (button), "toggled", 
                      G_CALLBACK (toggle_callback), (gpointer)"1");
    gtk_table_attach_defaults (GTK_TABLE (controltablesub1), 
                               button, 0, 1, 0, 1);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);

    //segon botó
    button = gtk_radio_button_new_with_label
           (gtk_radio_button_group (GTK_RADIO_BUTTON (button)), 
            "Algoritme Finestra + SAD");
    g_signal_connect (G_OBJECT (button), "toggled",
                      G_CALLBACK (toggle_callback), (gpointer)"2"); 
    gtk_table_attach_defaults (GTK_TABLE (controltablesub1), 
                               button, 0, 1, 1, 2);

    //tercer botó
    button = gtk_radio_button_new_with_label
           (gtk_radio_button_group (GTK_RADIO_BUTTON (button)), 
            "Algoritme RSR + CC");
    g_signal_connect (G_OBJECT (button), "toggled",
                      G_CALLBACK (toggle_callback), (gpointer)"3");
    gtk_table_attach_defaults (GTK_TABLE (controltablesub1), 
                               button, 0, 1, 2, 3);

    //Scale de disparitat
    controltablesub1 = gtk_table_new(7, 1, false); //6 paràmetres de l'algoritme
 
    //ficam la taula dins un frame
    frame = gtk_frame_new(NULL);
    gtk_frame_set_label( GTK_FRAME(frame), "Parametres disparitat" );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.0);
    gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_ETCHED_OUT);        
    gtk_container_add (GTK_CONTAINER (frame), controltablesub1);
  
     //ficam el frame dins la taula de controls
    gtk_table_attach_defaults (GTK_TABLE (controltable), frame, 0, 1, 1, 2);
  
    //cream els Scale
    //els ficam dins una taula
    GtkObject *adj;     
    GtkWidget* hscale;   
    
    //primer scale
    adj = gtk_adjustment_new (ocv->disparam11, 1.0, 101.0, 1.0, 1.0, 1.0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                        GTK_SIGNAL_FUNC (adj_callback), (gpointer)"11");
    gui->hscale11 = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_table_attach_defaults (GTK_TABLE (controltablesub1), gui->hscale11, 
                               0, 1, 0, 1);
    
    adj = gtk_adjustment_new (ocv->disparam21, 1.0, 20.0, 1.0, 1.0, 1.0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                        GTK_SIGNAL_FUNC (adj_callback), (gpointer)"21");
    gui->hscale21 = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_table_attach_defaults (GTK_TABLE (controltablesub1), gui->hscale21, 
                               0, 1, 0, 1);

    adj = gtk_adjustment_new (ocv->disparam31, 1.0, 20.0, 1.0, 1.0, 1.0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                        GTK_SIGNAL_FUNC (adj_callback), (gpointer)"31");
    gui->hscale31 = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_table_attach_defaults (GTK_TABLE (controltablesub1), gui->hscale31, 
                               0, 1, 0, 1);
       
    //segon scale
    adj = gtk_adjustment_new (ocv->disparam12, 0.0, 101.0, 1.0, 1.0, 1.0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                        GTK_SIGNAL_FUNC (adj_callback), (gpointer)"12");
    gui->hscale12 = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_table_attach_defaults (GTK_TABLE (controltablesub1), gui->hscale12, 
                               0, 1, 1, 2);
    
    adj = gtk_adjustment_new (ocv->disparam22, 1.0, 20.0, 1.0, 1.0, 1.0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                        GTK_SIGNAL_FUNC (adj_callback), (gpointer)"22");
    gui->hscale22 = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_table_attach_defaults (GTK_TABLE (controltablesub1), gui->hscale22, 
                               0, 1, 1, 2);
    
    adj = gtk_adjustment_new (ocv->disparam32, 1.0, 20.0, 1.0, 1.0, 1.0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                        GTK_SIGNAL_FUNC (adj_callback), (gpointer)"32");
    gui->hscale32 = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_table_attach_defaults (GTK_TABLE (controltablesub1), gui->hscale32, 
                               0, 1, 1, 2);    
    
    //tercer scale
    adj = gtk_adjustment_new (ocv->disparam13, 0.0, 101.0, 1.0, 1.0, 1.0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                        GTK_SIGNAL_FUNC (adj_callback), (gpointer)"13");
    gui->hscale13 = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_table_attach_defaults (GTK_TABLE (controltablesub1), gui->hscale13, 
                               0, 1, 2, 3);

    adj = gtk_adjustment_new (ocv->disparam23, 1.0, 50.0, 1.0, 1.0, 1.0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                        GTK_SIGNAL_FUNC (adj_callback), (gpointer)"23");
    gui->hscale23 = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_table_attach_defaults (GTK_TABLE (controltablesub1), gui->hscale23, 
                               0, 1, 2, 3); 
 
    adj = gtk_adjustment_new (ocv->disparam33, 1.0, 20.0, 1.0, 1.0, 1.0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                        GTK_SIGNAL_FUNC (adj_callback), (gpointer)"33");
    gui->hscale33 = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_table_attach_defaults (GTK_TABLE (controltablesub1), gui->hscale33, 
                               0, 1, 2, 3);
 
    //quart scale
    adj = gtk_adjustment_new (ocv->disparam14, 0.0, 101.0, 1.0, 1.0, 1.0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                        GTK_SIGNAL_FUNC (adj_callback), (gpointer)"14");
    gui->hscale14 = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_table_attach_defaults (GTK_TABLE (controltablesub1), gui->hscale14, 
                               0, 1, 3, 4);

    adj = gtk_adjustment_new (ocv->disparam24, 1.0, 20.0, 1.0, 1.0, 1.0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                        GTK_SIGNAL_FUNC (adj_callback), (gpointer)"24");
    gui->hscale24 = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_table_attach_defaults (GTK_TABLE (controltablesub1), gui->hscale24, 
                               0, 1, 3, 4);
    
    adj = gtk_adjustment_new (ocv->disparam34, 1.0, 20.0, 1.0, 1.0, 1.0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                        GTK_SIGNAL_FUNC (adj_callback), (gpointer)"34");
    gui->hscale34 = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_table_attach_defaults (GTK_TABLE (controltablesub1), gui->hscale34, 
                               0, 1, 3, 4);    
 
    //cinquè scale
    adj = gtk_adjustment_new (ocv->disparam15, 0.0, 101.0, 1.0, 1.0, 1.0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                        GTK_SIGNAL_FUNC (adj_callback), (gpointer)"15");
    gui->hscale15 = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_table_attach_defaults (GTK_TABLE (controltablesub1), gui->hscale15, 
                               0, 1, 4, 5);

    adj = gtk_adjustment_new (ocv->disparam25, 0.0, 101.0, 1.0, 1.0, 1.0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                        GTK_SIGNAL_FUNC (adj_callback), (gpointer)"25");
    gui->hscale25 = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_table_attach_defaults (GTK_TABLE (controltablesub1), gui->hscale25, 
                               0, 1, 4, 5);

    adj = gtk_adjustment_new (ocv->disparam35, 0.0, 101.0, 1.0, 1.0, 1.0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                        GTK_SIGNAL_FUNC (adj_callback), (gpointer)"35");
    gui->hscale35 = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_table_attach_defaults (GTK_TABLE (controltablesub1), gui->hscale35, 
                               0, 1, 4, 5);    

    //sisè scale
    adj = gtk_adjustment_new (ocv->disparam16, 0.0, 101.0, 1.0, 1.0, 1.0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                        GTK_SIGNAL_FUNC (adj_callback), (gpointer)"16");
    gui->hscale16 = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_table_attach_defaults (GTK_TABLE (controltablesub1), gui->hscale16, 
                               0, 1, 5, 6);

    //SCALE CONTORNS
    controltablesub1 = gtk_table_new(3, 1, false); 
 
    //ficam la taula dins un frame
    frame = gtk_frame_new(NULL);
    gtk_frame_set_label( GTK_FRAME(frame), "Nivell de detall 3D" );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.0);
    gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_ETCHED_OUT);        
    gtk_container_add (GTK_CONTAINER (frame), controltablesub1);
  
     //ficam el frame dins la taula de controls
    gtk_table_attach_defaults (GTK_TABLE (controltable), frame, 0, 1, 2, 3);
  
    //TEXTBOXES [coordenades 2D]   
    GtkWidget* coordtable = gtk_table_new(1, 2, true); //per les subtaules
    //cream un frame per a la taula de les imatges
    frame = gtk_frame_new(NULL);
    // Set the frame's label
    gtk_frame_set_label( GTK_FRAME(frame), "Coordenades 2D" );
    // Align the label at the right of the frame */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.0);
    // Set the style of the frame
    gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_ETCHED_OUT);        
    // Put the table in the main window
    gtk_container_add (GTK_CONTAINER (frame), coordtable);
    //ficam el frame a l'esquerra de la taula
    gtk_table_attach_defaults (GTK_TABLE (controltable), frame, 0, 1, 3, 4);    
    
    //cream la taula per ficar-hi les subtaules
    GtkWidget* subcoordtable = gtk_table_new(2, 1, true); 
    
    gui->entry_2d_u_l = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(gui->entry_2d_u_l), "0.0");
    gtk_editable_set_editable(GTK_EDITABLE(gui->entry_2d_u_l), TRUE);
    gtk_entry_set_max_length (GTK_ENTRY (gui->entry_2d_u_l), 50);
    gtk_entry_set_width_chars (GTK_ENTRY (gui->entry_2d_u_l), 10);
    g_signal_connect (G_OBJECT (gui->entry_2d_u_l), "activate",
		      G_CALLBACK (enter_callback),
		     (gpointer)"2D");  
    gtk_table_attach_defaults (GTK_TABLE (subcoordtable), gui->entry_2d_u_l, 
                               0, 1, 0, 1);

    gui->entry_2d_v_l = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(gui->entry_2d_v_l), "0.0");
    gtk_editable_set_editable(GTK_EDITABLE(gui->entry_2d_v_l), TRUE);
    gtk_entry_set_max_length (GTK_ENTRY (gui->entry_2d_v_l), 50);
    gtk_entry_set_width_chars (GTK_ENTRY (gui->entry_2d_v_l), 10);
    g_signal_connect (G_OBJECT (gui->entry_2d_v_l), "activate",
		      G_CALLBACK (enter_callback),
		     (gpointer)"2D");  
    gtk_table_attach_defaults (GTK_TABLE (subcoordtable), gui->entry_2d_v_l, 
                               0, 1, 1, 2);

    //ficam les subtaules dins un frame i el frame a la taula coordtable
    //frames subtaules de coord - subframe esquerra
    frame = gtk_frame_new(NULL);
    gtk_frame_set_label( GTK_FRAME(frame), "Esquerra" );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.0);
    gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_ETCHED_OUT);        
    gtk_container_add (GTK_CONTAINER (frame), subcoordtable);
    gtk_table_attach_defaults (GTK_TABLE (coordtable), frame, 0, 1, 0, 1);  

    //punt dret
    subcoordtable = gtk_table_new(2, 1, true); //hi ficarem les subtaules    
    
    gui->entry_2d_u_r = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(gui->entry_2d_u_r), "0.0");
    gtk_editable_set_editable(GTK_EDITABLE(gui->entry_2d_u_r), TRUE);
    gtk_entry_set_max_length (GTK_ENTRY (gui->entry_2d_u_r), 50);
    gtk_entry_set_width_chars (GTK_ENTRY (gui->entry_2d_u_r), 10);
    g_signal_connect (G_OBJECT (gui->entry_2d_u_r), "activate",
		      G_CALLBACK (enter_callback),
		     (gpointer)"2D");  
    gtk_table_attach_defaults (GTK_TABLE (subcoordtable), gui->entry_2d_u_r, 
                               0, 1, 0, 1);

    gui->entry_2d_v_r = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(gui->entry_2d_v_r), "0.0");
    gtk_editable_set_editable(GTK_EDITABLE(gui->entry_2d_v_r), TRUE);
    gtk_entry_set_max_length (GTK_ENTRY (gui->entry_2d_v_r), 50);
    gtk_entry_set_width_chars (GTK_ENTRY (gui->entry_2d_v_r), 10);
    g_signal_connect (G_OBJECT (gui->entry_2d_v_r), "activate",
		      G_CALLBACK (enter_callback),
		     (gpointer)"2D");  
    gtk_table_attach_defaults (GTK_TABLE (subcoordtable), gui->entry_2d_v_r, 
                               0, 1, 1, 2);

    //ficam les subtaules dins un frame i el frame a la taula coordtable
    //frames subtaules de coord - subframe esquerra
    frame = gtk_frame_new(NULL);
    gtk_frame_set_label( GTK_FRAME(frame), "Dreta" );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.0);
    gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_ETCHED_OUT);        
    gtk_container_add (GTK_CONTAINER (frame), subcoordtable);
    gtk_table_attach_defaults (GTK_TABLE (coordtable), frame, 1, 2, 0, 1);  

    //COORDENADES 3D 

    coordtable = gtk_table_new(1, 3, true);
    //cream un frame per a la taula de les imatges
    frame = gtk_frame_new(NULL);
    gtk_frame_set_label( GTK_FRAME(frame), "Coordenades 3D" );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.0);
    gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_ETCHED_OUT);        
    gtk_container_add (GTK_CONTAINER (frame), coordtable);
    //ficam el frame a l'esquerra de la taula
    gtk_table_attach_defaults (GTK_TABLE (controltable), frame, 0, 1, 4, 5);    
    
    gui->entry_3d_x = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(gui->entry_3d_x), "0.0");
    gtk_editable_set_editable(GTK_EDITABLE(gui->entry_3d_x), TRUE);
    gtk_entry_set_max_length (GTK_ENTRY (gui->entry_3d_x), 50);
    gtk_entry_set_width_chars (GTK_ENTRY (gui->entry_3d_x), 10);
    g_signal_connect (G_OBJECT (gui->entry_3d_x), "activate",
		      G_CALLBACK (enter_callback),
		     (gpointer)"3D");  
    gtk_table_attach_defaults (GTK_TABLE (coordtable), gui->entry_3d_x, 
                               0, 1, 0, 1);

    gui->entry_3d_y = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(gui->entry_3d_y), "0.0");
    gtk_editable_set_editable(GTK_EDITABLE(gui->entry_3d_y), TRUE);
    gtk_entry_set_max_length (GTK_ENTRY (gui->entry_3d_y), 50);
    gtk_entry_set_width_chars (GTK_ENTRY (gui->entry_3d_y), 10);
    g_signal_connect (G_OBJECT (gui->entry_3d_y), "activate",
		      G_CALLBACK (enter_callback),
		     (gpointer)"3D");  
    gtk_table_attach_defaults (GTK_TABLE (coordtable), gui->entry_3d_y, 
                               1, 2, 0, 1);

    gui->entry_3d_z = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(gui->entry_3d_z), "0.0");
    gtk_editable_set_editable(GTK_EDITABLE(gui->entry_3d_z), TRUE);
    gtk_entry_set_max_length (GTK_ENTRY (gui->entry_3d_z), 50);
    gtk_entry_set_width_chars (GTK_ENTRY (gui->entry_3d_z), 10);
    g_signal_connect (G_OBJECT (gui->entry_3d_z), "activate",
		      G_CALLBACK (enter_callback),
		     (gpointer)"3D");  
    gtk_table_attach_defaults (GTK_TABLE (coordtable), gui->entry_3d_z, 
                               2, 3, 0, 1);

    //BOTONS DE TRANSFORMACIÓ
   
    GtkWidget* buttontable = gtk_table_new(2, 2, false); 

    //ficam la taula dins l'altre taula
    gtk_table_attach_defaults (GTK_TABLE (controltable),buttontable, 
                               0, 1, 5, 6);
    
    //botó 3d a 2d
    GtkWidget *vbox;
    vbox = gtk_vbox_new (FALSE, 0);
    button = gtk_button_new_with_label ("3D a 2D");
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
    gtk_widget_set_size_request(GTK_WIDGET (vbox), 50, 20);
    gtk_table_attach_defaults (GTK_TABLE (buttontable), vbox, 
                               0, 1, 0, 1);
    
    //connectam l'objecte al callback
    g_signal_connect_swapped (G_OBJECT (button), "clicked",
                            G_CALLBACK (transform3Dto2D),
                            G_OBJECT (window));
    
    //botó 2d a 3d
    vbox = gtk_vbox_new (FALSE, 0);
    button = gtk_button_new_with_label ("2D a 3D");
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
    gtk_widget_set_size_request(GTK_WIDGET (vbox), 50, 20);
    gtk_table_attach_defaults (GTK_TABLE (buttontable), vbox, 1, 2, 0, 1);
    
    //connectam l'objecte al callback 
    g_signal_connect_swapped (G_OBJECT (button), "clicked",
                            G_CALLBACK (transform2Dto3D),
                            G_OBJECT (window));


    //subtaula congelar/guardar
    buttontable = gtk_table_new(2, 2, false); 

    //ficam la taula dins l'altre taula
    gtk_table_attach_defaults (GTK_TABLE (controltable),buttontable, 
                               0, 1, 6, 7);

    //botó Congelar
    vbox = gtk_vbox_new (FALSE, 0);
    gui->freezebutton = gtk_button_new_with_label ("Congelar");
    gtk_box_pack_start (GTK_BOX (vbox), gui->freezebutton, FALSE, FALSE, 0);
    gtk_widget_set_size_request(GTK_WIDGET (vbox), 50, 20);
    gtk_table_attach_defaults (GTK_TABLE (buttontable), vbox, 0, 1, 0, 1);
    
    //connectam l'objecte al callback
    g_signal_connect_swapped (G_OBJECT (gui->freezebutton), "clicked",
                            G_CALLBACK (freeze_callback),
                            G_OBJECT (window));

    //botó Guardar
    vbox = gtk_vbox_new (FALSE, 0);
    gui->savebutton = gtk_button_new_with_label ("Guardar");
    gtk_box_pack_start (GTK_BOX (vbox), gui->savebutton, FALSE, FALSE, 0);
    gtk_widget_set_size_request(GTK_WIDGET (vbox), 50, 20);
    gtk_table_attach_defaults (GTK_TABLE (buttontable), vbox, 1, 2, 0, 1);
    
    //connectam l'objecte al callback
    g_signal_connect_swapped (G_OBJECT (gui->savebutton), "clicked",
                            G_CALLBACK (save_callback),
                            G_OBJECT (window));

     
    //botó Sortir
    vbox = gtk_vbox_new (FALSE, 0);
    button = gtk_button_new_with_label ("Sortir");
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
    gtk_widget_set_size_request(GTK_WIDGET (vbox), 50, 20);
    gtk_table_attach_defaults (GTK_TABLE (controltable), vbox, 0, 1, 7, 8);
    
    //connectam l'objecte al callback
    g_signal_connect_swapped (G_OBJECT (button), "clicked",
                            G_CALLBACK (gtk_widget_destroy),
                            G_OBJECT (window));

    //codi GtkGlExt (OpenGL)

    gtk_gl_init (&argc, &argv);
    
    //demanam la versió de l'extensió OpenGL    
    gdk_gl_query_version (&major, &minor);
    g_print ("\nOpenGL extension version - %d.%d\n",
           major, minor);
    
    //configuram la visualització OpenGL
    //Intentam double-buffering
    glconfig = gdk_gl_config_new_by_mode (static_cast<GdkGLConfigMode>
                   (GDK_GL_MODE_RGB    |
                    GDK_GL_MODE_DEPTH  |
                    GDK_GL_MODE_DOUBLE) );
    
    if (glconfig == NULL)
    {
      g_print ("*** Cannot find the double-buffered visual.\n");
      g_print ("*** Trying single-buffered visual.\n");
    
    //intentam visualització amb single-buffer

        glconfig = gdk_gl_config_new_by_mode (static_cast<GdkGLConfigMode>
                                              (GDK_GL_MODE_RGB   
                                              | GDK_GL_MODE_DEPTH));
      
      if (glconfig == NULL)
        {
          g_print ("*** No appropriate OpenGL-capable visual found.\n");
          exit (1);
        }
    }
    
    //iniciam OpenCv
    ocv->ocvStart();
    
    //tercer scale - determina correctament el màxim nivell de disparitat
    adj = gtk_adjustment_new (ocv->mImageStep, 2.0, 
                              (float)ocv->mMaxStep+1.0, 1.0, 1.0, 1.0);
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                        GTK_SIGNAL_FUNC (adj_callback), (gpointer)"-3");
    hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_table_attach_defaults (GTK_TABLE (controltablesub1), hscale, 
                               0, 1, 2, 3);
    
    //Pas previ a l'execució
        
    //mostram tots els widgets
    gtk_widget_show_all (GTK_WIDGET(window));
    
    //ocultam el widgets dels algoritmes que no es fan servir al començament
    gui->hide_widgets_alg2 ();
    gui->hide_widgets_alg3 ();

    GError *error = NULL;
    int yes_args;
    
    if (!g_thread_create (idle_thread, &yes_args, FALSE, &error)){
        g_printerr ("Failed to create YES thread: %s\n", error->message);
        return 1;
    }

    //entram al bucle principal de Gtk+
    gdk_threads_enter ();
    gtk_main ();
    gdk_threads_leave ();
    
    //retornam  
    return 0;
}
