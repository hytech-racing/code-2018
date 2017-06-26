CXX = g++
FLAGS = -w -std=c++11

all: main_can

main_can: hytech17.o can_lib.o bt_lib.o main_can.o
	$(CXX) $(FLAGS) -o main_can BMS_temperatures.o PCU_status.o PCU_voltages.o TCU_status.o TCU_readings.o main_can.o can_lib.o bt_lib.o


# Object files

hytech17.o: ../../HyTech17_Library/*.cpp
	$(CXX) $(FLAGS) -c ../../HyTech17_Library/BMS_temperatures.cpp ../../HyTech17_Library/PCU_status.cpp ../../HyTech17_Library/PCU_voltages.cpp ../../HyTech17_Library/TCU_status.cpp ../../HyTech17_Library/TCU_readings.cpp

can_lib.o: can_lib.cpp
	$(CXX) $(FLAGS) -c can_lib.cpp

bt_lib.o: bt_lib.cpp
	$(CXX) $(FLAGS) -c bt_lib.cpp

main_can.o: main_can.cpp
	$(CXX) $(FLAGS) -c main_can.cpp

clean:
	-rm -f ../../HyTech17_Library/*.o
	-rm -f *.o
	-rm -f main_can

remake: clean all

clear_logs:
	-rm -f logs_*
