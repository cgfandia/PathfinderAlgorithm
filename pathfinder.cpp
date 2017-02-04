#include <cstring>
#include <queue>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <omp.h>
#include "pathfinder.h"
using namespace std;

static float Fvp;
static float Fvh;
const size_t BUFF_SIZE = 2048;

NODE::NODE() : used(false) , minWeight(FLT_MAX), nodeOccupancy(0), occupancyHistory(1) , occupancyMult(1) , nodeCapacity(INT_MAX), prevNode(0) {}

inline void NODE::setPv(const size_t& Niter){
	occupancyMult = 1 + (Niter - 1) * Fvp * nodeOccupancy;
}

inline void NODE::setHv(const size_t& Niter){
	if (Niter <= 1)
	{
		occupancyHistory = 1;
	}else{
		occupancyHistory = occupancyHistory + Fvh * nodeOccupancy;
	}
}

inline float NODE::getWeightToThisNode(const float& baseWeightToNode){
	return occupancyMult * (baseWeightToNode + occupancyHistory);
}

void PATHFINDER::initGraphAndConnections(const string& graphPath , const string& connectionsPath , bool directionalGraphFlag){
	directionalGraph = directionalGraphFlag;
	ifstream graphFile( graphPath , ifstream::in);
	ifstream connectionsFile( connectionsPath , ifstream::in);
	if (!(graphFile && connectionsFile))
	{
		cerr << "Failed to open files\n";
		exit(0);
	}

/* Set graphSize by find max ID in file */
	unsigned int maxID = 0;
	
	while (!graphFile.eof())
	{
		unsigned int fstID , secID;
		char buffChar[BUFF_SIZE];
		graphFile.getline(buffChar , BUFF_SIZE);
		string fileString = string(buffChar);
		istringstream IDsString( fileString );
		size_t tmpStringSize = fileString.size();
		if (tmpStringSize == 0){ continue; }
		try{
			string tmpString;
			IDsString >> tmpString;
			fstID = stoi(tmpString);

			IDsString >> tmpString;
			secID = stoi(tmpString);
			if (fstID > maxID){ maxID = fstID; }
			if (secID > maxID){ maxID = secID; }
		}
		catch (const std::invalid_argument& ia)
		{
			cerr << "Initialization graph: " << ia.what() << '\n';
			exit(0);
		}
	}
	graphSize = maxID;
	try
	{
		nodesGraph = new NODE[graphSize + 1];
	}
	catch (std::bad_alloc& ba){
		cerr << "Graph memory allocation error: " << ba.what() << '\n';
		exit(1);
	}

/* Initialization graph with data from file (fromID toID nodeWeight nodeCapacity)*/
	graphFile.clear();
	graphFile.seekg (0, graphFile.beg);
	while (!graphFile.eof())
	{
		unsigned int fstID , secID , nodeCapacity;
		float nodeWeight;
		char buffChar[BUFF_SIZE];
		graphFile.getline(buffChar , BUFF_SIZE);
		string fileString = string(buffChar);
		istringstream IDsString( fileString );
		size_t tmpStringSize = fileString.size();
		if (tmpStringSize == 0){ continue; }
		try{
			string tmpString;
			IDsString >> tmpString;
			fstID = stoi(tmpString);

			IDsString >> tmpString;
			secID = stoi(tmpString);

			IDsString >> tmpString;
			nodeWeight = stof(tmpString);

			IDsString >> tmpString;
			nodeCapacity = stoi(tmpString);
			
		}
		catch (const std::invalid_argument& ia)
		{
			cerr << "Initialization graph: " << ia.what() << '\n';
			exit(1);
		}

		nodesGraph[fstID].ID = fstID;
		nodesGraph[fstID].nodeCapacity = nodeCapacity;
		nodesGraph[fstID].neighbors.insert(pair<unsigned int , float>(secID , nodeWeight)); // Insert to node its neighbor and neighbor's weight
		if (!directionalGraph)
		{
			nodesGraph[secID].ID = secID;
			nodesGraph[secID].neighbors.insert(pair<unsigned int , float>(fstID , nodeWeight));
		}
	}
	graphFile.close();

/* Initialization connections with data from file (srcID destID destID ...)*/
	while (!connectionsFile.eof())
	{
		char buffChar[BUFF_SIZE];
		connectionsFile.getline(buffChar , BUFF_SIZE);
		vector<unsigned int> tmpIDList;
		unsigned int ID;
		string fileString = string(buffChar);
		istringstream IDsString( fileString );
		size_t tmpStringSize = fileString.size();
		if (tmpStringSize == 0){ continue; }
		do 
		{
			string tmpString;
			IDsString >> tmpString;
			tmpStringSize = tmpString.size();
			if (tmpStringSize == 0){ break; }
			try
			{
				ID = stoi(tmpString);
			}
			catch (const std::invalid_argument& ia)
			{
				cerr << "Initialization connections: " << ia.what() << '\n';
				exit(1);
			}
			tmpIDList.push_back(ID);
		} while ( tmpStringSize != 0 );
		connectionsList.push_back(tmpIDList);
	}
	connectionsFile.close();
}

