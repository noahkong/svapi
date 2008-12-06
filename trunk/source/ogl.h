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

//arxiu ogl.h

#ifndef OGL_H
#define OGL_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

//extensions OpenGL
#include <gtk/gtkgl.h>
#include <gdk/gdkglglext.h>

#ifdef G_OS_WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include "config.h"
#include "ocv.h"
#include "gui.h"

#define MAXGRID 63 //valor constant per definir area de visualitzció OpenGL      

//funcions relacionades amb OpenGL i L'exgensió GtkGlExt per a Gtk+

//alliberació de textura
void freeTexture ();

//generació de la textura
void generateTexture ();

//addició d'una funció de timeout
void timeout_add    (GtkWidget* widget);

//eliminació del timeout
void timeout_remove (GtkWidget* widget);

//activació de la següent animació
void toggle_animation_next (GtkWidget* widget);

//activació de l'animació anterior
void toggle_animation_prev (GtkWidget* widget);

//configuració d'OpenGL
GdkGLConfig* configure_gl ();

//creació del menú popup
GtkWidget* create_popup_menu (GtkWidget* drawing_area);

//dibuixat dels punts a partir de la disparitat
void drawPoints (float max);

//dibuixat de les linies a partir de la disparitat
void drawWireframe (float max);

//obtenció del color a partir de la disparitat, per a representar-la
void getColor (float depth, float &r, float &g, float &b);

//dibuixat dels polígons a partir de la disparitat
void drawPolygons (float max, bool  modetexture);

//callback d'opengl necessari quan es crea el drawingarea
void realize_gl (GtkWidget* widget, gpointer  data);
	 
//callback de configuració del drawing area
gboolean configure_event_gl (GtkWidget* widget, GdkEventConfigure* event,
                             gpointer   data);

//callback d'exposició del drawing area
gboolean expose_event_gl (GtkWidget* widget, GdkEventExpose *event,
                             gpointer   data);

//funció de timeout
gboolean timeout (GtkWidget* widget);

//callback de detecció de moviment sobre l'area OpenGL
gboolean motion_notify_event_gl (GtkWidget* widget, GdkEventMotion* event,
                                 gpointer   data);

//callback d'utilització de botons sobre l'area OpenGL
gboolean button_press_event_gl  (GtkWidget* widget, GdkEventButton* event,
                                 gpointer  data);
		    
//callback d'activació d'un element del popup
gboolean button_press_event_popup_menu (GtkWidget*      widget,
			                            GdkEventButton* event,
			                            gpointer        data);

//callback d'activació de les tecles
gboolean key_press_event (GtkWidget* widget, GdkEventKey* event, gpointer data);

//funció per a la destrucció de l'area OpenGL
void unrealize_gl (GtkWidget* widget, gpointer data);    

//mapejat d'un esdeveniment OpenGL (animació)
gboolean map_event_gl (GtkWidget* widget, GdkEvent* event, gpointer data);

//desmapejat d'un esdeveniment OpenGL (animació)                          	   
gboolean unmap_event_gl (GtkWidget* widget, GdkEvent* event, gpointer data);   

//processament quan el drawing area de dibuixat OpenGL és ocultat
gboolean visibility_notify_event_gl (GtkWidget*          widget,
			                         GdkEventVisibility* event,
			                         gpointer            data);
#endif
