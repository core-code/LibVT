CC=g++
CXX=g++
LDFLAGS=-O3
LDLIBS=-lpthread
CXXFLAGS=-O3 -funroll-loops -fomit-frame-pointer 

OPTIM=1

MACHINE=$(shell uname -s)
ARCHITECTURE=$(shell uname -p)

ifeq ($(MACHINE), Darwin)

CXXFLAGS+=-I/opt/local/include
LDLIBS+=-framework OpenGL

ifeq ($(ARCHITECTURE), i386)
	# Intel Mac

ifeq ($(OPTIM), 1)
# MacPro
# CXXFLAGS+=-march=nocona -msse3 -DDXT_INTR

# MacbookPro
CXXFLAGS+=-march=i686 -msse2 -DDXT_INTR

else

#
CXXFLAGS+=
CXXFLAGS=-g
endif

endif

ifeq ($(ARCHITECTURE), powerpc)
	# PowerPC Mac
echo "Disabling SSE instructions on PowerPC"
CXXFLAGS+=

endif

else

LDLIBS+=-lGL

ifeq ($(ARCHITECTURE), x86_64)
	# Opteron
CXXFLAGS+=-march=opteron -msse2 -DDXT_INTR

else

	# everything considered i386/linux
CXXFLAGS+=-msse2 -DDXT_INTR

endif

endif

CFLAGS=$(CXXFLAGS) `sdl-config --cflags` -I/opt/local/include


default: main example 2dxt viewdxt playdxt

main: dxt.o main.o util.o intrinsic.o
example: example.o libdxt.o dxt.o util.o intrinsic.o
2dxt: dxt.o 2dxt.o libdxt.o util.o intrinsic.o


viewdxt:viewdxt.o glsl.o dxt.o
	${CC} -o viewdxt viewdxt.o glsl.o dxt.o intrinsic.o `sdl-config --libs` $(LDLIBS) -lGLEW

playdxt:playdxt.o glsl.o dxt.o
	${CC} -o playdxt playdxt.o glsl.o dxt.o intrinsic.o `sdl-config --libs` $(LDLIBS) -lGLEW

install: default
	/bin/cp -f main example 2dxt playdxt viewdxt bin

clean:
	/bin/rm -f *.o *~ main example 2dxt playdxt viewdxt *.rgba *.dxt out.*

