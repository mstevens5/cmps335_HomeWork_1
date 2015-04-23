CFLAGS = -I ./include
LIB    = ./libggfonts.so
LFLAGS = $(LIB) -lrt -lX11 -lGLU -lGL -pthread -lm #-lXrandr

all: homework1 

homework1: homework1.cpp
	g++ $(CFLAGS) homework1.cpp -Wall -Wextra $(LFLAGS) -o homework1

clean:
	rm -f homework1
	rm -f *.o

