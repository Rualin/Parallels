all: task2

task1: task3_1.o functions.o
	g++ -fopenmp task3_1.o functions.o -o task3_1
task3_1.o:
	g++ -fopenmp task3_1.cpp -c -o task3_1.o

task2: task3_2.o functions.o
	g++ -fopenmp task3_2.o functions.o -o task3_2
task3_2.o:
	g++ -fopenmp task3_2.cpp -c -o task3_2.o

functions.o:
	g++ functions.cpp -c -o functions.o

clean:
	rm *.o task3_1 task3_2
