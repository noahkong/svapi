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

//arxiu ogl.cpp

#include "ogl.h"

#define SCALE config->getDefGridScale()
#define SCALE_DIV 1.0/SCALE
#define DEF_IMAGE_STEP 2

extern Config* config;
extern Ocv* ocv;
extern Gui* gui;

//variables estàtiques globals
static gboolean animate = TRUE;

static int grid = (MAXGRID/2);
static int beginX, beginY;

static float dt = 0.008;
static float sphi = -90.0;
static float stheta = 45.0;
static float sdepth = 5.0/4.0 * (MAXGRID/2);
static float zNear = (MAXGRID/2)/100.0;
static float zFar = (MAXGRID/2)*3.0;
static float aspect = 5.0/4.0;

static float lightPosition[4] = {0.0, 0.0, 1.0, 1.0};

GLuint mTexture;        //punter de la textura OpenGL 
     
static const float def_low_color[3] = {0.0, 0.0, 0.5};
static const float def_high_color[3] = {1.0, 1.0, 1.0};

static guint timeout_id = 0;         //identificador del timeout

//dibuixat de punts a partir del mapa de disparitat
void drawPoints (float max)
{
    glColor3f(1.0, 1.0, 1.0);
    float depth = 0.0;
    for(int i=0; i <= ocv->depthImageRGB->width; i = i + ocv->mImageStep){
          glBegin(GL_POINTS);
          for(int j=0; j<=ocv->depthImageRGB->height; j = j + ocv->mImageStep){
              depth = ocv->getDepth(i, j);
              glColor3f (depth / config->getDefGridMax(), 
                         depth / config->getDefGridMax(), 
                         1.0);
              glVertex3f ((float)j/config->getDefGridScale(),
                  (float)i/config->getDefGridScale(), 
                  (depth/config->getDefGridMax()) * config->getDefGridScale()); 
          }
          glEnd();
    }
}

//dibuixat del wireframe
void drawWireframe (float max)
{
    glColor3f(1.0, 1.0, 1.0);    
    float depth = 0.0;
    for(int i=0; i<=ocv->depthImageRGB->width; i = i + ocv->mImageStep){
          glBegin(GL_LINE_STRIP);
          for(int j=0; j<=ocv->depthImageRGB->height; j = j + ocv->mImageStep){
              depth = ocv->getDepth(i, j);
              glColor3f (depth / config->getDefGridMax(), 
                         depth / config->getDefGridMax(),
                         1.0);
              glVertex3f ((float)j/config->getDefGridScale(),
                  (float)i/config->getDefGridScale(), 
                  (depth/config->getDefGridMax()) * config->getDefGridScale()); 
          }
          glEnd();
    }
    for(int j=0; j<=ocv->depthImageRGB->height; j = j + ocv->mImageStep){
        for(int i=0; i<=ocv->depthImageRGB->width; i = i + ocv->mImageStep){
          glBegin(GL_LINE_STRIP);
              depth = ocv->getDepth(i, j);
              glColor3f (depth/config->getDefGridMax(), 
                         depth/config->getDefGridMax(), 
                         1.0);
              glVertex3f ((float)j/config->getDefGridScale(),
                  (float)i/config->getDefGridScale(), 
                  (depth/config->getDefGridMax()) * config->getDefGridScale()); 
          }
          glEnd();
    }
}

//càlcul del color per a la representació OpenGL de la disparitat
void getColor (float depth, float &r, float &g, float &b)
{
    //fem servir depth com si fos un percentatge
    r = def_low_color[0] + depth*(def_high_color[0]- def_low_color[0]);
    g = def_low_color[1] + depth*(def_high_color[1]- def_low_color[1]);
    b = def_low_color[2] + depth*(def_high_color[2]- def_low_color[2]);
}

//generació de la textura per a l'OpenGL
void generateTexture () 
{
	glEnable(GL_TEXTURE_2D);
	glGenTextures( 1, &mTexture );
	glBindTexture(GL_TEXTURE_2D, mTexture);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3,
                      ocv->getTexture()->width, ocv->getTexture()->height,               
					  GL_RGB, GL_UNSIGNED_BYTE, ocv->getTexture()->imageData);            
	glTexParameteri(GL_TEXTURE_2D, 
                    GL_TEXTURE_MIN_FILTER,
					GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,             
					GL_TEXTURE_MAG_FILTER,     
					GL_LINEAR_MIPMAP_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisable(GL_TEXTURE_2D);
}

