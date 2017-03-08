#include <iostream>
#include "FPGA.h"
using namespace std;
PATHFINDER fpga;
int runViewer(const PATHFINDER&, int, char**);
int main(int argc , char* argv[]){
	fpga.init("FPGA_tests/placed/clma.place", "FPGA_tests/net/clma.net");
	fpga.pathfinder(0.3, 1.2, 100);
	runViewer(fpga, argc, argv);
	return 0;
}