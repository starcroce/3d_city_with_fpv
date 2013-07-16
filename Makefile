#  List of all programs you want to build
EX=final

#  Libraries - Linux and various flavors of Unix
LIBS=-lglut -lGLU
CLEAN=rm -f $(EX) *.o *.a
#  Libraries - OSX
ifeq "$(OSTYPE)" "darwin"
LIBS=-framework GLUT -framework OpenGL
endif
#  Libraries - MinGW
ifeq "$(OS)" "Windows_NT"
LIBS=-lglut32cu -lglu32 -lopengl32
CLEAN=del *.exe *.o *.a
endif

#  Main target
all: $(EX)

#  Generic compile rules
.c.o:
	gcc -c -O -Wall $<

#  Generic compile and link
%: %.c CSCIx229.a
	gcc -Wall -O3 -o $@ $^ $(LIBS)

#  Delete unwanted files
clean:;$(CLEAN)

#  Create archive (include glWindowPos here if you need it)
CSCIx229.a:fatal.o loadtexbmp.o print.o project.o errcheck.o object.o
	ar -rcs CSCIx229.a $^

#  SDL version of ex20
sdl20: sdl20.o fatal.o loadtexbmp.o project.o errcheck.o printsdl.o
	gcc -Wall -O3 -o $@ $^ -lSDL -lSDL_mixer -lGLU -lGL -lm

#  Obligatory UNIX inside joke
love:
	@echo "not war?"
