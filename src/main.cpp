#include <iostream>
#include <thread>
#include <string>
#include "FPGA.h"
#include <GL/glut.h>

#ifndef __GNUC__
#pragma comment(lib, "glut.lib")
#pragma comment(lib, "glut32.lib")
#endif

//#define  _debug_mode_
#define  _glut_

using namespace std;

extern int mainWindow;
bool exitGL = false; // pathfinder exit flag
void display();
void idleDisplay();
void reshape(GLsizei width, GLsizei height);
void initGL();

PATHFINDER fpga;
float Fvp;
float Fvh;
unsigned int iterCount;

void runPathfinderThread(){
	fpga.pathfinder(Fvh, Fvp, iterCount);
	exitGL = true;
}

int main(int argc , char* argv[]){
	try{
#if defined(_debug_mode_) || defined(_DEBUG)

		fpga.init("D:/C++Projects/Pathfinder/FPGA_tests/placed/alu4.place", "D:/C++Projects/Pathfinder/FPGA_tests/net/alu4.net", 100.0f, 7.0f);
		fpga.pathfinder(0.3, 1.2, 100);

#else

		if (argc == 1){
			cerr << "Enter .place .net edgeWeight channelCapacity Fvp Fvh iterCount\n";
			return 0;
		}

		float edgeWeight = stof(argv[3]);
		unsigned int channelCapacity = stoul(argv[4]);
		Fvp = stof(argv[5]);
		Fvh = stof(argv[6]);
		iterCount = stoul(argv[7]);

		fpga.init(argv[1], argv[2], edgeWeight, channelCapacity);

#ifdef _glut_
		thread pathfinderThread(runPathfinderThread);
		glutInit(&argc, argv);
		glutInitWindowSize(800, 800);
		glutInitWindowPosition(50, 50);
		mainWindow = glutCreateWindow("Pathfinder");
		glutDisplayFunc(display);
		glutIdleFunc(idleDisplay);
		glutReshapeFunc(reshape);
		initGL();
		glutMainLoop();
		pathfinderThread.join();
#else
		fpga.pathfinder(Fvh, Fvp, iterCount);
#endif

#endif

	}
	catch (exception& e){
		cerr << "error: " << e.what() << endl;
		return 1;
	}
	catch (...){
		cerr << "Unknown error" << endl;
		return 1;
	}

	return 0;
}
