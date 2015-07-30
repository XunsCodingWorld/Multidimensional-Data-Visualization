CPP = g++
INC = -I../glslutil -I../cryphutil -I../imageutil
C_FLAGS = -fPIC -O -c -DGL_GLEXT_PROTOTYPES $(INC)

LINK = g++ -fPIC
ifndef GL_LIB_LOC
GL_LIB_LOC = /usr/lib64/nvidia
endif

OBJS = Controller.o ModelView.o ModelViewWithLighting.o BasicShape.o BasicShapeModelView.o

libmvc.so: $(OBJS)
	$(LINK) -shared -L$(GL_LIB_LOC) -o libmvc.so $(OBJS)
	cp libmvc.so ../lib/

Controller.o: Controller.h Controller.c++
	$(CPP) $(C_FLAGS) Controller.c++

ModelView.o: ModelView.h ModelView.c++
	$(CPP) $(C_FLAGS) ModelView.c++

ModelViewWithLighting.o: ModelViewWithLighting.h ModelViewWithLighting.c++
	$(CPP) $(C_FLAGS) ModelViewWithLighting.c++

BasicShape.o: BasicShape.h BasicShape.c++
	$(CPP) $(C_FLAGS) BasicShape.c++

BasicShapeModelView.o: BasicShapeModelView.h BasicShapeModelView.c++
	$(CPP) $(C_FLAGS) BasicShapeModelView.c++
