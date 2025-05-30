all: graph

graph: graph.cu
	nvc++ graph.cu -lboost_program_options -o graph

clear:
	rm graph *.o
