SYSTEMC_HOME = /usr/local/systemc
SYSTEMC_INC = -I$(SYSTEMC_HOME)/include
# SYSTEMC_LIB = -L$(SYSTEMC_HOME)/lib -lsystemc
SYSTEMC_LIB = -lsystemc

UVM_SC_HOME = /usr/local/uvm-systemc
# UVM_SC_INC = -I$(UVM_SC_HOME)/include
UVM_SC_INC = 
# UVM_SC_LIB = -L$(UVM_SC_HOME)/lib -luvm-systemc
# UVM_SC_LIB = $(UVM_SC_HOME)/lib/libuvm-systemc.so
UVM_SC_LIB = -luvm-systemc
# UVM_SC_SRC = $(wildcard /mnt/d/code/SYSTEMC/uvm-systemc-1.0-beta6/src/uvmsc/**/*.cpp)

CROSSBAR_PATH = ../../src/
CROSSBAR_INC = -I$(CROSSBAR_PATH)
CROSSBAR_SRC = $(wildcard $(CROSSBAR_PATH)/crossbar/*.cpp)

VIP_PATH = .
VIP_INC = -I$(VIP_PATH)
VIP_SRC = $(wildcard $(VIP_PATH)/**/*.cpp)

TEST_PATH = .
TEST_INC = -I$(TEST_PATH)
TEST_SRC = $(wildcard $(TEST_PATH)/*.cpp)

DBG_FLAG = -g -O0
CXXFLAGS = -std=c++17 -fPIC $(DBG_FLAG)

all:
	g++ $(CXXFLAGS) -o test.out $(TEST_SRC) $(TEST_INC) \
  $(CROSSBAR_INC) $(CROSSBAR_SRC) $(VIP_INC) $(VIP_SRC) \
	$(UVM_SC_LIB) $(SYSTEMC_LIB)
