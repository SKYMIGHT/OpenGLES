# Straight forward Makefile to compile all examples in a row

INCDIR=-I./Common
LIBS=-lGLESv2 -lEGL -lm -lX11
FLAGS=-fpermissive
COMMONSRC=./Common/esShader.c    \
          ./Common/esTransform.c \
          ./Common/esShapes.c    \
          ./Common/esUtil.c		 \
		  ./MultiTexture.c		 \
		  ./scrolltext.cpp       \
		  ./text_info.cpp		 \
		  ./window_info.cpp
COMMONHRD=esUtil.h


all:
	g++ ${COMMONSRC} -o Test ${LIBS} ${INCDIR} $(FLAGS) `pkg-config --cflags --libs gtk+-3.0`

clean:
	find . -name "CH??_*" | xargs rm -f
