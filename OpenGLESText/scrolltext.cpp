#include "scrolltext.h"

static bool time_handler(gpointer data);
typedef int ( * PFNGLXSWAPINTERVALSGIPROC) (int interval);
//PFNGLXSWAPINTERVALSGIPROC swapInterval;

#define MAX_TEXT_LENGTH    1920

ScrollText::ScrollText():
	m_align(0),
	m_direction(0),
	m_rate(0),
	m_width(0),
	m_height(0),
	m_draw_area(NULL),
	m_background_image(NULL),
	m_scroll_text(NULL),
	m_context(NULL),
	m_font_desc(NULL),
	m_startplay_idle_id(0),
	m_play_idle_id(0),
	m_timeid(0)
{
	m_layout_vec.clear();
//	swapInterval = (PFNGLXSWAPINTERVALSGIPROC)glXGetProcAddress("glXSwapIntervalSGI");

}

ScrollText::~ScrollText()
{

	if (m_timeid) {
		g_source_remove(m_timeid);
		m_timeid = 0;
	}
	if (m_startplay_idle_id) {
		g_source_remove(m_startplay_idle_id);
		m_startplay_idle_id = 0;
	}
	if (m_play_idle_id) {
		g_source_remove(m_play_idle_id);
		m_play_idle_id = 0;
	}
	if (m_context) {
		g_object_unref(m_context);
		m_context = NULL;
	}
	if (m_font_desc) {
		pango_font_description_free(m_font_desc);
		m_font_desc = NULL;
	}
	if (m_background_image) {
		delete[] m_background_image;
		m_background_image = NULL;
	}
	if (m_scroll_text) {
		delete[]  m_scroll_text;
		m_scroll_text = NULL;
	}

	m_layout_vec.clear();

	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(MAX_TEXTURE_SIZE+1, m_texture_buffer.texture);
}
static bool time_handler(gpointer user_data)
{
	ScrollText *instance = (ScrollText*) user_data;
	instance->scroll_text_at_idle();
	return true;
}

static gboolean g_callback_playscrolltxt(gpointer user_data)
{
	ScrollText *instance = (ScrollText*) user_data;
	return false;
}

static gboolean g_callback_draw_text_at_idle(gpointer user_data)
{
	ScrollText *instance = (ScrollText*) user_data;
	instance->move_and_redraw();
	return false;
}
bool ScrollText::init(GtkWidget* draw_area)
{
	if (draw_area == NULL) {
		printf("Cannot get draw area\n");
		return false;
	}

	m_draw_area = draw_area;
	gtk_widget_get_size_request(m_draw_area, &m_width, &m_height);

	if (m_width == 0) {
		printf("Cannot get window size\n");
		return false;
	}
	m_rate = m_rate * 2 / m_width;
	m_startplay_idle_id = g_idle_add(g_callback_playscrolltxt, this);
	return true;
}

bool ScrollText::run()
{
	m_startplay_idle_id = g_idle_add(g_callback_draw_text_at_idle, this);
	return true;
}

void ScrollText::set_background(const char *image, GdkColor *background_color)
{
	if (image != NULL) {
		m_background_image = new char [strlen(image) + 1];
		strcpy(m_background_image, image);
	}
	m_backgound_color = *background_color;
}

void ScrollText::set_text_style(int font_size, char *font_name, GdkColor *text_color)
{
	m_text_color = *text_color;
	m_font_desc = pango_font_description_from_string(font_name);
	pango_font_description_set_size(m_font_desc, font_size*PANGO_SCALE);
	m_context = gdk_pango_context_get();
	pango_context_set_font_description(m_context, m_font_desc);
}

inline cairo_t* create_cairo_context (int width, int height, int channels,
		cairo_surface_t** surf, unsigned char** buffer)
{
	*buffer = (unsigned char *)calloc (channels * width * height,
			sizeof (unsigned char));
	*surf = cairo_image_surface_create_for_data (*buffer,
			CAIRO_FORMAT_ARGB32, width, height, channels * width);
	return cairo_create (*surf);
}

inline cairo_t* create_layout_context ()
{
	cairo_surface_t *temp_surface;
	cairo_t *context;

	temp_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 0, 0);
	context = cairo_create (temp_surface);
	cairo_surface_destroy (temp_surface);
	return context;
}

inline void get_text_size (PangoLayout *layout, int *width, int *height)
{
	pango_layout_get_size (layout, width, height);
	*width /= PANGO_SCALE;
	*height /= PANGO_SCALE;
}

