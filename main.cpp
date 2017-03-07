#include <iostream>
#include <algorithm>
#include "FPGA.h"
using namespace std;
int main(int argc , char* argv[]){
	//PATHFINDER pathfinderOBJ;
	//pathfinderOBJ.initGraphAndConnections("FPGA_tests/graph8.txt" , "FPGA_tests/graph8C.txt" , true);
	//pathfinderOBJ.pathfinder(0.8f, 1.2f, 1);
	PATHFINDER fpga;
	fpga.init("FPGA_tests/placed/clma.place", "FPGA_tests/net/clma.net");
	fpga.pathfinder(0.5, 0.5, 1);
	return 0;
}