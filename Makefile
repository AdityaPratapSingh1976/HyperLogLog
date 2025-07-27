
EXTERNLIB=/Users/adityapratapsingh/Documents/External_libraries
INCDIR=	.\
		$(EXTERNLIB)/boost/include\
		$(EXTERNLIB)\
		./include

CODEDIR=./src

CXX=/opt/homebrew/bin/g++-12
CXXFLAGS=-std=c++17 -MM -MP -Wall -Wextra -Wpedantic $(foreach dir, $(INCDIR), -I$(dir))
OPT=-O3


LDFLAG_BOOST=-L$(EXTERNLIB)/boost/lib -lboost_serialization
LDFLAG_XXHASH=-L$(EXTERNLIB)/xxHash/lib -lxxhash
LDFLAGS= -Wl,-rpath,$(EXTERNLIB)/boost/lib \
		-Wl,-rpath,$(EXTERNLIB)/xxHash/lib

SOURCE=$(foreach dir, $(CODEDIR), $(wildcard $(dir)/*.cpp))
OBJ=$(SOURCE:.cpp=.o)

HEADER_DEPS=$(wildcard ./include/*.h)
DEP_FILES := $(HEADER_DEPS:.h=.d)


TARGET=main
BINARY=./bin/$(TARGET)

all: $(BINARY)

$(BINARY): $(OBJ)
	@echo SRC=$(SOURCE)
	@echo OBJ=$(OBJ)
	@echo "Linking $@"
	$(CXX) $(OBJ) $(LDFLAG_BOOST) $(LDFLAG_XXHASH) $(LDFLAGS) $(OPT) -o $@

%.o: %.cpp
	@echo "Compiling $^"
	$(CXX) $(CXXFLAGS) -c -o $@ $^


clean:
	rm $(BINARY) $(OBJS) $(CODEDIR)/*.o $(CODEDIR)/*.d