#include <iostream>
#include "pathfinder.h"
using namespace std;
int main(int argc , char* argv[]){
	PATHFINDER pathfinderOBJ;
	pathfinderOBJ.initGraphAndConnections("graph8.txt" , "graph8C.txt" , true);
	pathfinderOBJ.pathfinder(0.8f , 1.2f , 1);
	return 0;
}