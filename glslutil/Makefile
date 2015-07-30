CPP = g++
C_FLAGS = -fPIC -O -c -DGL_GLEXT_PROTOTYPES

LINK = g++ -fPIC
ifndef GL_LIB_LOC
GL_LIB_LOC = /usr/lib64/nvidia
endif

OBJS = ShaderIF.o

libglsl.so: $(OBJS)
	$(LINK) -shared -L$(GL_LIB_LOC) -o libglsl.so $(OBJS)
	cp libglsl.so ../lib/

ShaderIF.o: ShaderIF.h ShaderIF.c++
	$(CPP) $(C_FLAGS) ShaderIF.c++
