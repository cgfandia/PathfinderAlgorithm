#include <iostream>
#include <thread>
#include "FPGA.h"
using namespace std;
PATHFINDER fpga;
int runViewer(int, char**);

void runPathfinderThread(){
	fpga.pathfinder(0.9, 1.2, 300);
}

int main(int argc , char* argv[]){
	if (argc <= 1)
		return 0;

	fpga.init(argv[1], argv[2], 7.0f, 300.0f);
	thread pathfinderThread(runPathfinderThread);
	//fpga.pathfinder(0.3, 1.2, 100);
	runViewer(argc, argv);
	pathfinderThread.join();
	return 0;
}