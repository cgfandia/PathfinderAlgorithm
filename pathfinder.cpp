#include <cstring>
#include <algorithm>
#include <queue>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cfloat>
#include <climits>
#include <regex>
#include <omp.h>
#include "pathfinder.h"
using namespace std;

static float Fvp;
static float Fvh;

void checkFileOpening(const ifstream& file, const string& filename);

CHANNEL::CHANNEL() : ID(0), used(0), itsDestination(0), minWeight(FLT_MAX), channelOccupancy(0), occupancyHistory(1), occupancyMult(1), channelCapacity(INT_MAX), prevChannel(0) {}

inline void CHANNEL::setPv(const size_t& Niter){
	occupancyMult = 1 + (Niter - 1) * Fvp * channelOccupancy;
}

inline void CHANNEL::setHv(const size_t& Niter){
	if (Niter <= 1)
	{
		occupancyHistory = 1;
	}
	else{
		occupancyHistory = occupancyHistory + Fvh * channelOccupancy;
	}
}

inline float CHANNEL::getWeightToThisChannel(const float& baseWeightToChannel){
	return occupancyMult * (baseWeightToChannel + occupancyHistory);
}

PATHFINDER::~PATHFINDER(){
	delete[]channelsGraph;
	channelsGraph = nullptr;
	delete[]routedPaths;
	routedPaths = nullptr;
};

