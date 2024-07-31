SRC = $(wildcard *.cpp)
SRC += $(wildcard ../log/*.cpp)
SRC += $(wildcard ../model_utils/*.cpp)
SRC += $(wildcard ../config/*.cpp)
SRC += $(wildcard ../noc/*.cpp)
HRD = $(wildcard *.h)
HRD += $(wildcard ../log/*.h)
HRD += $(wildcard ../model_utils/*.h)

comp: $(SRC) $(HRD)
	@echo "src: $(SRC)"
	g++ -std=c++17 -g -O0 -o test.exe $(SRC) -I./ -I../ -I../log/

dbg: $(SRC) $(HRD)
	@echo "src: $(SRC)"
	g++ -std=c++17 -fsanitize=address -g -fno-omit-frame-pointer \
	-g -O0 -o test.exe $(SRC) -I./ -I../ -I../log/

.PHONY: run
run:
	./test.exe

.PHONY: clean
clean:
	rm -rf *.exe
