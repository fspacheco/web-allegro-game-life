CC=gcc
CFLAGS=`pkg-config --cflags allegro-5 allegro_acodec-5 allegro_audio-5 allegro_color-5 allegro_dialog-5 allegro_font-5 allegro_image-5 allegro_main-5 allegro_memfile-5 allegro_physfs-5 allegro_primitives-5 allegro_ttf-5`
LDLIBS=-lm `pkg-config --libs allegro-5 allegro_acodec-5 allegro_audio-5 allegro_color-5 allegro_dialog-5 allegro_font-5 allegro_image-5 allegro_main-5 allegro_memfile-5 allegro_physfs-5 allegro_primitives-5 allegro_ttf-5`

all: allegro-life allegro-life-BIG

allegro-life: main.o
	$(CC) main.o -o allegro-life $(LDLIBS)

main.o: main.c cell_files.h
	$(CC) -o main.o -DWIDTH=100 -DHEIGHT=64 -c main.c

allegro-life-BIG: main-BIG.o
	$(CC) main.o -o allegro-life-BIG $(LDLIBS)

main-BIG.o: main.c
	$(CC) -o main.o -DWIDTH=25 -DHEIGHT=16 -c main.c 

clean:
	rm -f allegro-life allegro-life-BIG *.o
