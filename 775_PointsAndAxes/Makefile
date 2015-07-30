CPP = g++
INC = -I../cryphutil -I../fontutil -I../glslutil -I../mvcutil
C_FLAGS = -fPIC -g -c -DGL_GLEXT_PROTOTYPES $(INC)

LINK = g++ -fPIC -g
LOCAL_UTIL_LIBRARIES = -L../lib -lcryph -lfont -lglsl -limage -lmvc
ifndef GL_LIB_LOC
GL_LIB_LOC = /usr/lib64/nvidia
endif
OGL_LIBRARIES = -L$(GL_LIB_LOC) -lglut -lGLU -lGL

OBJS = main.o AxesMV.o PointsMV.o PCA.o

main: $(OBJS) ../lib/libcryph.so ../lib/libfont.so ../lib/libglsl.so ../lib/libimage.so ../lib/libmvc.so
	$(LINK) -o main $(OBJS) $(LOCAL_UTIL_LIBRARIES) $(OGL_LIBRARIES)

../lib/libcryph.so: ../cryphutil/AffPoint.h ../cryphutil/AffPoint.c++ ../cryphutil/AffVector.h ../cryphutil/AffVector.c++ ../cryphutil/Matrix4x4.h ../cryphutil/Matrix4x4.c++
	(cd ../cryphutil; make)

../lib/libfont.so: ../fontutil/CFont.h ../fontutil/CFont.c++ ../fontutil/CGLString.h ../fontutil/CGLString.c++
	(cd ../fontutil; make)

../lib/libglsl.so: ../glslutil/ShaderIF.h ../glslutil/ShaderIF.c++
	(cd ../glslutil; make)

../lib/libimage.so: ../imageutil/ImageReader.h ../imageutil/ImageReader.c++
	(cd ../imageutil; make)

../lib/libmvc.so: ../mvcutil/Controller.h ../mvcutil/Controller.c++ ../mvcutil/ModelView.h ../mvcutil/ModelView.c++
	(cd ../mvcutil; make)

main.o: main.c++
	$(CPP) $(C_FLAGS) main.c++
AxesMV.o: AxesMV.h AxesMV.c++
	$(CPP) $(C_FLAGS) AxesMV.c++
PointsMV.o: PointsMV.h PointsMV.c++
	$(CPP) $(C_FLAGS) PointsMV.c++
PCA.o: PCA.h PCA.c++
	$(CPP) $(C_FLAGS) PCA.c++
