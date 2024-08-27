SRC = $(wildcard *.cpp)
# SRC += $(wildcard ../*/*.cpp)
SRC += $(wildcard ../log/*.cpp)
SRC += $(wildcard ../model_utils/*.cpp)
SRC += $(wildcard ../config/*.cpp)
SRC += $(wildcard ../noc/*.cpp)

SC_SRC += $(wildcard ../crossbar/*.cpp)

HRD = $(wildcard *.h)
# HRD += $(wildcard ../*/*.h)
HRD += $(wildcard ../log/*.h)
HRD += $(wildcard ../model_utils/*.h)

SC_HRD += $(wildcard ../crossbar/*.h)

SIMV = simv

comp: $(SRC) $(HRD)
	@echo "src: $(SRC)"
	g++ -std=c++17 -g -O0 -o $(SIMV) $(SRC) -I./ -I../ -I../log/

sc: $(SC_SRC) $(SC_HRD)
	@echo "src: $(SC_SRC)"
	g++ -std=c++17 -g -O0 -lsystemc -o $(SIMV).o $(SC_SRC) -I./ -I../ -I../log/

dbg: $(SRC) $(HRD)
	@echo "src: $(SRC)"
	g++ -std=c++17 -fsanitize=address -g -fno-omit-frame-pointer \
	-g -O0 -o $(SIMV) $(SRC) -I./ -I../ -I../log/

.PHONY: run
run:
	./$(SIMV)

.PHONY: clean
clean:
	rm -rf $(SIMV)
