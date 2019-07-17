#sudo apt-get install libomp-dev

CXX11_HOME = /usr

THIRD_PARTY_HOME = ../thirdparty
YAS_HOME = ../thirdparty/yas/include
PUB_HOME = ../public
SNARK_HOME = ../depends/install
MCL_HOME = $(SNARK_HOME)/include/depends/mcl/include

CXXFLAGS := \
 -g3 \
 -fPIC \
 -fopenmp \
 -std=c++17 \
 -Wall \
 -Wextra \
 -gdwarf-2 \
 -gstrict-dwarf \
 -Wno-parentheses \
 -Wdeprecated-declarations \
 -fmerge-all-constants  \
 -march=native \
 -mtune=native \
 -I $(CXX11_HOME)/include \
 -I $(MCL_HOME) \
 -I $(YAS_HOME) \
 -I $(THIRD_PARTY_HOME) \
 -I $(PUB_HOME) \
 -I. \

MCL_FLAGS := \
 -DMCL_DONT_USE_OPENSSL

RELEASE_FLAGS := \
 -O3 \
 -DNDEBUG \
 -DLINUX \
 -DMULTICORE \

DEBUG_FLAGS := \
 -O0 \
 -D_DEBUG \
 -DLINUX \
 
LDFLAGS += \
 -fmerge-all-constants \
 -L$(SNARK_HOME)/lib

LIBS := \
 -lpthread \
 -lrt \
 -lgmpxx \
 -lgmp \
 -lcryptopp \
 -lboost_filesystem \
 -lboost_system \
 -lboost_iostreams \
 -lboost_program_options \
 -lmcl \

DIR := . 

SRC := $(foreach d, $(DIR), $(wildcard $(d)/*.cc))



RELEASE_OBJ := $(patsubst %.cc, %.o, $(SRC))

DEBUG_OBJ := $(patsubst %.cc, %.d.o, $(SRC))



CXX := export LD_LIBRARY_PATH=${CXX11_HOME}/lib; ${CXX11_HOME}/bin/g++

#CC := export LD_LIBRARY_PATH=${CXX11_HOME}/lib; ${CXX11_HOME}/bin/gcc



all: pod_dummyd pod_dummy

%.o : %.cc
	$(CXX) -c $^ $(CXXFLAGS) $(RELEASE_FLAGS) $(MCL_FLAGS) -o $@


%.d.o : %.cc
	$(CXX) -c $^ $(CXXFLAGS) $(DEBUG_FLAGS) $(MCL_FLAGS) -o $@



pod_dummyd : $(DEBUG_OBJ)
	$(CXX) $^ -o $@.exe $(LDFLAGS) $(LIBS)

pod_dummy : $(RELEASE_OBJ)
	$(CXX) $^ -o $@.exe $(LDFLAGS) $(LIBS)

clean:
	find . -regex "\(.*\.o\|.*\.exe\)" | xargs rm

.PHONY :
