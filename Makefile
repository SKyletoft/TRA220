CXX := $(HIPCC)
CC  := $(HIPCC)

OPT_LEVEL ?= -O3 -g0
# OPT_LEVEL ?= -Og -g3

CXXFLAGS := \
	-xhip \
	-std=gnu++23 \
	-Wall -Wextra \
	-D __HIP_PLATFORM_AMD__ \
	\
	-isystem /opt/rocm/hip/include \
	-isystem fmt/include \
	-I. \
	-Iinc \
	\
	-Wno-unused-parameter \
	-Wfloat-equal \
	-Wnon-virtual-dtor \
	-Wcast-align \
	-Wzero-as-null-pointer-constant \
	-Wunused \
	-Woverloaded-virtual \
	-Wmisleading-indentation \
	-Wnull-dereference \
	\
	-Werror=format-security  \
	-Werror=missing-field-initializers \
	-Werror=return-type \
	-Werror=conversion \
	-Werror=sign-conversion \
	-Werror=float-conversion

CLEAN := -rm -r Main *.o gcm.cache

vpath %.cpp src

SRCS := $(notdir $(wildcard src/*.cpp))
OBJS := $(SRCS:.cpp=.o)

run: Main
	./Main

compile_commands.json: Makefile
	bear -- $(CXX) $(OPT_LEVEL) $(CXXFLAGS) *.cpp -o Main

Main: $(OBJS) libfmt.a
	$(CXX) $^ -o $@

%.o: %.cpp Makefile $(wildcard inc/*.h) $(wildcard inc/*.hpp)
	$(CXX) $(OPT_LEVEL) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean lint run

lint:
	clang-tidy $(wildcard *.cpp) $(wildcard *.cu) $(wildcard *.c)

clean:
	$(CLEAN)
