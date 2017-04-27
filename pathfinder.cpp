#include <cstring>
#include <algorithm>
#include <queue>
#include <unordered_set>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cfloat>
#include <climits>
#include <regex>
#include <chrono>
#include <omp.h>
#include "FPGA.h"
using namespace std;

static float Fvp;
static float Fvh;

void checkFileOpening(const ifstream& file, const string& filename);
void updateFPGA();

CHANNEL::CHANNEL() : used(0), inQueue(0), itsDestination(0), minWeight(FLT_MAX), channelOccupancy(0), occupancyHistory(1), occupancyMult(1), channelCapacity(INT_MAX), prevChannel(-1) {}

inline void CHANNEL::setPv(const size_t& Niter){
	occupancyMult = 1 + (Niter - 1) * Fvp * max(0, (int)(channelOccupancy + 1 - channelCapacity));
}

inline void CHANNEL::setHv(const size_t& Niter){
	if (Niter <= 1){
		occupancyHistory = 1;
	}
	else{
		occupancyHistory = occupancyHistory + Fvh * (Niter - 1);
		//occupancyHistory = occupancyHistory + Fvh * max(0, (int)(channelOccupancy + 1 - channelCapacity));
	}
}

inline float CHANNEL::getWeightToThisChannel(const float& baseWeightToChannel){
	return occupancyMult * (baseWeightToChannel + occupancyHistory);
}

PATHFINDER::~PATHFINDER(){
	delete[]channelsGraph;
	channelsGraph = nullptr;
		
	for (size_t i = 0; i < channels2DArrayWH; i++)
	{
		delete[] channels2DArray[i];
		channels2DArray[i] = nullptr;
	}

	delete[] channels2DArray;
	channels2DArray = nullptr;

	routedChannels.clear();
};

