CXX = g++
FLAGS = -w -std=c++11

all: main_can

main_can: can_lib.o bt_lib.o main_can.o
	$(CXX) $(FLAGS) -o main_can main_can.o can_lib.o bt_lib.o


# Object files

can_lib.o: can_lib.cpp
	$(CXX) $(FLAGS) -c can_lib.cpp

bt_lib.o: bt_lib.cpp
	$(CXX) $(FLAGS) -c bt_lib.cpp

main_can.o: main_can.cpp
	$(CXX) $(FLAGS) -c main_can.cpp

clean:
	-rm -f *.o
	-rm -f main_can

remake: clean all
