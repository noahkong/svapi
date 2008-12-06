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

//arxiu gui.cpp

#include "gui.h"
#include "time.h"

//constructor
Gui::Gui (Ocv* ocv, svLib* sv)
{
    strcpy(&mRadioButtonAct, "1");  //el radio button actiu és el 1er
    pixmapleft = NULL;  //pixmap esquerre
    pixmapright = NULL; //pixmap dret
    pixmapdepth = NULL; //pixmap dret
    mode3d = MDE_POLYGON;
    freeze = false;
    this->mOcv = ocv;
    this->mSv = sv;
}

//destructor
Gui::~Gui()
{
}

gboolean Gui::expose_event_drawarea (GtkWidget      *widget,
                                     GdkEventExpose *event,
                                     gpointer       data)
{   
    //al drawing area de l'esquerra ensenyam la imatge dreta
    if (strcmp((char*)data, "left")==0){
        gdk_draw_drawable(widget->window,
                          widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                          pixmapright,                        
                          event->area.x, event->area.y,
                          event->area.x, event->area.y,
                          event->area.width, event->area.height);
    }
    
    //al drawing area de la dreta ensenyam la imatge esquerra
    if (strcmp((char*)data, "right")==0){
        gdk_draw_drawable(widget->window,
                          widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                          pixmapleft,                          
                          event->area.x, event->area.y,
                          event->area.x, event->area.y,
                          event->area.width, event->area.height);
    }    
    
    if (strcmp((char*)data, "depth")==0){
        gdk_draw_drawable(widget->window,
                          widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                          pixmapdepth,                           
                          event->area.x, event->area.y,
                          event->area.x, event->area.y,
                          event->area.width, event->area.height);
    }    
    return FALSE;
}


gboolean Gui::configure_event_drawarea (GtkWidget         *widget,
                                        GdkEventConfigure *event,
                                        gpointer          data)
{
    //ESQUERRA
    if (pixmapleft){
       g_object_unref(pixmapleft);     //l'elimina
    }
    //en crea un de nou
    pixmapleft = gdk_pixmap_new(widget->window,
                                widget->allocation.width,
                                widget->allocation.height,
                                -1);

    //si no hi ha pixbuf, el crea
    if (mOcv->pixbufleft == NULL){
       mOcv->pixbufleft = gdk_pixbuf_new_from_file((gchar *) "data/empty.png", NULL);
    }
    
    if (mOcv->pixbufleft){
       gdk_draw_pixbuf(pixmapleft, NULL, mOcv->pixbufleft, 
                       0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_NORMAL, 0, 0);
    }else{
        g_print("No pixbuf!\n");
    }
    
    //DRETA
    if (pixmapright){
       g_object_unref (pixmapright);     //l'elimina
    }
    //en crea un de nou
    pixmapright = gdk_pixmap_new(widget->window,
                                 widget->allocation.width,
                                 widget->allocation.height,
                                 -1);

    //si no hi ha pixbuf, el crea
    if (mOcv->pixbufright == NULL){
       mOcv->pixbufright = gdk_pixbuf_new_from_file((gchar *) "data/empty.png", NULL);
    }

    if (mOcv->pixbufright){
       gdk_draw_pixbuf(pixmapright, NULL, mOcv->pixbufright, 0, 0, 0, 0, -1, -1, 
                       GDK_RGB_DITHER_NORMAL, 0, 0);
    }else{
        g_print("No pixbuf!\n");
    }
   
    //PROFUNDITAT
    if (pixmapdepth){
       g_object_unref (pixmapdepth);     //l'elimina
    }
    
    //en crea un de nou
    pixmapdepth = gdk_pixmap_new (widget->window,
                                  widget->allocation.width,
                                  widget->allocation.height,
                                  -1);

    //si no hi ha pixbuf, el crea
    if (mOcv->pixbufdepth == NULL){
       mOcv->pixbufdepth = gdk_pixbuf_new_from_file((gchar *) "data/empty.png", 
                                                    NULL);
    }

    if (mOcv->pixbufdepth){
       gdk_draw_pixbuf(pixmapdepth, NULL, mOcv->pixbufdepth, 0, 0, 0, 0, -1, -1, 
                       GDK_RGB_DITHER_NORMAL, 0, 0);
    }else{
        g_print("No pixbuf!\n");
    }
    
    return TRUE;
}


