all: sample2D

sample2D: shivay.cpp glad.c
	g++ -o sample2D shivay.cpp glad.c  -lglfw -lftgl -lSOIL  -I/usr/local/include -I/usr/include/freetype2 -L/usr/local/lib -ldl -lGL  -lmpg123

clean:
	rm sample2D
