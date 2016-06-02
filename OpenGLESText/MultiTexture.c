#include "esUtil.h"
#include "scrolltext.h"
#include "text_info.h"
#include "window_info.h"

#include <stdlib.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <sys/time.h>

#define WINDOW_HEIGHT win_info->window_height
#define WINDOW_WIDTH win_info->window_width

typedef struct
{
    // Handle to a program object
    GLuint programObject;

    // Attribute locations
    GLint  positionLoc;
    GLint  texCoordLoc;

    // Sampler locations
    GLint baseMapLoc;
    GLint lightMapLoc;

    // Texture handle
    GLuint baseMapTexId;
    GLuint lightMapTexId;

    // Uniform locations
    GLint  mvpLoc;

    // Rotation angle
    GLfloat   angle;

    // MVP matrix
    ESMatrix  mvpMatrix;

} UserData;

int textureWidth;
int textureHeight;

GdkPixbuf *textPixbuf = NULL;
UserData  userData;
GdkWindow *DrawingWindow=NULL;
GtkWidget *da=NULL;
WindowInfo *win_info;

//
// Load texture from disk
//
GLuint LoadTexture (const char *fileName)
{
    int width, height;
    GdkPixbuf *pixbuf = NULL;
    GdkPixbuf *pixbuf_dest = NULL;
    GError * error = NULL;

    char *buffer;// = esLoadTGA ( fileName, &width, &height );

    //   pixbuf = gdk_pixbuf_new_from_file("testn.jpg", &error);
    if (textPixbuf != NULL)
        pixbuf = textPixbuf;
    GLuint texId;
    buffer = (char *)gdk_pixbuf_get_pixels(pixbuf);
    width = gdk_pixbuf_get_width (pixbuf);
    height = gdk_pixbuf_get_height (pixbuf);


    if ( buffer == NULL )
    {
        esLogMessage ( "Error loading (%s) image.\n", fileName );
        return 0;
    }
    textureWidth = width;
    textureHeight = height;
    glGenTextures ( 1, &texId );
    glBindTexture ( GL_TEXTURE_2D, texId );

    glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    //   free ( buffer );

    return texId;
}



///
// Initialize the shader and program object
//
int Init ( ESContext *esContext )
{
    UserData *userData = (UserData *)esContext->userData;
    GLbyte vShaderStr[] =
        "attribute vec4 a_position;   \n"
        "attribute vec2 a_texCoord;   \n"
        "varying vec2 v_texCoord;     \n"
        "uniform mat4 u_mvpMatrix;    \n"
        "void main()                  \n"
        "{                            \n"
        "   gl_Position = u_mvpMatrix * a_position; \n"
        "   v_texCoord = a_texCoord;  \n"
        "}                            \n";

    GLbyte fShaderStr[] =
        "precision mediump float;                            \n"
        "varying vec2 v_texCoord;                            \n"
        "uniform sampler2D s_baseMap;                        \n"
        "uniform sampler2D s_lightMap;                       \n"
        "void main()                                         \n"
        "{                                                   \n"
        "  vec4 baseColor;                                   \n"
        "  vec4 lightColor;                                  \n"
        "                                                    \n"
        "  baseColor = texture2D( s_baseMap, v_texCoord );   \n"
        "  lightColor = texture2D( s_lightMap, v_texCoord ); \n"
        "  gl_FragColor = baseColor * (lightColor + 0.25);   \n"
        "}                                                   \n";

    // Load the shaders and get a linked program object
    userData->programObject = esLoadProgram ((const char *) vShaderStr, (const char *)fShaderStr );

    // Get the attribute locations
    userData->positionLoc = glGetAttribLocation ( userData->programObject, "a_position" );
    userData->texCoordLoc = glGetAttribLocation ( userData->programObject, "a_texCoord" );
    // Get the sampler location
    userData->baseMapLoc = glGetUniformLocation ( userData->programObject, "s_baseMap" );
    userData->lightMapLoc = glGetUniformLocation ( userData->programObject, "s_lightMap" );

    // Load the textures
    userData->lightMapTexId = LoadTexture ( "test1.jpg" );
    userData->baseMapTexId = LoadTexture ( "test1.jpg" );

    if ( userData->baseMapTexId == 0 || userData->lightMapTexId == 0 )
        return FALSE;

    userData->mvpLoc = glGetUniformLocation( userData->programObject, "u_mvpMatrix" );
    userData->angle = 45.0f;
    glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
    return TRUE;
}