//Dibuixat de les línies allà on es fa click, per les imatges dreta i esquerra
void Gui::draw_brush (GtkWidget *widget, gdouble x, gdouble y, gpointer  data)
{
    //printf("%s\n", (char*)data);

    char u[40];
    sprintf(u, "%f", x);

    if (strcmp((char*)data, "left")==0){
        hline_x_left = x;
        //actualitzam coordenada u
   	    gtk_entry_set_text(GTK_ENTRY (entry_2d_u_l), u);
    }    
    if (strcmp((char*)data, "right")==0){
        hline_x_right = x;
        //actualitzam coordenada u
   	    gtk_entry_set_text(GTK_ENTRY (entry_2d_u_r), u);
    }
    
    char v[40];
    sprintf(v, "%f", y);
    //actualitzam els dos v
    gtk_entry_set_text(GTK_ENTRY (entry_2d_v_l), v);
    gtk_entry_set_text(GTK_ENTRY (entry_2d_v_r), v);

    //guardam la coordenada y
    hline_y = y;
}

//dibuixa les linies verticals i horitzontals 
void Gui::draw_brush_lines (GtkWidget *widget, gdouble x, gdouble y, 
                            gpointer data)
{
    GdkRectangle update_rect;
    update_rect.x = 0;
    update_rect.y = (int)y-5;
    update_rect.width  = widget->allocation.width;
    update_rect.height = 1;
    
    //color
    GdkColor *color;
    GdkGC *gc;
    
    //cream un GC per dibuixar
    gc = gdk_gc_new (widget->window);
    
    //triam el color
    color = (GdkColor*)malloc(sizeof(GdkColor));
    
    int red=255;
    int green = 0;
    int blue = 0;
    color->red   = red   * (65535/255);
    color->green = green * (65535/255);
    color->blue  = blue  * (65535/255);
    
    //calculam el color
    color->pixel = (gulong)(red*65536 + green*256 + blue);
    
    //fem lloc al color
    gdk_color_alloc (gtk_widget_get_colormap (widget), color);
    
    //posam el color de primer pla
    gdk_gc_set_foreground (gc, color);
       
    //linia horitzontal
    gdk_draw_rectangle (pixmapleft, gc, TRUE,
                        update_rect.x, update_rect.y, 
                        update_rect.width, update_rect.height);
    
    //linia horitzontal - imatge dreta (nou)
    gdk_draw_rectangle (pixmapright, gc, TRUE,
                        update_rect.x, update_rect.y,
                        update_rect.width, update_rect.height);
            
    gtk_widget_queue_draw_area (widget, update_rect.x, update_rect.y,
                                update_rect.width, update_rect.height);
        
    //pintam vertical
    update_rect.x = (int)x;
    update_rect.y = 0;
    update_rect.width  = 1;
    update_rect.height = widget->allocation.height;
          
    //esquerra
    gdk_draw_rectangle (pixmapleft, gc, TRUE,
                        update_rect.x, update_rect.y,
                        update_rect.width, update_rect.height);
       
    //dreta
    gdk_draw_rectangle (pixmapright, gc, TRUE,
                        update_rect.x, update_rect.y,
                        update_rect.width, update_rect.height);       
       
    gtk_widget_queue_draw_area (widget, update_rect.x, update_rect.y,
                                update_rect.width, update_rect.height);
}

//esdeveniment d'apretat de botó
gboolean Gui::button_press_event (GtkWidget *widget, GdkEventButton *event,
                                  gpointer  data)
{
    //si és click esquerre i el pixmap existeix, dibuixa el punter
    if (event->button == 1 && pixmapleft != NULL){ 
       draw_brush (widget, event->x, event->y, data);
    }
    return TRUE;
}

//controla el moviment del ratolí sobre les imatges, i actualitza les linies si cal
gboolean Gui::motion_notify_event (GtkWidget *widget, GdkEventMotion *event,
                                   gpointer  data)
{
    int x, y;
    GdkModifierType state;

    if (event->is_hint){
        gdk_window_get_pointer (event->window, &x, &y, &state);
    }else{
        x = (int)event->x;
        y = (int)event->y;
        state = (GdkModifierType) event->state;
    }
    if (state & GDK_BUTTON1_MASK && pixmapleft != NULL){       //alerta
        draw_brush (widget, x, y, data);
    }
    //printf("motion\n");
    return TRUE;
}