//alliberació de memòria (textura)
void freeTexture ()
{
     glDeleteTextures (1, &mTexture);
}

//nova funció per dibuixar polígons
void drawPolygons (float max, bool modetexture)
{
    bool draw =  true;
    static float scale = 20.0;
    float depth = 0.0;
    float depth4 = 0.0;
    int i=0, j=0;
    if (max<=0){
        max = config->getDefGridMax();
    }
    glColor3f (1.0, 1.0, 1.0);
    if (modetexture){
        glEnable(GL_TEXTURE_2D);
        glBindTexture( GL_TEXTURE_2D, mTexture);
    }
    for(int j=0; j<=ocv->depthImageRGB->height-ocv->mImageStep;  
        j = j + ocv->mImageStep){
        
        glBegin(GL_TRIANGLE_STRIP);
        draw =  true;
        for(int i=0; i<=ocv->depthImageRGB->width; i = i + ocv->mImageStep){
           depth = ocv->getDepth(i, j);
           depth4 =  ocv->getDepth(i, j+ ocv->mImageStep);
           if (modetexture){
               glTexCoord2f ((1.0/ocv->depthImageRGB->width)*i, 
                             (1.0/ocv->depthImageRGB->height)*j);
           }else{
               glColor3f (depth/config->getDefGridMax(), 
                          depth/config->getDefGridMax(), 
                          1.0);
           }
           glVertex3f 
               ((float)j/config->getDefGridScale(),
                (float)i/config->getDefGridScale(), 
                (depth/config->getDefGridMax())*config->getDefGridScale());
           if (modetexture){
                glTexCoord2f
                    ((1.0/ocv->depthImageRGB->width)*i, 
                    (1.0/ocv->depthImageRGB->height)*(j+ocv->mImageStep));
           }else{
               glColor3f(depth4/max, depth4/max, 1.0);
           }
           glVertex3f 
               ((float)(j+ocv->mImageStep)/config->getDefGridScale(),
                (float)i/config->getDefGridScale(), 
                (depth4/config->getDefGridMax())*config->getDefGridScale());  
        }
        glEnd();
    }
    if (modetexture){
        glDisable(GL_TEXTURE_2D);
    }
}

//realize: inicialització d'OpenGL
void realize_gl (GtkWidget *widget, gpointer data)
{
    GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
    
    GdkGLProc proc = NULL;
    
    //inici OpenGL
    if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
    return;
    
    //glPolygonOffsetEXT
    proc = gdk_gl_get_glPolygonOffsetEXT ();
    if (proc == NULL){
        //glPolygonOffset
        proc = gdk_gl_get_proc_address ("glPolygonOffset");
        if (proc == NULL)
        {
            g_print ("Sorry, glPolygonOffset() is not supported by this renderer.\n");
            exit (1);
        }
    }
    
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);
    glClearColor (0.0, 0.0, 0.0, 0.0);
    gdk_gl_glPolygonOffsetEXT (proc, 1.0, 1.0);
    glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable (GL_COLOR_MATERIAL);
    glColorMaterial (GL_FRONT, GL_DIFFUSE);
    glLightfv (GL_LIGHT0, GL_POSITION, lightPosition);
    glEnable (GL_LIGHT0);
    glShadeModel(GL_SMOOTH);  
    glDisable (GL_LIGHTING);
    
    gdk_gl_drawable_gl_end (gldrawable);
    //fi OpenGL
    
    return;
}

//processament de la reconfiguració de l'area
gboolean configure_event_gl (GtkWidget *widget, GdkEventConfigure *event,
		                     gpointer  data)
{
    
    GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
    
    GLfloat w = widget->allocation.width;
    GLfloat h = widget->allocation.height;
    
    //inici OpenGL
    if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext)) return FALSE;
    
    aspect = (float)w/(float)h;
    glViewport (0, 0, (GLsizei)w, (GLsizei)h);
    
    gdk_gl_drawable_gl_end (gldrawable);
    //fi OpenGL
    
    return TRUE;
}

