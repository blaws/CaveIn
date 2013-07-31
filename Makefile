all: CaveIn

CaveIn: CaveIn.c
	g++ CaveIn.c font.c -o CaveIn -framework OpenGL -framework GLUT

clean:
	rm -f *~ CaveIn
