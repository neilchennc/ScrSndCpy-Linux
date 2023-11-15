C_FLAGS   := -s -O2 -rdynamic -no-pie -Wall
GTK_FLAGS := `pkg-config --cflags --libs gtk+-3.0`
OUTPUT    := ScrSndCpy

all:
	gcc -o ${OUTPUT} main.c net.c popen.c preference.c track_device.c ${C_FLAGS} ${GTK_FLAGS} -lm

clean:
	rm ${OUTPUT}
