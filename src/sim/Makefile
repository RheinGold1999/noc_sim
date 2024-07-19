SRC = $(wildcard *.cpp)
SRC += $(wildcard ../log/*.cpp)
SRC += $(wildcard ../model_utils/*.cpp)
SRC += $(wildcard ../config/*.cpp)
HRD = $(wildcard *.h)
HRD += $(wildcard ../log/*.h)
HRD += $(wildcard ../model_utils/*.h)

test.exe: $(SRC) $(HRD)
	@echo "src: $(SRC)"
	g++ -std=c++17 -g -O0 -o test.exe $(SRC) -I./ -I../ -I../log/

.PHONY: run
run:
	./test.exe

.PHONY: clean
clean:
	rm -rf *.exe