//esdeveniment d'exposició, cridat cada cop que s'ha de redibuixar l'area
gboolean expose_event_gl (GtkWidget *widget, GdkEventExpose *event, 
                          gpointer data)
{
    generateTexture();   //regeneram la textura   
    
    //printf("expose event gl\n");
    GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);
    
    //inici OpenGL
    if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext)) return FALSE;
    
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluPerspective (64.0, aspect, zNear, zFar);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    
    glTranslatef (0.0,0.0,-sdepth);
    glRotatef (-stheta, 1.0, 0.0, 0.0);
    glRotatef (sphi, 0.0, 0.0, 1.0);
    glTranslatef 
            (-(float)((ocv->depthImageRGB->height/config->getDefGridScale())/2), 
             -(float)((ocv->depthImageRGB->width/config->getDefGridScale())/2), 
             0.0);
    
    switch (gui->mode3d){
        case MDE_WIREFRAME:{
            drawWireframe(config->getDefGridMax());
            break;
        }
        case MDE_POINTS:{
            drawPoints(config->getDefGridMax());
            break;
        }
        case MDE_POLYGON:{
            drawPolygons(config->getDefGridMax(), false);
            break;
        }
        case MDE_POLYTEX:{
            drawPolygons(config->getDefGridMax(), true);
            break;
        }
    }

    //canviam buffers
    if (gdk_gl_drawable_is_double_buffered (gldrawable))
        gdk_gl_drawable_swap_buffers (gldrawable);
    else
        glFlush ();

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    gdk_gl_drawable_gl_end (gldrawable);
    //fi OpenGL
    
    freeTexture();  //eliminam la textura
    
    return TRUE;
}

//funció de timeout, actualitza el frame de manera continua
gboolean timeout (GtkWidget *widget)
{
    expose_event_gl (gui->drawing_area_gl, NULL, NULL);
    
    //invalidam tota la finestra
    gdk_window_invalidate_rect (widget->window, &widget->allocation, FALSE);
    
    //actualitzam síncronament
    gdk_window_process_updates (widget->window, FALSE);
    
    return TRUE;
}

//callback pel moviment sobre l'area OpenGL
gboolean motion_notify_event_gl (GtkWidget *widget, GdkEventMotion *event,
		                         gpointer  data)
{
    gboolean redraw = FALSE;
    
    if (event->state & GDK_BUTTON1_MASK)
    {
        sphi += (float)(event->x - beginX) / 4.0;
        stheta += (float)(beginY - event->y) / 4.0;
        redraw = TRUE;
    }
    
    if (event->state & GDK_BUTTON2_MASK)
    {
        sdepth -= ((event->y - beginY)/(widget->allocation.height))*(MAXGRID/2);
        redraw = TRUE;
    }
    
    beginX = (int)event->x;
    beginY = (int)event->y;
    
    if (redraw && !animate)
        gdk_window_invalidate_rect (widget->window, &widget->allocation, FALSE);
    
    return TRUE;
}

//processament dels botons de ratolí
gboolean button_press_event_gl (GtkWidget      *widget,
                    		    GdkEventButton *event,
                          	    gpointer       data)
{
    if (event->button == 1)
    {
        beginX = (int)event->x;
        beginY = (int)event->y;
        return TRUE;
    }
    
    if (event->button == 2)
    {
        beginX = (int)event->x;
        beginY = (int)event->y;
        return TRUE;
    }
    
    return FALSE;
}

//callback de botons per al menú popup
gboolean button_press_event_popup_menu (GtkWidget      *widget, 
                                        GdkEventButton *event,
                                        gpointer       data)
{
  if (event->button == 3)
    {
      gtk_menu_popup (GTK_MENU(widget), NULL, NULL, NULL, NULL,
		              event->button, event->time);
      return TRUE;
    }
  return FALSE;
}

