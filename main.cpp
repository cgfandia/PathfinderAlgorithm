#include <iostream>
#include <algorithm>
#include "pathfinder.h"
#include "FPGA.h"
using namespace std;
int main(int argc , char* argv[]){
	//PATHFINDER pathfinderOBJ;
	//pathfinderOBJ.initGraphAndConnections("FPGA_tests/graph8.txt" , "FPGA_tests/graph8C.txt" , true);
	//pathfinderOBJ.pathfinder(0.8f, 1.2f, 1);
	FPGA fpga;
	fpga.initFPGA("FPGA_tests/placed/ex5p.place", "FPGA_tests/net/ex5p.net");
	return 0;
}