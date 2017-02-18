#include <cstring>
#include <algorithm>
#include <queue>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cfloat>
#include <climits>
#include <omp.h>
#include "pathfinder.h"
using namespace std;

static float Fvp;
static float Fvh;
const size_t BUFF_SIZE = 2048;

NODE::NODE() : used(0), itsDestination(0), minWeight(FLT_MAX), nodeOccupancy(0), occupancyHistory(1), occupancyMult(1), nodeCapacity(INT_MAX), prevNode(0) {}

inline void NODE::setPv(const size_t& Niter){
	occupancyMult = 1 + (Niter - 1) * Fvp * nodeOccupancy;
}

inline void NODE::setHv(const size_t& Niter){
	if (Niter <= 1)
	{
		occupancyHistory = 1;
	}
	else{
		occupancyHistory = occupancyHistory + Fvh * nodeOccupancy;
	}
}

inline float NODE::getWeightToThisNode(const float& baseWeightToNode){
	return occupancyMult * (baseWeightToNode + occupancyHistory);
}

PATHFINDER::~PATHFINDER(){
	delete[]nodesGraph;
	nodesGraph = nullptr;
	delete[]routedPaths;
	routedPaths = nullptr;
};

void PATHFINDER::initGraphAndConnections(const string& graphPath, const string& connectionsPath, bool directionalGraphFlag){

	// Clear previous curcuits informations

	delete []nodesGraph;
	nodesGraph = nullptr;
	delete []routedPaths;
	routedPaths = nullptr;
	connectionsList.clear();

	directionalGraph = directionalGraphFlag;
	ifstream graphFile(graphPath, ifstream::in);
	ifstream connectionsFile(connectionsPath, ifstream::in);
	if (!(graphFile && connectionsFile))
	{
		cerr << "Failed to open files\n";
		exit(0);
	}

	/* Set graphSize by find max ID in file */
	unsigned int maxID = 0;

	while (!graphFile.eof())
	{
		unsigned int fstID, secID;
		char buffChar[BUFF_SIZE];
		graphFile.getline(buffChar, BUFF_SIZE);
		string fileString = string(buffChar);
		istringstream IDsString(fileString);
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
	catch (bad_alloc& ba){
		cerr << "Graph memory allocation error: " << ba.what() << '\n';
		exit(1);
	}

	/* Initialization graph with data from file (fromID toID nodeWeight nodeCapacity)*/
	graphFile.clear();
	graphFile.seekg(0, graphFile.beg);
	while (!graphFile.eof())
	{
		unsigned int fstID, secID, nodeCapacity;
		float nodeWeight;
		char buffChar[BUFF_SIZE];
		graphFile.getline(buffChar, BUFF_SIZE);
		string fileString = string(buffChar);
		istringstream IDsString(fileString);
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
		nodesGraph[fstID].baseNeighboursWeights.push_back(pair<unsigned int, float>(secID, nodeWeight)); // Insert to node its neighbor and neighbor's weight
		if (!directionalGraph)
		{
			nodesGraph[secID].ID = secID;
			nodesGraph[secID].baseNeighboursWeights.push_back(pair<unsigned int, float>(fstID, nodeWeight));
		}
	}
	graphFile.close();

	/* Initialization connections with data from file (srcID destID destID ...)*/
	while (!connectionsFile.eof())
	{
		char buffChar[BUFF_SIZE];
		connectionsFile.getline(buffChar, BUFF_SIZE);
		vector<unsigned int> tmpIDList;
		unsigned int ID;
		string fileString = string(buffChar);
		istringstream IDsString(fileString);
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
		} while (tmpStringSize != 0);
		connectionsList.push_back(tmpIDList);
	}
	connectionsFile.close();

	// Prepare memory for routed connections
	try
	{
		routedPaths = new vector<vector<unsigned int> >[connectionsList.size()];
	}
	catch (bad_alloc& ba){
		cerr << "routedPaths memory allocation error: " << ba.what() << '\n';
		exit(1);
	}
}

void PATHFINDER::buildPath(const NODE* finalGraph, const unsigned int& srcNode, const unsigned int& dstNode, vector<unsigned int>* outPath){
	unsigned int tempId = dstNode;
	while (tempId != srcNode)
	{
		outPath->push_back(tempId);
		tempId = finalGraph[tempId].prevNode;
		if (tempId == 0)
		{
			//std::cerr << "Cant find path\n";
			break;
		}
	}
	outPath->push_back(srcNode);
}

