EXTERNLIB=/Users/adityapratapsingh/Documents/External_libraries
INCDIR= . \
        $(EXTERNLIB)/boost/include \
        $(EXTERNLIB) \
        ./include

CODEDIR=./src

CXX=/opt/homebrew/bin/g++-12
CXXFLAGS=-std=c++17 -MMD -Wall -Wextra -Wpedantic $(foreach dir, $(INCDIR), -I$(dir))
OPT=-O3

LDFLAG_BOOST=-L$(EXTERNLIB)/boost/lib -lboost_serialization
LDFLAG_XXHASH=-L$(EXTERNLIB)/xxHash/lib -lxxhash
LDFLAGS= -Wl,-rpath,$(EXTERNLIB)/boost/lib \
        -Wl,-rpath,$(EXTERNLIB)/xxHash/lib

SOURCE=$(foreach dir, $(CODEDIR), $(wildcard $(dir)/*.cpp))
OBJ=$(addprefix $(CODEDIR)/,$(notdir $(SOURCE:.cpp=.o)))

HEADER_DEPS=$(wildcard ./include/*.h)
DEP_FILES := $(OBJ:.o=.d)

TARGET=main
BINARY=./bin/$(TARGET)

all: $(BINARY)

$(BINARY): $(OBJ)
	@echo "Linking $@"
	$(CXX) $(OBJ) $(LDFLAG_BOOST) $(LDFLAG_XXHASH) $(LDFLAGS) $(OPT) -o $@

$(CODEDIR)/%.o: $(CODEDIR)/%.cpp
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(BINARY) $(OBJ) $(DEP_FILES)

# Include the dependency files
-include $(DEP_FILES)