void Update ( ESContext *esContext, float deltaTime )
{
    UserData *userData = (UserData*) esContext->userData;
    ESMatrix perspective;
    ESMatrix modelview;
    static float step = 1 + (float) textureWidth / WINDOW_WIDTH;
    float stepUnit;

    if (esContext->width != 0) {
        stepUnit = 0.005 * 1920 / esContext->width;
    } else {
        stepUnit = 0.005;
    }
    step -= (0.005 * 1920) / esContext->width;
    if (step < -(1 + (float) textureWidth / WINDOW_WIDTH))
        step = 1 + (float) textureWidth / WINDOW_WIDTH;


    // Generate a perspective matrix with a 60 degree FOV
    esMatrixLoadIdentity( &perspective );
    float left = (float) WINDOW_WIDTH / textureWidth / 2 ;
    float top = (float) WINDOW_HEIGHT / textureHeight / 2;
    left = 1;
    top = 1;
    esOrtho( &perspective, -left, left, -top, top, -2, 2);

    // Generate a model view matrix to rotate/translate the cube
    esMatrixLoadIdentity( &modelview );

    // Translate away from the viewer
    esTranslate( &modelview, step, 0.0, -2.0 );

    // Compute the final MVP by multiplying the
    // modevleiw and perspective matrices together
    esMatrixMultiply( &userData->mvpMatrix, &modelview, &perspective );
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw ( ESContext *esContext )
{
    UserData *userData = (UserData *)esContext->userData;
    float left = (float) textureWidth / WINDOW_WIDTH;
    float top = (float) textureHeight / WINDOW_HEIGHT;
    GLfloat vVertices[] = {  -left,  top, 0.0f,  // Position 0
        0.0f,  0.0f,        // TexCoord 0
        -left,  -top, 0.0f,  // Position 1
        0.0f,  1.0f,        // TexCoord 1
        left,   -top, 0.0f,  // Position 2
        1.0f,  1.0f,        // TexCoord 2
        left,   top, 0.0f,  // Position 3
        1.0f,  0.0f         // TexCoord 3
    };
    GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

    // Set the viewport
    glViewport ( 0, 0, esContext->width, esContext->height );
    // Clear the color buffer
    glClear ( GL_COLOR_BUFFER_BIT );

    // Use the program object
    glUseProgram ( userData->programObject );

    // Load the vertex position
    glVertexAttribPointer ( userData->positionLoc, 3, GL_FLOAT,
            GL_FALSE, 5 * sizeof(GLfloat), vVertices );
    // Load the texture coordinate
    glVertexAttribPointer ( userData->texCoordLoc, 2, GL_FLOAT,
            GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3] );

    glEnableVertexAttribArray ( userData->positionLoc );
    glEnableVertexAttribArray ( userData->texCoordLoc );

    // Bind the base map
    glActiveTexture ( GL_TEXTURE0 );
    glBindTexture ( GL_TEXTURE_2D, userData->baseMapTexId );

    // Set the base map sampler to texture unit to 0
    glUniform1i ( userData->baseMapLoc, 0 );

    // Bind the light map
    glActiveTexture ( GL_TEXTURE1 );
    glBindTexture ( GL_TEXTURE_2D, userData->lightMapTexId );

    // Set the light map sampler to texture unit 1
    glUniform1i ( userData->lightMapLoc, 1 );

    glUniformMatrix4fv( userData->mvpLoc, 1, GL_FALSE, (GLfloat*) &userData->mvpMatrix.m[0][0] );
    glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );
    Update(esContext, 0.0);
}

///
// Cleanup
//
void ShutDown ( ESContext *esContext )
{
    UserData *userData = (UserData *)esContext->userData;

    // Delete texture object
    glDeleteTextures ( 1, &userData->baseMapTexId );
    glDeleteTextures ( 1, &userData->lightMapTexId );

    // Delete program object
    glDeleteProgram ( userData->programObject );
}

gboolean idle_callback(gpointer data) {
    static struct timeval t1, t2;
    static struct timezone tz;
    float deltatime;
    static float totaltime = 0.0f;
    static unsigned int frames = 0;

    ESContext *esContext = (ESContext*) data;
    static int flag = 0;
    if (flag == 0) {
        DrawingWindow = gtk_widget_get_window(GTK_WIDGET(da));
        esCreateWindow ( esContext, "DDD", WINDOW_WIDTH, WINDOW_HEIGHT, ES_WINDOW_RGB );
        if ( !Init ( esContext ) )
            return 0;
        flag = 1;
    }

    gettimeofday ( &t1 ,NULL );
    Draw(esContext);
    eglSwapBuffers(esContext->eglDisplay, esContext->eglSurface);

    // count the frame rate
    int count = 0;
    gettimeofday(&t2,  NULL);
    deltatime = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6);
    t1 = t2;

    totaltime += deltatime;
    frames++;
    if (totaltime >  2.0f)
    {
        printf("%4d frames rendered in %1.4f seconds -> FPS=%3.4f\n", frames, totaltime, frames/totaltime);
        totaltime  = 0;
        frames = 0;
    }

    return TRUE;
}

int main ( int argc, char *argv[] )
{
    gtk_init(&argc, &argv);

    static GtkWidget *window=NULL;
    DrawingWindow = NULL;
    TextInfo *text_info = new TextInfo;
    win_info = new WindowInfo;
    win_info->set_info(argc, argv);

    if (win_info->window_id > 0)
        window = gtk_plug_new(win_info->window_id);
    else
        window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    gtk_widget_set_size_request(window, win_info->window_width, win_info->window_height);
    da = gtk_drawing_area_new();

    gtk_container_add(GTK_CONTAINER(window), da);
    gtk_widget_show_all(window);

    GdkColor text_color;
    GdkColor background_color;

    text_info->set_info(argc, argv);

    // ARGB to RGBA
    background_color.pixel =  win_info->background_color << 8 | ((win_info->background_color >> 24) & 0xff);

    // 2Byte to store color
    text_color.red = ((text_info->text_color >> 16) & 0xff) << 8;
    text_color.green = ((text_info->text_color >> 8) & 0xff) << 8;
    text_color.blue = (text_info->text_color & 0xff) << 8;

    ScrollText *scroll_text = new ScrollText();
    scroll_text->set_align(text_info->align);
    scroll_text->set_direction(text_info->direction);
    scroll_text->set_rate(text_info->rate);
    scroll_text->set_background(NULL, &background_color);
    scroll_text->set_text_style(text_info->text_size, text_info->text_font, &text_color);
    scroll_text->set_text(win_info->content);
    textPixbuf = scroll_text->get_text_pixbuf();

    ESContext esContext;
    esInitContext ( &esContext );
    esContext.userData = &userData;
    eglSwapInterval(esContext.eglDisplay, 1);
    g_idle_add(idle_callback, &esContext);
    gtk_main();
    ShutDown ( &esContext );
}
