all: gpu 

gpu: gpu.cpp
	pgc++ -std=c++11 -lboost_program_options -acc=gpu -cudalib=cublas -Minfo=all gpu.cpp -o gpu

clean:
	rm *.o gpu
