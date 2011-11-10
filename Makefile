CC = g++
CFLAGS = -g -I. -I/usr/include/OpenEXR -L/usr/local/lib -lIlmImf -lImath -lHalf -Wall 

all: main

main: exr_test.cpp exr_test.h
	$(CC) $(CFLAGS) exr_test.cpp exr_test.h -o main
