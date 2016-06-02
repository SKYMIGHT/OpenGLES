#include "scrolltext.h"
#include "text_info.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

//#include <gtk/gtkgl.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

TextInfo::TextInfo():
	text_size(0),
	text_color(0),
	rate(1),
	direction(0)
{
	memset(text_font, 0, MAX_STRING_LEN);
}

void TextInfo::set_info(int argc, char *argv[])
{
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-text_font") == 0) {
			if (i + 1 >= argc) {
				printf("Cannot get text font\n");
				exit(-1);
			}
			sprintf(text_font, "%s", argv[i + 1]);
		}
		if (strcmp(argv[i], "-text_size") == 0) {
			if (i + 1 >= argc) {
				printf("Cannot get text size\n");
				exit(-1);
			}
			text_size = strtoul(argv[i + 1], NULL, 10);
		}
		if (strcmp(argv[i], "-text_color") == 0) {
			if (i + 1 >= argc) {
				printf("Cannot get text color\n");
				exit(-1);
			}
			sscanf(argv[i + 1] + 1, "%x", &text_color);
		}
		if (strcmp(argv[i], "-rate") == 0) {
			if (i + 1 >= argc) {
				printf("Cannot get text rate\n");
				exit(-1);
			}
			rate = strtoul(argv[i + 1], NULL, 10);
		}
		if (strcmp(argv[i], "-direction") == 0) {
			if (i + 1 >= argc) {
				printf("Cannot get text direction\n");
				exit(-1);
			}
			direction = strtoul(argv[i + 1], NULL, 10);
		}
		if (strcmp(argv[i], "-align") == 0) {
			if (i + 1 >= argc) {
				printf("Cannot get text align\n");
				exit(-1);
			}
			align = strtoul(argv[i + 1], NULL, 10);
		}

	}
}