void PATHFINDER::init(const string& placeFile, const string& netsFile, const float& edgeWeightParam, const float& channelCapacityParam){

	initFPGA(placeFile, netsFile);

	float edgeWeight = edgeWeightParam;
	unsigned int channelCapacity = channelCapacityParam;
	
	channels2DArrayWH = blocks2DArrayWH - 2;
	graphSize = channels2DArrayWH * channels2DArrayWH;
	channelsGraph = new CHANNEL[graphSize];

	channels2DArray = new CHANNEL**[blocks2DArrayWH];
	for (size_t yIt = 0; yIt < blocks2DArrayWH; yIt++)
	{
		channels2DArray[yIt] = new CHANNEL*[blocks2DArrayWH];
	}

	for (size_t yIt = 0; yIt < blocks2DArrayWH; yIt++)
	{
		for (size_t xIt = 0; xIt < blocks2DArrayWH; xIt++)
		{
			channels2DArray[yIt][xIt] = nullptr;
		}
	}

	// Set channels2DArray pointers

	for (size_t yIt = 1; yIt < blocks2DArrayWH - 1; yIt++)
	{
		for (size_t xIt = 1; xIt < blocks2DArrayWH - 1; xIt++)
		{
			// if block isn't LUT or I/O
			if (blocks2DArray[yIt][xIt] == nullptr){
				unsigned int ID = (yIt - 1) * channels2DArrayWH + xIt - 1;
				channels2DArray[yIt][xIt] = &channelsGraph[ID];
				channels2DArray[yIt][xIt]->coords[0] = xIt;
				channels2DArray[yIt][xIt]->coords[1] = yIt;
				channels2DArray[yIt][xIt]->ID = ID;
			}
		}
	}

	// Set neighbours in each channel

	for (size_t yIt = 1; yIt < blocks2DArrayWH - 1; yIt++)
	{
		for (size_t xIt = 1; xIt < blocks2DArrayWH - 1; xIt++)
		{
			if (channels2DArray[yIt][xIt] != nullptr){
				if (channels2DArray[yIt + 1][xIt] != nullptr && yIt < blocks2DArrayWH - 2){
					channels2DArray[yIt][xIt]->baseNeighboursWeights.push_back(pair<unsigned int, float>(channels2DArray[yIt + 1][xIt]->ID, edgeWeight));
				}
				if (channels2DArray[yIt - 1][xIt] != nullptr && yIt != 1){
					channels2DArray[yIt][xIt]->baseNeighboursWeights.push_back(pair<unsigned int, float>(channels2DArray[yIt - 1][xIt]->ID, edgeWeight));
				}
				if (channels2DArray[yIt][xIt + 1] != nullptr && xIt < blocks2DArrayWH - 2){
					channels2DArray[yIt][xIt]->baseNeighboursWeights.push_back(pair<unsigned int, float>(channels2DArray[yIt][xIt + 1]->ID, edgeWeight));
				}
				if (channels2DArray[yIt][xIt - 1] != nullptr && xIt != 1){
					channels2DArray[yIt][xIt]->baseNeighboursWeights.push_back(pair<unsigned int, float>(channels2DArray[yIt][xIt - 1]->ID, edgeWeight));
				}
			}
		}
	}

	// Set channelCapacity

	for (size_t i = 0; i < graphSize; i++)
	{
		channelsGraph[i].channelCapacity = channelCapacity;
		if (channelsGraph[i].baseNeighboursWeights.size() > 2){ // Its switchbox
			channelsGraph[i].channelCapacity = (channelsGraph[i].baseNeighboursWeights.size() - 1) * channelCapacity;
		}
	}

	// Loop over all blocks

	for (size_t i = 0; i < blocksCount; i++)
	{
		auto& currentBlock = blocksArray[i];
		const auto& currentID = currentBlock.ID;
		const auto& currentXIdx = currentBlock.coords[0];
		const auto& currentYIdx = currentBlock.coords[1];
		const auto& currentDestBlocks = currentBlock.destBlocks;

		auto& currentChannelSources = currentBlock.channelsConnections.first;
		auto& currentChannelDests = currentBlock.channelsConnections.second;

		if (currentBlock.type == blockType::CLB)
		{
			currentChannelSources.push_back(channels2DArray[currentYIdx - 1][currentXIdx]->ID); // Bottom out pin
			currentChannelSources.push_back(channels2DArray[currentYIdx][currentXIdx + 1]->ID); // Right out pin
		}
		else if (currentBlock.type == blockType::INPUT){
			if (currentXIdx == 0) // Left side inputs
			{
				currentChannelSources.push_back(channels2DArray[currentYIdx][currentXIdx + 1]->ID);
			}
			else if (currentXIdx == maxWH){ // Right side inputs
				currentChannelSources.push_back(channels2DArray[currentYIdx][currentXIdx - 1]->ID);
			}
			else if (currentYIdx == 0){ // Bottom side inputs
				currentChannelSources.push_back(channels2DArray[currentYIdx + 1][currentXIdx]->ID);
			}
			else if (currentYIdx == maxWH){ // Top side inputs
				currentChannelSources.push_back(channels2DArray[currentYIdx - 1][currentXIdx]->ID);
			}
		}
		else if (currentBlock.type == blockType::OUTPUT){
			continue;
		}

		// Loop over all destination blocks

		for (size_t dstIt = 0; dstIt < currentDestBlocks.size(); dstIt++)
		{
			auto& currentDest = blocksArray[currentDestBlocks[dstIt]];
			const auto& currentDestID = currentDest.ID;
			const auto& currentDestXIdx = currentDest.coords[0];
			const auto& currentDestYIdx = currentDest.coords[1];

			// Loop over all inputs of destination block

			if (currentDest.type == blockType::CLB){
				for (size_t dstInpIt = 0; dstInpIt < 4; dstInpIt++)
				{
					if (currentDest.inpBlocks[dstInpIt] == currentID){
						switch (dstInpIt){
						case 0:
							currentChannelDests.push_back(channels2DArray[currentDestYIdx - 1][currentDestXIdx]->ID);
							break;
						case 1:
							currentChannelDests.push_back(channels2DArray[currentDestYIdx][currentDestXIdx - 1]->ID);
							break;
						case 2:
							currentChannelDests.push_back(channels2DArray[currentDestYIdx + 1][currentDestXIdx]->ID);
							break;
						case 3:
							currentChannelDests.push_back(channels2DArray[currentDestYIdx][currentDestXIdx + 1]->ID);
							break;
						}
						break;
					}
				}
			}
			else{ // If blockType == OUTPUT
				if (currentDestXIdx == 0) // Left side outputs
				{
					currentChannelSources.push_back(channels2DArray[currentDestYIdx][currentXIdx + 1]->ID);
				}
				else if (currentDestXIdx == maxWH){ // Right side outputs
					currentChannelSources.push_back(channels2DArray[currentDestYIdx][currentDestXIdx - 1]->ID);
				}
				else if (currentDestYIdx == 0){ // Bottom side outputs
					currentChannelSources.push_back(channels2DArray[currentDestYIdx + 1][currentDestXIdx]->ID);
				}
				else if (currentDestYIdx == maxWH){ // Top side outputs
					currentChannelSources.push_back(channels2DArray[currentDestYIdx - 1][currentDestXIdx]->ID);
				}
			}
		}
	}
}

