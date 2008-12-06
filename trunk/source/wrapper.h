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

//arxiu wrapper.h

#ifndef WRAPPER_H
#define WRAPPER_H
#include "gui.h"

//esdeveniment de configuració
gboolean configure_event (GtkWidget *widget, GdkEventConfigure *event,
                          gpointer  data);

//esdeveniment d'exposició: redibuixa
gboolean expose_event    (GtkWidget *widget, GdkEventExpose *event,
                          gpointer  data);

//dibuixat de les linies
void draw_brush (GtkWidget *widget, gdouble x, gdouble y, gpointer data);

//dibuixa les linies verticals i horitzontals 
void draw_brush_lines (GtkWidget *widget, gdouble x, gdouble y, gpointer data);

//esdeveniment d'apretat de botó
gboolean button_press_event  (GtkWidget *widget, GdkEventButton *event,
                              gpointer  data);
                                    
//controla el moviment del ratolí sobre les imatges, i actualitza les linies si cal
gboolean motion_notify_event (GtkWidget *widget, GdkEventMotion *event,
                              gpointer  data);

//callback de les barres d'ajustament
void adj_callback    (GtkAdjustment *get, gpointer data);

//callback d'apretat dels botons radio
void toggle_callback (GtkWidget *widget, gpointer data);

//callback cridat en apretar enter dins un entry
void enter_callback  (GtkWidget* widget, gpointer data);

//realitza la transformació de 2D a 3D
gboolean transform2Dto3D (GtkWidget* widget, gpointer data);

//realitza la transformació de 3D a 2D
gboolean transform3Dto2D (GtkWidget* widget, gpointer data);

//callback del botó de congelament de la imatge
gboolean freeze_callback (GtkWidget* widget, gpointer data);

//callback del botó de guardat de la imatge
gboolean save_callback   (GtkWidget* widget, gpointer data);

#endif
