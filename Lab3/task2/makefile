TYPE ?= double

all: main test #easier

main: main.o
	g++ main.o -o main
main.o: main.cpp client_server.hpp funcs.hpp
	g++ main.cpp -DTYPE=$(TYPE) -c -o main.o

test: test.o
	g++ test.o -o test
test.o: test.cpp funcs.hpp
	g++ test.cpp -DTYPE=$(TYPE) -c -o test.o

clean:
	rm -f main *.o
