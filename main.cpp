#include <iostream>
#include <thread>
#include "FPGA.h"
using namespace std;
PATHFINDER fpga;
//int runViewer(int, char**);

void runPathfinderThread(){
	fpga.pathfinder(0.3, 1.2, 100);
}

int main(int argc , char* argv[]){
	cout << sizeof(CHANNEL_TEMP) << " " << sizeof(CHANNEL) << endl;
	fpga.init("FPGA_tests/placed/clma.place", "FPGA_tests/net/clma.net");
	thread pathfinderThread(runPathfinderThread);

	//runViewer(argc, argv);
	pathfinderThread.join();
	return 0;
}