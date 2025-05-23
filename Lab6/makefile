all: cpu gpu cpu_mult

non_parallel: non_parallel.o
	g++ non_parallel.o -lboost_program_options -o non_parallel
non_parallel.o: non_parallel.cpp
	g++ non_parallel.cpp -lboost_program_options -c -o non_parallel.o

# cpu: cpu.o
# 	pgc++ -std=c++11 cpu.o -o cpu
# cpu.o: cpu.cpp
# 	pgc++ -std=c++11 -lboost_program_options -acc=host -Minfo=all cpu.cpp -c -o cpu.o

cpu: cpu.cpp
	pgc++ -std=c++11 -lboost_program_options -acc=host -Minfo=all cpu.cpp -o cpu

# gpu: gpu.o
# 	pgc++ -std=c++11 gpu.o -o gpu
# gpu.o: gpu.cpp
# 	pgc++ -std=c++11 -lboost_program_options -acc=gpu -Minfo=all gpu.cpp -c -o gpu.o

gpu: gpu.cpp
	pgc++ -std=c++11 -lboost_program_options -acc=gpu -Minfo=all gpu.cpp -o gpu

cpu_mult: cpu.cpp
	pgc++ -std=c++11 -lboost_program_options -acc=multicore -Minfo=all cpu.cpp -o cpu_mult

easier: easier.cpp
	g++ easier.cpp -o easier

clean:
	rm *.o non_parallel cpu gpu cpu_mult
	