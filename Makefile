CC = g++
CFLAGS = -g -I. -I/usr/include/OpenEXR -L/usr/local/lib -lIlmImf -lImath -lHalf -Wall 

all: main

main: exr_test.cpp exr_test.h
	$(CC) $(CFLAGS) exr_test.cpp exr_test.h -o main

test: main aatestR.exr checkerboard_many.exr checkerboard_small.exr mttamwest.exr sailplane.exr squares_medium.exr squares_small.exr stilllife.exr tree.exr
	./main aatestR.exr -b 0.8
	./main sailplane.exr -b 0.8
	./main checkerboard_many.exr -b 0.8 
	./main checkerboard_small.exr -b 0.8
#	./main mttamwest.exr -b 0.8
	./main squares_medium.exr -b 0.8
	./main squares_small.exr -b 0.8
	./main stilllife.exr -b 0.8
	./main tree.exr -b 0.8