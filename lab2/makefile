all: mat_vec integral easier
	
mat_vec: mat_vec.o
	g++ -fopenmp mat_vec.o -o mat_vec
mat_vec.o: mat_vec.cpp
	g++ -fopenmp mat_vec.cpp -o mat_vec.o -c

integral: integral.o
	g++ -fopenmp integral.o -o integral
integral.o: integral.cpp
	g++ -fopenmp integral.cpp -o integral.o -c

easier: easier.o
	g++ easier.o -o easier
easier.o: easier.cpp
	g++ easier.cpp -c -o easier.o

clean:
	rm *.o integral mat_vec
