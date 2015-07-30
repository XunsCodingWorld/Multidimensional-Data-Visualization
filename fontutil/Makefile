CPP = g++
C_FLAGS = -fPIC -O -c -DGL_GLEXT_PROTOTYPES -I../cryphutil

LINK = g++ -fPIC
ifndef GL_LIB_LOC
GL_LIB_LOC = /usr/lib64/nvidia
endif

OBJS = CFont.o CGLString.o

libfont.so: $(OBJS)
	$(LINK) -shared -L$(GL_LIB_LOC) -o libfont.so $(OBJS)
	cp libfont.so ../lib/

CFont.o: CFont.h CFont.c++
	$(CPP) $(C_FLAGS) CFont.c++

CGLString.o: CGLString.h CGLString.c++
	$(CPP) $(C_FLAGS) CGLString.c++