void PATHFINDER::initGraphAndConnections(const string& graphPath, const string& connectionsPath, bool directionalGraphFlag){

	const size_t BUFF_SIZE = 2048;

	// Clear previous curcuits informations

	delete []channelsGraph;
	channelsGraph = nullptr;
	delete []routedPaths;
	routedPaths = nullptr;
	connectionsList.clear();

	directionalGraph = directionalGraphFlag;
	ifstream graphFile(graphPath, ifstream::in);
	ifstream connectionsFile(connectionsPath, ifstream::in);

	checkFileOpening(graphFile, graphPath);
	checkFileOpening(connectionsFile, connectionsPath);

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
	graphSize = maxID + 1;
	try
	{
		channelsGraph = new CHANNEL[graphSize];
	}
	catch (bad_alloc& ba){
		cerr << "Graph memory allocation error: " << ba.what() << '\n';
		exit(1);
	}

	/* Initialization graph with data from file (fromID toID channelWeight channelCapacity)*/
	graphFile.clear();
	graphFile.seekg(0, graphFile.beg);
	while (!graphFile.eof())
	{
		unsigned int fstID, secID, channelCapacity;
		float channelWeight;
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
			channelWeight = stof(tmpString);

			IDsString >> tmpString;
			channelCapacity = stoi(tmpString);
		}
		catch (const std::invalid_argument& ia)
		{
			cerr << "Initialization graph: " << ia.what() << '\n';
			exit(1);
		}

		channelsGraph[fstID].ID = fstID;
		channelsGraph[fstID].channelCapacity = channelCapacity;
		channelsGraph[fstID].baseNeighboursWeights.push_back(pair<unsigned int, float>(secID, channelWeight)); // Insert to channel its neighbor and neighbor's weight
		if (!directionalGraph)
		{
			channelsGraph[secID].ID = secID;
			channelsGraph[secID].baseNeighboursWeights.push_back(pair<unsigned int, float>(fstID, channelWeight));
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

bool PATHFINDER::buildPath(const CHANNEL* finalGraph, const unsigned int& srcChannel, const unsigned int& dstChannel, vector<unsigned int>* outPath){
	auto currentID = dstChannel;
	while (currentID != srcChannel)
	{
		auto prevChannelID = finalGraph[currentID].prevChannel;
		if (prevChannelID == 0)
		{
			//std::cerr << "Cant find path\n";
			return false;
		}
		outPath->push_back(currentID);
		currentID = prevChannelID;
	}
	outPath->push_back(srcChannel);
	return true;
}

void PATHFINDER::dijkstra(const unsigned int& iterN, const int& currentConnectionsListIt, unordered_set<unsigned int>* usedChannelsSet){

	/* Initialization temp graph */

	CHANNEL* tempGraph = (CHANNEL*)malloc(sizeof(CHANNEL) * (graphSize + 1));
	if (tempGraph == nullptr){
		cerr << "Temporary graph memory allocation error" << endl;
		exit(1);
	}
	memcpy(tempGraph, channelsGraph, sizeof(CHANNEL) * (graphSize + 1));;

	const vector<unsigned int>* currentConnectionsList = &connectionsList[currentConnectionsListIt];
	vector<vector<unsigned int> >* currentRoutedPaths = &routedPaths[currentConnectionsListIt];
	unsigned int initChannel = currentConnectionsList->at(0); // Source channel = connections[0]

	// Set destinations in graph by connections[1...end()]

	for (size_t i = 1; i < currentConnectionsList->size(); ++i) tempGraph[currentConnectionsList->at(i)].itsDestination = 1;

	// Main algorithm

	unsigned int destinationsCount = currentConnectionsList->size() - 1;
	queue<unsigned int> queueOfChannels;
	tempGraph[initChannel].prevChannel = initChannel;
	tempGraph[initChannel].minWeight = 0;
	do
	{
		if (queueOfChannels.size() > 0)
		{
			initChannel = queueOfChannels.front();
			queueOfChannels.pop();
			if (tempGraph[initChannel].itsDestination){
				destinationsCount--;
				tempGraph[initChannel].itsDestination = 0;
				tempGraph[initChannel].minWeight = 0;
				if (destinationsCount == 0) break;
			}
		}
		tempGraph[initChannel].used = 1;

		for (int i = 0; i < tempGraph[initChannel].baseNeighboursWeights.size(); ++i)
		{
			unsigned int currentNeighborId = tempGraph[initChannel].baseNeighboursWeights[i].first;
			if (tempGraph[ tempGraph[initChannel].baseNeighboursWeights[i].first].used == 0){
				queueOfChannels.push(currentNeighborId);
				float Cn = tempGraph[currentNeighborId].getWeightToThisChannel(tempGraph[initChannel].baseNeighboursWeights[i].second); // tempGraph[initChannel].baseNeighboursWeights[i].second : base graph weight(edge weight)
				if (tempGraph[currentNeighborId].minWeight > tempGraph[initChannel].minWeight + Cn)
				{
					tempGraph[currentNeighborId].minWeight = tempGraph[initChannel].minWeight + Cn;
					tempGraph[currentNeighborId].prevChannel = initChannel;
				}
			}
		}

	} while (queueOfChannels.size() != 0);

	/* Add channels to wire(subgraph or path of source channel and destination channels) */

	for (size_t i = 1; i < currentConnectionsList->size(); ++i)
	{
		vector<unsigned int> tempPath;
		if (buildPath(tempGraph, currentConnectionsList->at(0), currentConnectionsList->at(i), &tempPath)) // If path to destination channel exist
		{
			currentRoutedPaths->push_back(tempPath);
			usedChannelsSet->insert(tempPath.begin(), tempPath.end());
			/*for_each(tempPath.begin(), tempPath.end(), [](unsigned int i){cout << i << " "; });
			cout << endl;*/
		}
	}
	free(tempGraph);
}

void PATHFINDER::pathfinder(const float& FvhParam, const float& FvpParam, const size_t& maxIter){
	Fvh = FvhParam;
	Fvp = FvpParam;
	for (size_t i = 1; i <= maxIter; ++i)
	{
		vector<unsigned int> usedChannels;
		usedChannels.reserve(graphSize);
#ifndef _OPENMP

		// Loop over all multi terminal wires(connections)

		for (int cListIt = 0; cListIt < connectionsList.size(); ++cListIt)
		{
			unordered_set<unsigned int> usedChannelsSet;
			dijkstra(i, cListIt, &usedChannelsSet);
			usedChannels.insert(usedChannels.end(), usedChannelsSet.begin(), usedChannelsSet.end());// Copy channels from path to collections of used channels
		}

		// Clear channelOccupancy in each channel after first iteration

		if (i > 1){
			for (size_t i = 0; i <= graphSize; ++i){ channelsGraph[i].channelOccupancy = 0; }
		}

		// Loop over all used channels to increase channelOccupancy in each used channel

		for (size_t i = 0; i < usedChannels.size(); ++i){ channelsGraph[i].channelOccupancy++; }

		// Loop over all used channels to update occupancyHustory and occupancyMult in each used channel

		for (size_t i = 0; i < usedChannels.size(); ++i){ channelsGraph[usedChannels[i]].setHv(i); channelsGraph[usedChannels[i]].setPv(i); }
#endif
#ifdef _OPENMP
#pragma omp parallel shared(usedChannels)
		{			
			double A = omp_get_wtime();
#pragma omp for
			for (int cListIt = 0; cListIt < connectionsList.size(); ++cListIt)
			{
				unordered_set<unsigned int> usedChannelsSet;
				dijkstra(i, cListIt, &usedChannelsSet);
#pragma omp critical
				usedChannels.insert(usedChannels.end(), usedChannelsSet.begin(), usedChannelsSet.end());
			}
			if (i > 1){
#pragma omp for
				for (int i = 0; i < graphSize; ++i){ channelsGraph[i].channelOccupancy = 0; }
			}
#pragma omp for
			for (int i = 0; i < usedChannels.size(); ++i){
#pragma omp atomic
				channelsGraph[usedChannels[i]].channelOccupancy++;
			}
#pragma omp for
			for (int i = 0; i < usedChannels.size(); ++i){
#pragma omp critical
				{
					channelsGraph[usedChannels[i]].setHv(i);
					channelsGraph[usedChannels[i]].setPv(i);
				}
			}
#pragma omp single
			cout << omp_get_wtime() - A << endl;
		}
#endif
	}
}