unsigned char* ScrollText::render_text_pixles(int *text_width, int *text_height)
{
	cairo_t *layout_context;
	cairo_t *render_context;
	cairo_surface_t *surface;
	PangoLayout *layout;
	unsigned char* surface_data = NULL;

	layout_context = create_layout_context();

	/* Create a PangoLayout, set the font and text */
	layout = pango_cairo_create_layout (layout_context);
	pango_layout_set_text (layout, m_scroll_text, -1);

	/* Load the font */
	pango_layout_set_font_description (layout, m_font_desc);

	/* Get text dimensions and create a context to render to */
	get_text_size (layout, text_width, text_height);
	render_context = create_cairo_context (*text_width, *text_height, 4,
			&surface, &surface_data);

	/* Render */
	//        cairo_set_source_rgba (render_context, (double)m_text_color.blue / 65535, (double)m_text_color.green / 65535,(double)m_text_color.red / 65535, 1);
	cairo_set_source_rgba (render_context, (double)m_text_color.blue / 65535, (double)m_text_color.green / 65535,(double)m_text_color.red / 65535, 1);

	pango_cairo_show_layout (render_context, layout);

	g_object_unref(layout);
	cairo_destroy(layout_context);
	cairo_destroy(render_context);
	cairo_surface_destroy (surface);
	return surface_data;
}

GLuint ScrollText::bind_background_texture()
{
	glBindTexture(GL_TEXTURE_2D, m_texture_buffer.texture[MAX_TEXTURE_SIZE]);
	GdkPixbuf * pixbuf = NULL;
	GError * error = NULL;

	if(m_background_image != NULL && m_background_image[0]) {
		pixbuf = gdk_pixbuf_new_from_file (m_background_image,&error);
	} else {
		pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, false, 8, m_width, m_height);
		gdk_pixbuf_fill(pixbuf, m_backgound_color.pixel);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
			gdk_pixbuf_get_width(pixbuf),
			gdk_pixbuf_get_height(pixbuf),
			0, GL_RGB, GL_UNSIGNED_BYTE,
			gdk_pixbuf_get_pixels(pixbuf));

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glEnable(GL_TEXTURE_2D);

	if(pixbuf)
	{
		g_object_unref(pixbuf);
		pixbuf = NULL;
	}
	return m_texture_buffer.texture[MAX_TEXTURE_SIZE];
}



void ScrollText::init_opengl()
{

}


void ScrollText::set_text(const char *text)
{
	char *pos;
	if (text != NULL) {
		if (m_scroll_text) {
			delete[] m_scroll_text;
			m_scroll_text = NULL;
		}
		size_t line_number = 0;
		for (int i = 0; i < strlen(text); i++)
			if (text[i] == '\n')
				line_number++;
		m_scroll_text = new char[strlen(text) + line_number * 8 + 1];
		int posi = 0;
		for (int i = 0; i < strlen(text); i++) {
			if (posi >= MAX_TEXT_LENGTH -1)
				break;
			if (text[i] == '\n') {
				memset(m_scroll_text + posi, ' ', 8);
				posi += 8;
			} else {
				m_scroll_text[posi] = text[i];
				posi++;
			}
		}
		m_scroll_text[posi] = '\0';
	}
}

void ScrollText::clear_texture_buffer()
{
	if (m_startplay_idle_id) {
		g_source_remove(m_startplay_idle_id);
		m_startplay_idle_id = 0;
	}

	if (m_layout_vec.size()) {
		for (vector <ScrollLayout> :: iterator iter = m_layout_vec.begin(); iter != m_layout_vec.end(); iter++) {
			m_layout_vec.erase(iter);
		}
	}
}
void ScrollText::redraw_background()
{
}

void ScrollText::redraw_text(GLuint texture, double move, double left, double top)
{
}

void ScrollText::redraw()
{
}

void ScrollText::move_text()
{
	vector <ScrollLayout> :: iterator iter = m_layout_vec.begin();
	for(vector <ScrollLayout> :: iterator iter = m_layout_vec.begin(); iter != m_layout_vec.end();iter++)
	{
		iter->draw_x -= m_rate;
	}
	vector <ScrollLayout> :: iterator end_iter = m_layout_vec.end() - 1;
	double value;

	value = 2.0 - ((double)end_iter->text_width)*2.0/((double)m_width);

	if(end_iter->draw_x <= value  && end_iter->meet_end==false) {
		end_iter->meet_end = true;
		set_text(NULL);
		m_startplay_idle_id = g_idle_add(g_callback_playscrolltxt, this);
	}

	vector <ScrollLayout> :: iterator start_iter = m_layout_vec.begin();
	value = -((double)start_iter->text_width)*2.0/((double)m_width);

	if(start_iter->draw_x <= value) {
		set_texture_available(start_iter->texture);
		if (m_layout_vec.size()) {
			m_layout_vec.erase(start_iter);
		}
	}
}