void PATHFINDER::dijkstra(const unsigned int& iterN, const int& currentConnectionsListIt, unordered_set<unsigned int>* usedNodesSet){

	/* Initialization temp graph */
	NODE* tempGraph = (NODE*)malloc(sizeof(NODE) * (graphSize + 1));
	if (tempGraph == nullptr){
		cerr << "Temporary graph memory allocation error" << endl;
		exit(1);
	}
	memcpy(tempGraph, nodesGraph, sizeof(NODE) * (graphSize + 1));

	const vector<unsigned int>* currentConnectionsList = &connectionsList[currentConnectionsListIt];
	vector<vector<unsigned int> >* currentRoutedPaths = &routedPaths[currentConnectionsListIt];
	unsigned int initNode = currentConnectionsList->at(0); // Source node = connections[0]

	// Set destinations in graph by connections[1...end()]
	for (size_t i = 1; i < currentConnectionsList->size(); ++i) tempGraph[currentConnectionsList->at(i)].itsDestination = 1;

	unsigned int destinationsCount = currentConnectionsList->size() - 1;
	queue<unsigned int> queueOfNodes;
	tempGraph[initNode].prevNode = initNode;
	tempGraph[initNode].minWeight = 0;
	do
	{
		if (queueOfNodes.size() > 0)
		{
			initNode = queueOfNodes.front();
			queueOfNodes.pop();
			if (tempGraph[initNode].itsDestination){
				destinationsCount--;
				tempGraph[initNode].itsDestination = 0;
				tempGraph[initNode].minWeight = 0;
				if (destinationsCount == 0) break;
			}
		}
		tempGraph[initNode].used = 1;

		for (int i = 0; i < tempGraph[initNode].baseNeighboursWeights.size(); ++i)
		{
			unsigned int currentNeighborId = tempGraph[initNode].baseNeighboursWeights[i].first;
			if (tempGraph[ tempGraph[initNode].baseNeighboursWeights[i].first].used == 0){
				queueOfNodes.push(currentNeighborId);
				float Cn = tempGraph[currentNeighborId].getWeightToThisNode(tempGraph[initNode].baseNeighboursWeights[i].second); // tempGraph[initNode].baseNeighboursWeights[i].second : base graph weight(edge weight)
				if (tempGraph[currentNeighborId].minWeight > tempGraph[initNode].minWeight + Cn)
				{
					tempGraph[currentNeighborId].minWeight = tempGraph[initNode].minWeight + Cn;
					tempGraph[currentNeighborId].prevNode = initNode;
				}
			}
		}

	} while (queueOfNodes.size() != 0);

		/* Add nodes to wire(subgraph or path of source node and destination nodes) */
		//#ifdef _OPENMP
		//#pragma omp critical
		//#endif
		{
			/*ofstream connecctionsOutF("newSlashdotConn.txt", ofstream::app);
			string tempS = to_string(currentConnectionsList->at(0));
			bool destExist = false;*/
			for (size_t i = 1; i < currentConnectionsList->size(); ++i)
			{
				vector<unsigned int> tempPath;
				buildPath(tempGraph, currentConnectionsList->at(0), currentConnectionsList->at(i), &tempPath);
				/*if (tempPath.size() > 1)
				{
				tempS += " ";
				tempS += to_string(tempPath.front());
				destExist = true;
				}*/
				currentRoutedPaths->push_back(tempPath);
				usedNodesSet->insert(tempPath.begin(), tempPath.end());
				
				/*for_each(tempPath.begin(), tempPath.end(), [](unsigned int i){cout << i << " "; });
				cout << endl;*/
			}
			/*if (destExist)
			{
			connecctionsOutF << tempS << endl;
			}
			connecctionsOutF.close();*/
		}
		free(tempGraph);
}

void PATHFINDER::pathfinder(const float& FvhParam, const float& FvpParam, const size_t& maxIter){
	Fvh = FvhParam;
	Fvp = FvpParam;
	for (size_t i = 1; i <= maxIter; ++i)
	{
		// Loop over all multi terminal wires(connections)
		vector<unsigned int> usedNodes;
		usedNodes.reserve(graphSize);
#ifndef _OPENMP
		for (size_t i = 0; i <= graphSize; ++i){
			for (size_t nIt = 0; nIt < nodesGraph[i].baseNeighboursWeights.size(); ++nIt){
				nodesGraph[i].nodeOccupancy++;
			}
		}

		for (int cListIt = 0; cListIt < connectionsList.size(); ++cListIt)
		{
			unordered_set<unsigned int> usedNodesSet;
			dijkstra(i, cListIt, &usedNodesSet);
			usedNodes.insert(usedNodes.end(), usedNodesSet.begin(), usedNodesSet.end());// Copy nodes from path to collections of used nodes
		}
		if (i > 1){
			for (size_t i = 0; i <= graphSize; ++i){ nodesGraph[i].nodeOccupancy = 0; }
		}
		// Loop over all used nodes to increase nodeOccupancy in each node
		for (size_t i = 0; i < usedNodes.size(); ++i){ nodesGraph[i].nodeOccupancy++; }
		// Loop over all used nodes to update occupancyHustory and occupancyMult in each node
		for (size_t i = 0; i < usedNodes.size(); ++i){ nodesGraph[usedNodes[i]].setHv(i); nodesGraph[usedNodes[i]].setPv(i); }
#endif
#ifdef _OPENMP
			double A = omp_get_wtime();
#pragma omp parallel shared(usedNodes)
		{
#pragma omp for
			for (int cListIt = 0; cListIt < connectionsList.size(); ++cListIt)
			{
				unordered_set<unsigned int> usedNodesSet;
				dijkstra(i, cListIt, &usedNodesSet);
#pragma omp critical
				usedNodes.insert(usedNodes.end(), usedNodesSet.begin(), usedNodesSet.end());
			}
			if (i > 1){
#pragma omp for
				for (int i = 0; i < graphSize; ++i){ nodesGraph[i].nodeOccupancy = 0; }
			}
#pragma omp for
			for (int i = 0; i < usedNodes.size(); ++i){
#pragma omp atomic
				nodesGraph[usedNodes[i]].nodeOccupancy++;
			}
#pragma omp for
			for (int i = 0; i < usedNodes.size(); ++i){
#pragma omp critical
				{
					nodesGraph[usedNodes[i]].setHv(i);
					nodesGraph[usedNodes[i]].setPv(i);
				}
			}
#pragma omp single
			cout << omp_get_wtime() - A << endl;
		}
#endif
	}
}