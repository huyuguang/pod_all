#CXX11_HOME = /home/alpha/opt/gcc
#sudo apt-get install libomp-dev
#sudo apt-get install libcrypto++-dev
#sudo apt-get install libboost-all-dev

CXX11_HOME = /usr

MCL_HOME = ./mcl/include
CSV_HOME = ./csv

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
 -I $(CXX11_HOME)/include \
 -I $(MCL_HOME) \
 -I $(CSV_HOME) \
 -I.

MCL_FLAGS := \
 -DMCL_DONT_USE_OPENSSL

BOOST_FLAGS := \
 -DBOOST_SPIRIT_THREADSAFE 

RELEASE_FLAGS := \
 -O3 \
 -DNDEBUG \
 -DLINUX \

DEBUG_FLAGS := \
 -O0 \
 -D_DEBUG \
 -DLINUX \
 
LDFLAGS += \
 -static-libstdc++ -static-libgcc \
 -fmerge-all-constants \
 -L${CXX11_HOME}/lib64

LIBS := \
 -lpthread \
 -lrt \
 -lgmpxx \
 -lgmp \
 -lcryptopp \
 -lgomp \
 -lboost_filesystem \
 -lboost_system \
 -lboost_iostreams \
 -lboost_program_options

DIR := . $(CSV_HOME)

SRC := $(foreach d, $(DIR), $(wildcard $(d)/*.cc))

RELEASE_OBJ := $(patsubst %.cc, %.o, $(SRC))

DEBUG_OBJ := $(patsubst %.cc, %.d.o, $(SRC))

CXX := export LD_LIBRARY_PATH=${CXX11_HOME}/lib; ${CXX11_HOME}/bin/g++


all: pod_publishd pod_publish

%.o : %.cc
	$(CXX) -c $^ $(CXXFLAGS) $(RELEASE_FLAGS) $(MCL_FLAGS) $(BOOST_FLAGS) -o $@


%.d.o : %.cc
	$(CXX) -c $^ $(CXXFLAGS) $(DEBUG_FLAGS) $(MCL_FLAGS) $(BOOST_FLAGS) -o $@

pod_publishd : $(DEBUG_OBJ)
	$(CXX) $^ -o $@.exe $(LDFLAGS) $(LIBS)

pod_publish : $(RELEASE_OBJ)
	$(CXX) $^ -o $@.exe $(LDFLAGS) $(LIBS)

clean:
	find . -regex "\(.*\.o\|.*\.exe\)" | xargs rm

.PHONY :
