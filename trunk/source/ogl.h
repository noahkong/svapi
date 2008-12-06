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

#define MAXGRID 63 //valor constant per definir area de visualitzci� OpenGL      

//funcions relacionades amb OpenGL i L'exgensi� GtkGlExt per a Gtk+

//alliberaci� de textura
void freeTexture ();

//generaci� de la textura
void generateTexture ();

//addici� d'una funci� de timeout
void timeout_add    (GtkWidget* widget);

//eliminaci� del timeout
void timeout_remove (GtkWidget* widget);

//activaci� de la seg�ent animaci�
void toggle_animation_next (GtkWidget* widget);

//activaci� de l'animaci� anterior
void toggle_animation_prev (GtkWidget* widget);

//configuraci� d'OpenGL
GdkGLConfig* configure_gl ();

//creaci� del men� popup
GtkWidget* create_popup_menu (GtkWidget* drawing_area);

//dibuixat dels punts a partir de la disparitat
void drawPoints (float max);

//dibuixat de les linies a partir de la disparitat
void drawWireframe (float max);

//obtenci� del color a partir de la disparitat, per a representar-la
void getColor (float depth, float &r, float &g, float &b);

//dibuixat dels pol�gons a partir de la disparitat
void drawPolygons (float max, bool  modetexture);

//callback d'opengl necessari quan es crea el drawingarea
void realize_gl (GtkWidget* widget, gpointer  data);
	 
//callback de configuraci� del drawing area
gboolean configure_event_gl (GtkWidget* widget, GdkEventConfigure* event,
                             gpointer   data);

//callback d'exposici� del drawing area
gboolean expose_event_gl (GtkWidget* widget, GdkEventExpose *event,
                             gpointer   data);

//funci� de timeout
gboolean timeout (GtkWidget* widget);

//callback de detecci� de moviment sobre l'area OpenGL
gboolean motion_notify_event_gl (GtkWidget* widget, GdkEventMotion* event,
                                 gpointer   data);

//callback d'utilitzaci� de botons sobre l'area OpenGL
gboolean button_press_event_gl  (GtkWidget* widget, GdkEventButton* event,
                                 gpointer  data);
		    
//callback d'activaci� d'un element del popup
gboolean button_press_event_popup_menu (GtkWidget*      widget,
			                            GdkEventButton* event,
			                            gpointer        data);

//callback d'activaci� de les tecles
gboolean key_press_event (GtkWidget* widget, GdkEventKey* event, gpointer data);

//funci� per a la destrucci� de l'area OpenGL
void unrealize_gl (GtkWidget* widget, gpointer data);    

//mapejat d'un esdeveniment OpenGL (animaci�)
gboolean map_event_gl (GtkWidget* widget, GdkEvent* event, gpointer data);

//desmapejat d'un esdeveniment OpenGL (animaci�)                          	   
gboolean unmap_event_gl (GtkWidget* widget, GdkEvent* event, gpointer data);   

//processament quan el drawing area de dibuixat OpenGL �s ocultat
gboolean visibility_notify_event_gl (GtkWidget*          widget,
			                         GdkEventVisibility* event,
			                         gpointer            data);
#endif