void PATHFINDER::buildPath(const NODE* finalGraph , const unsigned int& srcNode , const unsigned int& dstNode , set<unsigned int>& outPath){
	unsigned int tempId = dstNode;
	while (tempId != srcNode)
	{
		outPath.insert(tempId);
		tempId = finalGraph[tempId].prevNode;
		if (tempId == 0)
		{
			//std::cerr << "Cant find path\n";
			break;
		}
	}
	outPath.insert(srcNode);
}

void PATHFINDER::dijkstra( const vector<unsigned int>* connections, const unsigned int& iterN, set<unsigned int>& outPath ){
/* Initialization temp graph */
	NODE* tempGraph = (NODE*)malloc(sizeof(NODE) * ( graphSize + 1 ));
	if(tempGraph == nullptr){
		cerr << "Temporary graph memory allocation error" << endl;
		exit(1);
	}
	memcpy(tempGraph , nodesGraph , sizeof(NODE) * ( graphSize + 1 ));

	unsigned int initNode = connections->at(0); // Source node = connections[0]
	queue<unsigned int> queueOfNodes;

	tempGraph[initNode].prevNode = initNode;
	tempGraph[initNode].minWeight = 0;
	do 
	{
		if (queueOfNodes.size() >= 1)
		{
			initNode = queueOfNodes.front();
			queueOfNodes.pop();
		}
		tempGraph[initNode].used = true;

		for (auto neighborsIt = tempGraph[initNode].neighbors.begin() ; neighborsIt != tempGraph[initNode].neighbors.end() ; ++neighborsIt)
		{
			unsigned int currentNeighborId = neighborsIt->first;
			if(!tempGraph[currentNeighborId].used){
				queueOfNodes.push(currentNeighborId); 
				float Cn = tempGraph[currentNeighborId].getWeightToThisNode(neighborsIt->second); // neighborsIt->second : base graph weight(edge weight)
				if (tempGraph[currentNeighborId].minWeight > tempGraph[initNode].minWeight + Cn)
				{
					tempGraph[currentNeighborId].minWeight = tempGraph[initNode].minWeight + Cn;
					tempGraph[currentNeighborId].prevNode = initNode;
				}
			}
		}

	} while (queueOfNodes.size() != 0);

/* Add nodes to wire(subgraph or path of source node and destination nodes) */
	for (size_t i = 1 ; i < connections->size() ; ++i)
	{
		buildPath(tempGraph , connections->at(0) , connections->at(i) , outPath);
	}
/*#pragma omp critical
	{
		for_each(outPath.begin() , outPath.end() , [](unsigned int i){cout << i << " ";});
		cout << endl;
	}*/
	free(tempGraph);
}

void PATHFINDER::pathfinder(const float& FvhParam, const float& FvpParam, const size_t& maxIter){
	Fvh = FvhParam;
	Fvp = FvpParam;
	for (size_t i = 1 ; i <= maxIter ; ++i)
	{
		// Loop over all multi terminal wires(connections)
		vector<unsigned int> usedNodes;
		double A = omp_get_wtime();
#ifdef _OPENMP
	#pragma omp parallel shared(usedNodes)
	#pragma omp for nowait
#endif
		for (int cListIt = 0 ; cListIt < connectionsList.size() ; ++cListIt)
		{			
			set<unsigned int> outPath;
			dijkstra(&(connectionsList[cListIt]) , i , outPath);
	#ifdef _OPENMP
		#pragma omp critical
	#endif
			usedNodes.insert(usedNodes.end() , outPath.begin() , outPath.end()); // Copy nodes from path to collections of used nodes
		}
		cout << omp_get_wtime() - A << endl;

		// Clear nodeOccupancy in all nodes 
		if (i > 1){
			for (NODE* graphIt = nodesGraph ; graphIt < nodesGraph + graphSize; ++graphIt){ graphIt->nodeOccupancy = 0; }
		}

		// Loop over all used nodes to increase nodeOccupancy in each node
		NODE* graphPtr = nodesGraph;
		for_each(usedNodes.begin() , usedNodes.end() , [graphPtr](unsigned int uNode){ graphPtr[uNode].nodeOccupancy++; });

		// Loop over all used nodes to update occupancyHustory and occupancyMult in each node
		for_each(usedNodes.begin() , usedNodes.end() , [graphPtr , i](unsigned int uNode){ graphPtr[uNode].setHv(i); graphPtr[uNode].setPv(i); });
	}
}