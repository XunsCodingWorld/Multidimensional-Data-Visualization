CPP = g++
C_FLAGS = -fPIC -O -c

LINK = g++ -fPIC

OBJS = AffPoint.o AffVector.o Matrix4x4.o

libcryph.so: $(OBJS)
	$(LINK) -shared -o libcryph.so $(OBJS)
	cp libcryph.so ../lib/

AffPoint.o: AffPoint.h AffPoint.c++
	$(CPP) $(C_FLAGS) AffPoint.c++

AffVector.o: AffVector.h AffVector.c++
	$(CPP) $(C_FLAGS) AffVector.c++

Matrix4x4.o: Matrix4x4.h Matrix4x4.c++
	$(CPP) $(C_FLAGS) Matrix4x4.c++
