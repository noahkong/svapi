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

//arxiu wrapper.cpp

#include "wrapper.h"

extern Gui* gui;

//esdeveniment de configuració dels drawarea
gboolean configure_event (GtkWidget *widget, GdkEventConfigure *event,
                          gpointer  data)
{
    gui->configure_event_drawarea (widget, event, data);                                 
}

//esdeveniment d'exposició: redibuixa
gboolean expose_event    (GtkWidget *widget, GdkEventExpose *event,
                          gpointer  data)
{
  gui->expose_event_drawarea (widget, event, data);     
}

//dibuixat de les línies allà on es fa click, per les imatges dreta i esquerra
void draw_brush       (GtkWidget *widget, gdouble x, gdouble y, gpointer data)
{
    gui->draw_brush (widget, x, y, data);
}

//dibuixa les linies verticals i horitzontals 
void draw_brush_lines (GtkWidget *widget, gdouble x, gdouble y, gpointer data)
{
    gui->draw_brush_lines (widget, x, y, data);                        
}

//esdeveniment de botó apretat
gboolean button_press_event  (GtkWidget *widget, GdkEventButton *event,
                              gpointer  data)
{
    gui->button_press_event (widget, event, data);
}

//controla el moviment del ratolí sobre les imatges, i actualitza les linies si cal
gboolean motion_notify_event (GtkWidget *widget, GdkEventMotion *event,
                              gpointer  data)
{
    gui->motion_notify_event (widget, event, data);
}

//esdeveniment de moviment dels ajustos
void adj_callback    (GtkAdjustment *get, gpointer data)
{
    gui->adj_callback (get, data);
}

//esdeveniment de canvi de radiobutton actiu
void toggle_callback (GtkWidget *widget, gpointer data)
{
    gui->toggle_callback (widget, data);            
}

//callback cridat en apretar enter dins un entry
void enter_callback  (GtkWidget* widget, gpointer data)
{
    gui->enter_callback (widget, data);
}

//realitza la transformació de 2D a 3D
gboolean transform2Dto3D (GtkWidget* widget, gpointer data)
{
    gui->transform2Dto3D (widget, data);
}

//transformació de coordenades 3D a 2D
gboolean transform3Dto2D (GtkWidget* widget, gpointer data)
{
    gui->transform3Dto2D (widget, data);
}

//esdeveniment d'apretat del botó de congelació d'imatge
gboolean freeze_callback (GtkWidget* widget, gpointer data)
{
    gui->freeze_callback (widget, data);
}

//esdeveniment de guardat d'imatges mitjançant el botó
gboolean save_callback   (GtkWidget* widget, gpointer data)
{
    gui->save_callback (widget, data);
}
