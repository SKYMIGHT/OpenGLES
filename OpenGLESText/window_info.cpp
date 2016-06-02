#include "window_info.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


WindowInfo::WindowInfo():
	window_id(-1),
	window_width(0),
	window_height(0),
	background_color(0)
{
	memset(content, 0, MAX_STRING_LEN);
	memset(background_image, 0, MAX_STRING_LEN); 
}

void WindowInfo::set_info(int argc, char *argv[])
{
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-wid") == 0) {
			if (i + 1 >= argc) {
				printf("Cannot get window id\n");
				exit(-1);
			}
			window_id = strtoul(argv[i + 1], NULL, 10);
		}

		if (strcmp(argv[i], "-width") == 0) {
			if (i + 1 >= argc) {
				printf("Cannot get window width\n");
				exit(-1);
			}
			window_width = strtoul(argv[i + 1], NULL, 10);
		}
		if (strcmp(argv[i], "-height") == 0) {
			if (i + 1 >= argc) {
				printf("Cannot get window height\n");
				exit(-1);
			}
			window_height = strtoul(argv[i + 1], NULL, 10);
		}
		if (strcmp(argv[i], "-content") == 0) {
			if (i + 1 >= argc) {
				printf("Cannot get window content\n");
				exit(-1);
			}
			sprintf(content, "%s", argv[i + 1]);
		}
		if (strcmp(argv[i], "-background_image") == 0) {
			if (i + 1 >= argc) {
				printf("Cannot get window background_image\n");
				exit(-1);
			}
			sprintf(background_image, "%s", argv[i + 1]);
		}
		if (strcmp(argv[i], "-background_color") == 0) {
			if (i + 1 >= argc) {
				printf("Cannot get window background_color\n");
				exit(-1);
			}
			// input string should be ARGB(FFFFFFFF), skip first charactor
			sscanf(argv[i + 1] + 1 , "%x", &background_color);
		}
	}
}
