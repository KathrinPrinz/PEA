// parallel_enumeration.cpp: Definiert den Einstiegspunkt für die Anwendung.
//
#include "algorithm_parallel_three_stages.h"
#include "ilcplex/cplex.h"
#include "cplex_interface.h"
#include <fstream>
int main(int arc, char *argv[])
{
	std::string instance = argv[1];
	std::string output_file = argv[2];
	int number_threads=std::stoi(argv[3]);
	initialize_parallel_tasks(instance,output_file,number_threads);

	return 0;
}
