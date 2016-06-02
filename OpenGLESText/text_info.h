#ifndef __TEXT_INFO_H
#define __TEXT_INFO_H
#define MAX_STRING_LEN 1024
class TextInfo
{
	public:
		TextInfo();
		void set_info(int argc, char *argv[]);
		char text_font[MAX_STRING_LEN];
		int text_size;
		int text_color;
		int rate;
		int direction;
		int align;
};

#endif