//processament de les tecles
gboolean key_press_event (GtkWidget *widget, GdkEventKey *event, gpointer data)
{   
    switch (event->keyval){
        case GDK_w:{
            sdepth += 2.0;
            break;
        }      
        case GDK_s:{
            sdepth -= 2.0;
            break;
        }
        case GDK_plus:{
            if (ocv->mImageStep<ocv->mMaxStep){
                ocv->mImageStep = ocv->mImageStep*2;           
            }
            break;     
        }  
        case GDK_Escape:{
            gtk_main_quit ();
            break;
        }
        default:{
            return FALSE;
        }
    }
    
    if (!animate)
        gdk_window_invalidate_rect (widget->window, &widget->allocation, FALSE);

    return TRUE;
}

//processament necessari quan es destrueix l'area opengl
void unrealize_gl (GtkWidget *widget, gpointer  data)
{
}

//addició del timeout
void timeout_add (GtkWidget *widget)
{
    if (timeout_id == 0){
        timeout_id = gtk_timeout_add (config->getDefTimeout3D(),
                                      (GtkFunction) timeout,
                                      widget);
    }
}

//eliminació del timeout
void timeout_remove (GtkWidget *widget)
{
    if (timeout_id != 0){
        gtk_timeout_remove (timeout_id);
        timeout_id = 0;
    }
}


//processament necessari quan l'area opengl és "mapejada"
gboolean map_event_gl (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    if (animate) timeout_add (widget);
    return TRUE;
}

//processament quan l'area opengl és "desmapejada"
gboolean unmap_event_gl (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    timeout_remove (widget);
    return TRUE;
}

//processament quan el drawing area de dibuixat OpenGL és ocultat
gboolean visibility_notify_event_gl (GtkWidget          *widget, 
                                     GdkEventVisibility *event,
			                         gpointer           data)
{
    if (animate){
        if (event->state == GDK_VISIBILITY_FULLY_OBSCURED)
            timeout_remove (widget);
        else
            timeout_add (widget);
    }
    return TRUE;
}

//canvi d'animació
void toggle_animation_next (GtkWidget *widget)
{
    gui->mode3d = (t_mode_3d)(((int)gui->mode3d + 1)%NUM_MODES);
}

void toggle_animation_prev (GtkWidget *widget)
{
    gui->mode3d = (t_mode_3d)(((int)gui->mode3d - 1 + NUM_MODES)%NUM_MODES);
}

//construcció de la GUI OpenGL
//creació del menú Popup
GtkWidget* create_popup_menu (GtkWidget *drawing_area)
{
    GtkWidget *menu;
    GtkWidget *menu_item;
    
    menu = gtk_menu_new ();
    
    //canvi de tipus d'animació 3D - mode seguent
    menu_item = gtk_menu_item_new_with_label ("Mode 3D seguent");
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
    g_signal_connect_swapped (G_OBJECT (menu_item), "activate",
                              G_CALLBACK (toggle_animation_next), 
                              drawing_area);
    gtk_widget_show (menu_item);
    
    //canvi de tipus d'aminació 3D - mode anterior
    menu_item = gtk_menu_item_new_with_label ("Mode 3D anterior");
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
    g_signal_connect_swapped (G_OBJECT (menu_item), "activate",
        		G_CALLBACK (toggle_animation_prev), drawing_area);
    gtk_widget_show (menu_item);
    
    return menu;
}

//utilitats
//configuram el frame buffer OpenGL
GdkGLConfig* configure_gl ()
{
    GdkGLConfig *glconfig;
    //intentam visualitzar amb doble buffer
    glconfig = gdk_gl_config_new_by_mode (static_cast<GdkGLConfigMode>
                                         (GDK_GL_MODE_RGB    |
                                          GDK_GL_MODE_DEPTH  |
                                          GDK_GL_MODE_DOUBLE));
    if (glconfig == NULL){
        g_print ("\n*** Cannot find the double-buffered visual.\n");
        g_print ("\n*** Trying single-buffered visual.\n");
        //intentam visualitzar amb un sol buffer
        glconfig = gdk_gl_config_new_by_mode (static_cast<GdkGLConfigMode>
                                             (GDK_GL_MODE_RGB   |
                                              GDK_GL_MODE_DEPTH));
        if (glconfig == NULL){
            g_print ("*** No appropriate OpenGL-capable visual found.\n");
            exit (1);
        }
    }
    return glconfig;
}