vector<unsigned int> PATHFINDER::buildPath(const CHANNEL* finalGraph, const vector<unsigned int>& srcChannels, const unsigned int& dstChannel){
	vector<unsigned int> outPath;
	auto currentID = dstChannel;
	unsigned int fstSrc = srcChannels[0];
	int secSrc = -1;

	if (srcChannels.size() > 1)
	{
		secSrc = srcChannels[1];
	}

	while (currentID != fstSrc && currentID != secSrc)
	{
		auto prevChannelID = finalGraph[currentID].prevChannel;
		if (prevChannelID == -1)
		{
			//std::cerr << "Cant find path\n";
			break;
		}
		outPath.push_back(currentID);
		currentID = prevChannelID;
	}

	outPath.push_back(currentID);

	return outPath;
}

void PATHFINDER::dijkstra(const LUT_IO_BLOCK& currentBlock){

	// Initialization temp graph
	CHANNEL* tempGraph;
#ifdef _OPENMP
#pragma omp critical
{
#endif
	tempGraph = (CHANNEL*)malloc(sizeof(CHANNEL) * (graphSize + 1));
	if (tempGraph == nullptr){
		cerr << "Temporary graph memory allocation error" << endl;
		exit(1);
	}
#ifdef _OPENMP
}
#endif
	
	memcpy(tempGraph, channelsGraph, sizeof(CHANNEL) * (graphSize + 1));;

	const vector<unsigned int>& currentChannelDests = currentBlock.channelsConnections.second;
	unsigned int initChannel = currentBlock.channelsConnections.first[0]; // Source channel = bottom pinout

	// Set destinations in graph by connections[1...end()]

	for (size_t i = 0; i < currentChannelDests.size(); ++i) tempGraph[currentChannelDests[i]].itsDestination = 1;

	// Main algorithm

	unsigned int destinationsCount = currentChannelDests.size();
	priority_queue<CHANNEL*, vector<CHANNEL*>, channelComp> queueOfChannels;

	tempGraph[initChannel].prevChannel = initChannel;
	tempGraph[initChannel].minWeight = 0;
	if (currentBlock.type == blockType::CLB)
	{
		auto rightPinChannel = currentBlock.channelsConnections.first[0];
		tempGraph[rightPinChannel].prevChannel = rightPinChannel;
		tempGraph[rightPinChannel].minWeight = 0;
	}

	do
	{
		if (queueOfChannels.size() > 0)
		{
			initChannel = queueOfChannels.top()->ID;
			queueOfChannels.pop();
			if (tempGraph[initChannel].itsDestination){
				destinationsCount--;
				tempGraph[initChannel].itsDestination = 0;
				tempGraph[initChannel].minWeight = 0;
				if (destinationsCount == 0) break;
			}
		}
		tempGraph[initChannel].used = 1;

		for (size_t i = 0; i < tempGraph[initChannel].baseNeighboursWeights.size(); ++i)
		{
			unsigned int currentNeighborId = tempGraph[initChannel].baseNeighboursWeights[i].first;
			if (tempGraph[currentNeighborId].used == 0){
				if (!tempGraph[currentNeighborId].inQueue) queueOfChannels.push(&channelsGraph[currentNeighborId]);				
				tempGraph[currentNeighborId].inQueue = 1;

				float Cn = tempGraph[currentNeighborId].getWeightToThisChannel(tempGraph[initChannel].baseNeighboursWeights[i].second); // tempGraph[initChannel].baseNeighboursWeights[i].second : base graph weight(edge weight)
				if (tempGraph[currentNeighborId].minWeight > tempGraph[initChannel].minWeight + Cn)
				{
					tempGraph[currentNeighborId].minWeight = tempGraph[initChannel].minWeight + Cn;
					tempGraph[currentNeighborId].prevChannel = initChannel;
				}
			}
		}

	} while (queueOfChannels.size() != 0);

	// Add channels to wire(subgraph or path of source channel and destination channels)

	vector<vector<unsigned int> > tempPath;
	for (size_t i = 0; i < currentChannelDests.size(); ++i)
	{
		tempPath.emplace_back(buildPath(tempGraph, currentBlock.channelsConnections.first, currentChannelDests[i]));
	}
	routedChannels[currentBlock.ID] = tempPath;

	free(tempGraph);
}