//callback dels ajustos, farem servir el paràmetre per sebre de quin es tracta
void Gui::adj_callback (GtkAdjustment *get, gpointer data)
{
    //printf("valor = %f\n", get->value);
    //printf("%s apretat\n", (char*) data);
 
    int i = atoi((char*)data);
    switch (i){          
        //parametres Birchfield/Tomasi
        case 11:{
            mOcv->disparam11 = (int)get->value;
            break;
        }
        case 12:{
            mOcv->disparam12 = (int)get->value;
            break;
        }
        case 13:{
            mOcv->disparam13 = (int)get->value;
            break;
        }
        case 14:{
            mOcv->disparam14 = (int)get->value;
            break;
        }
        case 15:{
            mOcv->disparam15 = (int)get->value;
            break;
        }
        case 16:{
            mOcv->disparam16 = (int)get->value;
            break;
        }          
        
         //parametres Finestra + SAD
        case 21:{
            mOcv->disparam21 = (int)get->value;
            break;
        }
        case 22:{
            mOcv->disparam22 = (int)get->value;
            break;
        }
        case 23:{
            mOcv->disparam23 = (int)get->value;
            break;
        }
        case 24:{
            mOcv->disparam24 = (int)get->value;
            break;
        }      
        case 25:{
            mOcv->disparam25 = (int)get->value;
            break;
        }      
        
        //parametres RSR + CC
        case 31:{
            mOcv->disparam31 = (int)get->value;
            break;
        }
        case 32:{
            mOcv->disparam32 = (int)get->value;
            break;
        }
        case 33:{
            mOcv->disparam33 = (int)get->value;
            break;
        }
        case 34:{
            mOcv->disparam34 = (int)get->value;
            break;
        }      
        case 35:{
            mOcv->disparam35 = (int)get->value;
            break;
        }      

        case -3:{
            int val = (int)get->value;                  //agafam el nou valor
            if ( val == mOcv->mImageStep) {
            }else{
                if (val > mOcv->mImageStep){
                    mOcv->mImageStep = mOcv->mImageStep * 2;
                }else if (val < mOcv->mImageStep){
                    mOcv->mImageStep = mOcv->mImageStep / 2;
                }
                get->value = mOcv->mImageStep;          //redondejam
            }
            break;
        }             
    }
}

//funció d'inactivitat: cridam processament opencv
gboolean Gui::idle (gpointer data, Ocv* ocv)
{
         
    //si no està congelada la imatge, realitzam una nova captura
    mOcv->ocvLoop(freeze);     //el loop actualitza les imatges
   
    //PROFUNDITAT
    //netejam el pixbuf per les linies
    if (pixmapdepth){
       g_object_unref (pixmapdepth);     //l'elimina
    }
    
    //en cream un de nou
    pixmapdepth = gdk_pixmap_new (drawing_area3->window,
                                  drawing_area3->allocation.width,
                                  drawing_area3->allocation.height,
                                  -1);


    //mostram el pixbuf
    if (mOcv->pixbufdepth){
        gdk_draw_pixbuf (pixmapdepth, NULL, mOcv->pixbufdepth, 
                         0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_NORMAL, 0, 0);
    }else{
        g_print("No pixbuf!\n");    
    }

    //hem de crear els pixbuf, i els pixmap a partir dels pixbuf  
        
    //ESQUERRA
    //netejam el pixbuf per les linies
    if (pixmapleft){
       g_object_unref (pixmapleft);     //l'elimina
    }
    
    //en cream un de nou
    pixmapleft = gdk_pixmap_new (drawing_area1->window,
                                 drawing_area1->allocation.width,
                                 drawing_area1->allocation.height,
                                 -1);

    //mostram el pixbuf
    if (mOcv->pixbufleft){
        gdk_draw_pixbuf (pixmapleft, NULL, mOcv->pixbufleft, 
                         0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_NORMAL, 0, 0);
    }else{
        g_print("No pixbuf!\n");    
    }

    //pintam el drawable amb el nou pixmap
    gdk_draw_drawable (drawing_area2->window,
                 drawing_area2->style->fg_gc[GTK_WIDGET_STATE (drawing_area2)],
                 pixmapleft, 0, 0, 0, 0, -1, -1);
    
   
    //DRETA
    //netejam el pixbuf per les linies
    if (pixmapright){
       g_object_unref (pixmapright);     //l'elimina
    }
    
    //en cream un de nou
    pixmapright = gdk_pixmap_new (drawing_area2->window,
                                  drawing_area2->allocation.width,
                                  drawing_area2->allocation.height,
                                  -1);

    //mostram el pixbuf
    if (mOcv->pixbufright){
        gdk_draw_pixbuf(pixmapright, NULL, mOcv->pixbufright, 
                        0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_NORMAL, 0, 0);
    }else{
        g_print("No pixbuf!\n");    
    }

    //pintam els drawable amb els nous pixmaps
    gdk_draw_drawable (drawing_area1->window,
                 drawing_area1->style->fg_gc[GTK_WIDGET_STATE (drawing_area1)],
                 pixmapright, 0, 0, 0, 0, -1, -1);   
    gdk_draw_drawable (drawing_area3->window,
                 drawing_area3->style->fg_gc[GTK_WIDGET_STATE (drawing_area3)],
                 pixmapdepth, 0, 0, 0, 0, -1, -1);
       
    //finalment pintam linies vermelles
    draw_brush_lines (drawing_area1, hline_x_left, hline_y, data);
    draw_brush_lines (drawing_area2, hline_x_right, hline_y, data);    
}

