#ifndef __SCROLL_TEXT_H
#define __SCROLL_TEXT_H
#include <GLES2/gl2.h>
#include <EGL/egl.h>

//#include <GL/gl.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

using namespace std;

#define MAX_TEXTURE_SIZE 255

typedef struct tagScrollLayout
{
	bool meet_end;
	int text_width;
	int text_height;
	double draw_x;
	double draw_y;
	GLuint texture;
} ScrollLayout;


typedef struct tagTextureBuffer
{
	GLuint texture[MAX_TEXTURE_SIZE+1];
	bool in_used[MAX_TEXTURE_SIZE];
} TextureBuffer;

class ScrollText
{
	public:
		ScrollText();
		virtual ~ScrollText();
		bool init(GtkWidget *);
		bool run();

		void set_align(int align) {m_align = align;}
		void set_direction(int direction) {m_direction = direction;}
		void set_rate(double rate) {m_rate = rate;}
		void set_background(const char *image, GdkColor *background_color);
		void set_text_style(int font_size, char *font_name, GdkColor *text_color);
		void set_text(const char *text);                

		void clear_texture_buffer();
		void clear_window();

		void move_and_redraw();
		void scroll_text_at_idle();
		void update_text_at_idle();
		GdkPixbuf* get_text_pixbuf();
		unsigned char * render_text_pixles(int *width, int *height);	
	private:
		void init_texture_buffer();
		void init_opengl();
		GLuint get_texture();
		void set_texture_available(GLuint id);
		
		void add_layout_to_list(GdkPixbuf *pixbuf, int start_x, int start_y, int width, int height);

		GLuint bind_background_texture();
		void redraw_background();
		
		GLuint bind_text_texture(GdkPixbuf *pixbuf);
		void draw_text_layout();

		void redraw_text(GLuint texture, double move, double left, double top);
		

		void move_text();
		void redraw();

	private:
		int m_align;
		int m_direction;
		double m_rate;

		int m_height;
		int m_width;

		GtkWidget * m_draw_area;

		guint m_startplay_idle_id;
		guint m_play_idle_id;
		guint m_timeid;

		char * m_background_image;
		GdkColor m_backgound_color;

		char * m_scroll_text;
		GdkColor m_text_color;

		PangoContext            *m_context;
		PangoFontDescription    *m_font_desc;
		vector <ScrollLayout>   m_layout_vec;
		TextureBuffer 		m_texture_buffer;
};
#endif
