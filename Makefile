CXX     = g++
CXXFLAGS = -std=c++17
LDLIBS  = `pkg-config opencv4 --cflags --libs`
WFLAGS = -Wall -Wextra -O3

main: src/main.cpp image.o line_segmentation.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(WFLAGS) $(LDLIBS) 

line_segmentation.o: src/line_segmentation.cpp src/line_segmentation.h
	$(CXX) -c $^ $(CXXFLAGS) $(WFLAGS) $(LDLIBS) 

image.o: src/image.cpp src/image.h
	$(CXX) -c $^ $(CXXFLAGS) $(WFLAGS) $(LDLIBS) 

.PHONY: clean

clean:
	rm -rf main *.o

