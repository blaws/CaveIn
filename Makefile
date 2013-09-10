all: CaveIn

CaveIn: CaveIn.cpp
	g++ CaveIn.cpp font.c -o CaveIn -framework OpenGL -framework GLUT

clean:
	rm -f *~ CaveIn
