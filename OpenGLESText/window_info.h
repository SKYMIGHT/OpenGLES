#ifndef __WINDOW_INFO_H
#define __WINDOW_INFO_H

#define MAX_STRING_LEN 1024

class WindowInfo
{
	public:
		WindowInfo();
		void set_info(int argc, char *argv[]);
		int window_id;
		int window_width;
		int window_height;
		char content[MAX_STRING_LEN];

		char background_image[MAX_STRING_LEN];
		unsigned int background_color;
};

#endif
