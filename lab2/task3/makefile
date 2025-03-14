all: task1 task2 #easier

task1: task3_1.o
	g++ -fopenmp task3_1.o -o task3_1
task3_1.o: task3_1.cpp
	g++ -fopenmp task3_1.cpp -c -o task3_1.o

task2: task3_2.o
	g++ -fopenmp task3_2.o -o task3_2
task3_2.o: task3_2.cpp
	g++ -fopenmp task3_2.cpp -c -o task3_2.o

# easier: easier.o
# 	g++ easier.o -o easier
# easier.o: easier.cpp
# 	g++ easier.cpp -c -o easier.o

clean:
	rm *.o task3_1 task3_2 easier