//oculta els widgets (scales) del primer algoritme
void Gui::hide_widgets_alg1 ()
{
    //ocultam tots els altres widgets
    gtk_widget_hide(hscale11);                
    gtk_widget_hide(hscale12);                
    gtk_widget_hide(hscale13);                
    gtk_widget_hide(hscale14);                
    gtk_widget_hide(hscale15);                
    gtk_widget_hide(hscale16);      
}

//oculta els widgets (scales) del segpm algoritme
void Gui::hide_widgets_alg2 ()
{
    //ocultam tots els altres widgets
    gtk_widget_hide(hscale21);                
    gtk_widget_hide(hscale22);                
    gtk_widget_hide(hscale23);                
    gtk_widget_hide(hscale24);                
    gtk_widget_hide(hscale25);                
}

//oculta els widgets (scales) del tercer algoritme
void Gui::hide_widgets_alg3 ()
{
    //ocultam tots els altres widgets
    gtk_widget_hide(hscale31);                
    gtk_widget_hide(hscale32);                
    gtk_widget_hide(hscale33);                
    gtk_widget_hide(hscale34);                
    gtk_widget_hide(hscale35);                
}

//mostra els widgets (scales) del primer algoritme
void Gui::show_widgets_alg1 ()
{
    gtk_widget_show(hscale11);
    gtk_widget_show(hscale12);
    gtk_widget_show(hscale13);
    gtk_widget_show(hscale14);
    gtk_widget_show(hscale15);
    gtk_widget_show(hscale16);
}

//mostra els widgets (scales) del segon algoritme
void Gui::show_widgets_alg2 ()
{
    gtk_widget_show(hscale21);
    gtk_widget_show(hscale22);
    gtk_widget_show(hscale23);
    gtk_widget_show(hscale24);
    gtk_widget_show(hscale25);
}

//mostra els widgets (scales) del tercer algoritme
void Gui::show_widgets_alg3 ()
{
    gtk_widget_show(hscale31);
    gtk_widget_show(hscale32);
    gtk_widget_show(hscale33);
    gtk_widget_show(hscale34);
    gtk_widget_show(hscale35);
}

//callback pels toggle (RADIO BUTTONS)
void Gui::toggle_callback (GtkWidget *widget, gpointer data)
{
    if (GTK_TOGGLE_BUTTON (widget)->active) 
    {
        int i = atoi((char*)data);
        switch(i){
            case 1:{
                mOcv->disalg = DA_BIRCHFIELD_TOMASI;
                hide_widgets_alg2 ();
                hide_widgets_alg3 ();
                //mostram els de l'algoritme 1                        
                show_widgets_alg1 ();                
                break;
            }
            case 2:{
                mOcv->disalg = DA_SAD;
                hide_widgets_alg1 ();
                hide_widgets_alg3 ();
                //mostram els de l'algoritme 2        
                show_widgets_alg2 ();                
                break;
            }
            case 3:{
                mOcv->disalg = DA_RSR;
                hide_widgets_alg1 ();
                hide_widgets_alg2 ();
                //mostram els de l'algoritme 3                        
                show_widgets_alg3 ();                
                break;
            }
        }
    }    
    //actualitzam informació sobre el widget actual
    strcpy (&mRadioButtonAct, (char*)data);
}

//callback cridat en apretar enter dins un entry
void Gui::enter_callback (GtkWidget* widget, gpointer data)
{
    const gchar* t = gtk_entry_get_text (GTK_ENTRY(widget));
    //printf("text: %s\n", (char*)t);
    if (strcmp((char*)data, "3D")==0){
       transform3Dto2D(widget, data);
    }
    if (strcmp((char*)data, "2D")==0){
       transform2Dto3D(widget, data);      
    }
}

