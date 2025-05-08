all: task1 #easier

task1: task1.o
	g++ task1.o -o task1
task1.o: task1.cpp
	g++ -c task1.cpp -o task1.o

# easier: easier.o
# 	g++ easier.o -o easier
# easier.o: easier.cpp
# 	g++ -c easier.cpp -o easier.o

clean:
	rm -f task1 *.o
