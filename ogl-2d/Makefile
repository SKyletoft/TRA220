CXX := g++
CC := gcc

OPT_LEVEL ?= -Og -g3

CXXFLAGS := \
	-std=c++26 \
	-Wall -Wextra \
	\
	-Wno-unused-parameter \
	-Wfloat-equal \
	-Wnon-virtual-dtor \
	-Wcast-align \
	-Wzero-as-null-pointer-constant \
	-Wunused \
	-Woverloaded-virtual \
	-Wmisleading-indentation \
	-Wduplicated-cond \
	-Wduplicated-branches \
	-Wlogical-op \
	-Wnull-dereference \
	\
	-Werror=format-security  \
	-Werror=missing-field-initializers \
	-Werror=return-type \
	-Werror=conversion \
	-Werror=sign-conversion \
	-Werror=float-conversion

SRC := $(wildcard *.cpp)
INC := $(wildcard *.hpp) $(wildcard *.h)
OBJ := $(SRC:.cpp=.o)

run: Main
	./Main

claude: claude.cpp
	g++ -O3 -std=c++26 `sdl2-config --cflags --libs` -lGL -lGLEW claude.cpp -o claude

Main: $(OBJ)
	$(CXX) $^ -o $@ `sdl2-config --cflags --libs` -lGL -lGLEW

compute.o: compute.cpp $(INC) $(wildcard *.glsl)

%.o: %.cpp $(INC)
	$(CXX) $(CPPFLAGS) $(OPT_LEVEL) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJ) Main claude