//realitza la transformació de 2D a 3D (botó)
gboolean Gui::transform2Dto3D (GtkWidget* widget, gpointer data)
{
   //passam el punt 2D a 3D
    CvPoint3D32f p3dcv;
    CvPoint2D32f pLcv = cvPoint2D32f
                           (atof(gtk_entry_get_text(GTK_ENTRY (entry_2d_u_l))),
                            atof(gtk_entry_get_text(GTK_ENTRY (entry_2d_v_l))));
    CvPoint2D32f pRcv = cvPoint2D32f
                           (atof(gtk_entry_get_text(GTK_ENTRY (entry_2d_u_r))),
                            atof(gtk_entry_get_text(GTK_ENTRY (entry_2d_v_r))));
	//mSv->Reconstruct3dPoint( p3dcv, pLcv, pRcv );
    mSv->stereoToWorld (p3dcv, pLcv, pRcv);
    char x[40];
    sprintf (x, "%f", p3dcv.x);
    char y[40];
    sprintf (y, "%f", p3dcv.y);    	
    char z[40];
    sprintf (z, "%f", p3dcv.z);
	//posam coordenades del punt 3D
	gtk_entry_set_text (GTK_ENTRY (entry_3d_x), x);
	gtk_entry_set_text (GTK_ENTRY (entry_3d_y), y);    	
    gtk_entry_set_text (GTK_ENTRY (entry_3d_z), z);
}

//realitza la transformació 3D a 2D (botó)
gboolean Gui::transform3Dto2D (GtkWidget* widget, gpointer data)
{
	CvPoint2D32f pLcv, pRcv;
    //passam el punt 3D a 2D
    CvPoint3D32f p3dcv = cvPoint3D32f( 
                 atof(gtk_entry_get_text(GTK_ENTRY (entry_3d_x))),
                 atof(gtk_entry_get_text(GTK_ENTRY (entry_3d_y))),
                 atof(gtk_entry_get_text(GTK_ENTRY (entry_3d_z))));

    mSv->worldToStereo (p3dcv, pLcv, pRcv);
    char ul[40];
    sprintf(ul, "%f", pLcv.x);
    char vl[40];
    sprintf(vl, "%f", pLcv.y);
    char ur[40];
    sprintf(ur, "%f", pRcv.x);
    char vr[40];
    sprintf(vr, "%f", pRcv.y);
	//posam coordenades
	gtk_entry_set_text(GTK_ENTRY (entry_2d_u_l), ul);
	gtk_entry_set_text(GTK_ENTRY (entry_2d_v_l), vl);
	gtk_entry_set_text(GTK_ENTRY (entry_2d_u_r), ur);
	gtk_entry_set_text(GTK_ENTRY (entry_2d_v_r), vr);
}

//congela la imatge (botó)
gboolean Gui::freeze_callback (GtkWidget* widget, gpointer data)
{
    if(!freeze){
        gtk_button_set_label (GTK_BUTTON (freezebutton), "Continuar");
    }else{
        gtk_button_set_label (GTK_BUTTON (freezebutton), "Congelar");
    }
    freeze = !freeze;
}

//construeix els noms d'arxiu per a cadascun dels 
void buildScreenName(string &s, string pre, string post)
{
    time_t rawtime;
    time (&rawtime);
    string ct = ctime (&rawtime);
    //adaptam l'hora als caracters permesos
    replace (ct.begin(), ct.end(), ':', '.');
    replace (ct.begin(), ct.end(), ' ', '_');
    //eliminam \n del final
    ct = ct.substr (0, ct.length()-1);    
    s += pre;
    s += ct;
    s += post;
}

//guarda la imatge (botó)
gboolean Gui::save_callback (GtkWidget* widget, gpointer data)
{
    string sleft;
    string sright;
    string sdepth;
    
    buildScreenName(sleft,  "captures/frameLeft_",  ".png");
    buildScreenName(sright, "captures/frameRight_", ".png");
    buildScreenName(sdepth, "captures/frameDepth_", ".png");    

    cvSaveImage(sleft.c_str(),  mOcv->frameLeft);
    cvSaveImage(sright.c_str(), mOcv->frameRight);
    cvSaveImage(sdepth.c_str(), mOcv->depthImage);
}