void PATHFINDER::pathfinder(const float& FvhParam, const float& FvpParam, const size_t& maxIter){
	Fvh = FvhParam;
	Fvp = FvpParam;
	chrono::time_point<std::chrono::system_clock> start, end;
	double fullTime = 0;
	unsigned int sumOfOccupancys = 0;
	//currentMaxOccupancy = 0;
	//cout << "iteration,maxPathLength,maxOccupancy,averagePathLength" << endl;
	for (size_t gIt = 1; gIt <= maxIter; ++gIt)
	{
		vector<unordered_set<unsigned int>> usedUniqueChannels;
		usedUniqueChannels.reserve(blocksCount);
		usedUniqueChannels.resize(blocksCount);
		unsigned int tempMaxOccupancy = 0;
		size_t pathsLengthSum = 0;
		size_t pathsCount = 0;
		update = false;
		maxPathLength = 0;
		start = std::chrono::system_clock::now();
		routedChannels.reserve(blocksCount);
		routedChannels.resize(blocksCount);

#pragma omp parallel
		{		

			// Loop over all multi terminal wires(connections)

#pragma omp for
			for (int blockIt = 0; blockIt < blocksCount; ++blockIt)
			{
				if (blocksArray[blockIt].type != blockType::OUTPUT && blocksArray[blockIt].channelsConnections.second.size()){
					dijkstra(blocksArray[blockIt]);
				}
			}

			// Clear channelOccupancy in each channel after first iteration

			if (gIt > 1){
#pragma omp for
				for (int i = 0; i <= graphSize; ++i){ channelsGraph[i].channelOccupancy = 0; }
			}

#pragma omp for
			for (int i = 0; i < routedChannels.size(); ++i){
#pragma omp atomic
				pathsCount += routedChannels[i].size();
				for (int j = 0; j < routedChannels[i].size(); ++j){
#pragma omp atomic
					pathsLengthSum += routedChannels[i][j].size();
#pragma omp critical
					maxPathLength = max(maxPathLength, routedChannels[i][j].size());
					usedUniqueChannels[i].insert(routedChannels[i][j].begin(), routedChannels[i][j].end());
				}
			}

			// Loop over all used channels to increase channelOccupancy in each used channel

#pragma omp for
			for (int i = 0; i < usedUniqueChannels.size(); ++i){

				for (auto it = usedUniqueChannels[i].begin(); it != usedUniqueChannels[i].end(); it++)
				{
#pragma omp atomic
					channelsGraph[*it].channelOccupancy++;
#pragma omp critical
					{
						//if (channelsGraph[*it].baseNeighboursWeights.size() < 3){
							tempMaxOccupancy = max(tempMaxOccupancy, channelsGraph[*it].channelOccupancy);
						//}
					}
				}
			}

			// Loop over all channels to update occupancyHustory and occupancyMult in each used channel

#pragma omp for
			for (int i = 0; i < graphSize; ++i){
#pragma omp critical
				{
					sumOfOccupancys += channelsGraph[i].channelOccupancy;
					channelsGraph[i].setHv(gIt);
					channelsGraph[i].setPv(gIt);
				}
			}
		}

		currentMaxOccupancy = tempMaxOccupancy;
		tempMaxOccupancy = 0;
		averageOccupancy = sumOfOccupancys / graphSize;
		sumOfOccupancys = 0;
		update = true;
		end = std::chrono::system_clock::now();
		chrono::duration<double> elapsed_seconds = end - start;
		fullTime += elapsed_seconds.count();
		cout << setw(10) << gIt << "," << maxPathLength << "," << currentMaxOccupancy << "," << averageOccupancy << endl;
		routedChannels.clear();
	}
	
	//cout << "Time: " << fullTime << "s. Iteration time: " << fullTime / maxIter << "s." << endl;
}