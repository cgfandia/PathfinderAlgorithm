#include <iostream>
#include "pathfinder.h"
using namespace std;
int main(int argc , char* argv[]){
	PATHFINDER pathfinderOBJ;
	pathfinderOBJ.initGraphAndConnections("testgraph.txt" , "testconnections.txt" , true);
	pathfinderOBJ.pathfinder(0.5f , 0.6f , 1);
	return 0;
}