void ScrollText::move_and_redraw()
{
	move_text();
	redraw();
}

void ScrollText::scroll_text_at_idle()
{
	m_play_idle_id = g_idle_add(g_callback_draw_text_at_idle,this);
}

void ScrollText::set_texture_available(GLuint texture)
{
	for(int i=0; i<MAX_TEXTURE_SIZE; i++) {
		if(m_texture_buffer.texture[i] == texture) {
			m_texture_buffer.in_used[i] = false;
		}
	}
}

GLuint ScrollText::bind_text_texture(GdkPixbuf *pixbuf)
{
	GLuint texture = get_texture();
	if(texture != -1)
	{
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0,
				GL_RGBA, gdk_pixbuf_get_width(pixbuf),
				gdk_pixbuf_get_height(pixbuf),
				0, GL_RGBA, GL_UNSIGNED_BYTE,
				gdk_pixbuf_get_pixels(pixbuf));

		//texture_colorkey(0,0,0,10);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	glEnable(GL_TEXTURE_2D);
	return texture;
}
void ScrollText::init_texture_buffer()
{
	glGenTextures(MAX_TEXTURE_SIZE+1, m_texture_buffer.texture);
	for(int i=0; i<MAX_TEXTURE_SIZE; i++)
		m_texture_buffer.in_used[i] = false;
}

void ScrollText::add_layout_to_list(GdkPixbuf *pixbuf, int start_x, int start_y, int width, int height)
{
	if (width == 0 || height == 0 || m_height < height)
		return;

	GdkPixbuf *sub_pixbuf = NULL;
	sub_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8, width, m_height);
	memset(gdk_pixbuf_get_pixels(sub_pixbuf), 0, width * m_height * 4);
	gdk_pixbuf_copy_area(pixbuf, start_x, start_y, width, height ,sub_pixbuf, 0, (m_height - height) / 2);

	ScrollLayout tmp_layout = {0};
	tmp_layout.meet_end = false;
	tmp_layout.draw_x = 2.5 + ((double)start_x)*2.0/((double)m_width);

	tmp_layout.text_width = width;
	tmp_layout.text_height = height;
	tmp_layout.texture = bind_text_texture(sub_pixbuf);
	m_layout_vec.push_back(tmp_layout);

	if(sub_pixbuf)
	{
		g_object_unref(sub_pixbuf);
		sub_pixbuf = NULL;
	}
}

void ScrollText::draw_text_layout()
{

	int width, height;
	int text_width, text_height;

	GdkPixbuf * pixbuf = NULL;
	GdkPixbuf * sub_pixbuf = NULL;
	GError * error = NULL;

	if (m_scroll_text == NULL || m_context == NULL)
		return ;

	guchar *data = render_text_pixles(&width, &height);

	pixbuf = gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB,
			true, 8, width, height, width * 4, NULL, NULL);

	text_width = gdk_pixbuf_get_width(pixbuf);
	text_height = gdk_pixbuf_get_height(pixbuf);

	if(m_direction == 1)
	{
		int nSlipt = text_width / MAX_TEXT_LENGTH;
		int endSize = text_width % MAX_TEXT_LENGTH;
		for(int n=0; n<nSlipt; n++)
			add_layout_to_list(pixbuf, n*MAX_TEXT_LENGTH, 0, MAX_TEXT_LENGTH, text_height);
		add_layout_to_list(pixbuf, nSlipt*MAX_TEXT_LENGTH, 0, endSize, text_height);
	}
	if(pixbuf) {
		g_object_unref(pixbuf);
		pixbuf = NULL;
	}

}

GLuint ScrollText::get_texture()
{
	for(int i=0; i<MAX_TEXTURE_SIZE; i++)
	{
		if(m_texture_buffer.in_used[i] == false)
		{
			m_texture_buffer.in_used[i] = true;
			return m_texture_buffer.texture[i];
		}
	}
	return -1;
}

GdkPixbuf* ScrollText::get_text_pixbuf()
{
	int width, height;

	GdkPixbuf *pixbuf = NULL;
	GdkPixbuf *pixbuf_dest = NULL;
	GError * error = NULL;
    if (m_scroll_text == NULL)
        printf("1\n");
    else if (m_context == NULL)
        printf("2\n");
	if (m_scroll_text == NULL || m_context == NULL)
		return NULL;

	guchar *data = render_text_pixles(&width, &height);
	pixbuf = gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB,
			true, 8, width, height, width * 4, NULL, NULL);

	return pixbuf;
}

