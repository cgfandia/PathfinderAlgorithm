#include <vector>
#include <queue>
#include <deque>
#include <set>
#include <map>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <cfloat>
using namespace std;
bool directionalGraph = true;
float Fvp = 0.1f;
float Fvh = 0.1f;
unsigned int maxNodeCapacity = 10;
class NODE{
public:
	bool used;
	unsigned int ID;
	unsigned int nodeCapacity;
	unsigned int nodeOccupancy;
	float occupancyHistory;
	float minDistance;
	unsigned int prevNode;
	map<unsigned int , unsigned int> neighbors; // neighbor ID <-> neighbor weight
	NODE() : used(false) , minDistance(FLT_MAX), nodeOccupancy(0), nodeCapacity(maxNodeCapacity), prevNode(0) {}
	void setId(unsigned int Id){
		ID = Id;
	}
	inline float Pv(unsigned int Niter){
		return 1 + (Niter - 1) * Fvp * nodeOccupancy;
	}
	inline float Hv(unsigned int Niter){
		if (Niter == 1)
		{
			return occupancyHistory = 1;
		}else{
			return occupancyHistory = occupancyHistory + Fvh * nodeOccupancy;
		}
	}
};
//void initGraph(vector<NODE>* emptyGraph , const unsigned int** initTable , unsigned int tableSize){
void initGraph(NODE* emptyGraph , const unsigned int initTable[][3] , const size_t& tableSize){
	for (size_t i = 0 ; i < tableSize ; ++i)
	{
		emptyGraph[initTable[i][0]].setId(initTable[i][0]);
		emptyGraph[initTable[i][0]].neighbors.insert(pair<unsigned int , unsigned int>(initTable[i][1] , initTable[i][2])); // Insert to node its neighbor and neighbor's weight
		if (!directionalGraph)
		{
			emptyGraph[initTable[i][1]].setId(initTable[i][1]);
			emptyGraph[initTable[i][1]].neighbors.insert(pair<unsigned int , unsigned int>(initTable[i][0] , initTable[i][2]));
		}
	}
}
void buildPath(const NODE* finalGraph , const unsigned int& srcNode , const unsigned int& dstNode , set<unsigned int>& outPath){
	unsigned int tempId = dstNode;
	while (tempId != srcNode)
	{
		outPath.insert(tempId);
		tempId = finalGraph[tempId].prevNode;
		if (tempId == 0)
		{
			std::cout << "Cant find path\n";
			break;
		}
	}
	outPath.insert(srcNode);
}
void dijkstra(const NODE* nodesGraph , const unsigned int& graphSize, vector<vector<unsigned int> >::const_iterator connections, const unsigned int& iterN, set<unsigned int>& outPath ){

// Init temp graph
	NODE* tempGraph = (NODE*)malloc(sizeof(NODE) * ( graphSize + 1 ));
	if(tempGraph == nullptr){
		cout << "Temporary graph memory allocation error" << endl;
		exit(1);
	}
	memcpy(tempGraph , nodesGraph , sizeof(NODE) * ( graphSize + 1 ));

	unsigned int initNode = connections->at(0); // Source node = connections[0]
	queue<unsigned int> queueOfNodes;
	
	tempGraph[initNode].prevNode = initNode;
	tempGraph[initNode].minDistance = 0;
	int Niter = 1;
	do 
	{
		tempGraph[initNode].used = true;

		for (auto neighborsIt = tempGraph[initNode].neighbors.begin() ; neighborsIt != tempGraph[initNode].neighbors.end() ; ++neighborsIt)
		{
			unsigned int currentNeighborId = neighborsIt->first;
			if(!tempGraph[currentNeighborId].used){
				queueOfNodes.push(currentNeighborId);
				unsigned int bn = neighborsIt->second; // base graph weight(edge weight)
				float Pv = tempGraph[currentNeighborId].Pv(Niter);
				float Hv = tempGraph[currentNeighborId].Hv(Niter);
				float Cn = Pv * (bn + Hv);
				if (tempGraph[currentNeighborId].minDistance > tempGraph[initNode].minDistance + Cn)
				{
					tempGraph[currentNeighborId].minDistance = tempGraph[initNode].minDistance + Cn;
					tempGraph[currentNeighborId].prevNode = initNode;
				}
			}
		}

		initNode = queueOfNodes.front();
		queueOfNodes.pop();
	} while (queueOfNodes.size() != 0);

// Add nodes to wire(subgraph or path of source node and destination nodes)
	for (size_t i = 1 ; i < connections->size() ; ++i)
	{
		buildPath(tempGraph , connections->at(0) , connections->at(i) , outPath);
	}
	for_each(outPath.begin() , outPath.end() , [](unsigned int i){cout << i << " ";});
	cout << endl;
	free(tempGraph);
}
void pathfinder(NODE* nodesGraph, const unsigned int& maxIter , const unsigned int& graphSize, const vector<vector<unsigned int> >* connectionsList){
	for (size_t i = 0 ; i < maxIter ; ++i)
	{

// Clear nodeOccupancy in all nodes 
		if (i > 0){
			for (NODE* graphIt = nodesGraph ; graphIt < graphSize + nodesGraph; ++graphIt){ graphIt->nodeOccupancy = 0; }
		}

// Loop over all multi terminal wires(connections)
		vector<unsigned int> usedNodes; 
		for (auto cListIt = (*(connectionsList)).begin() ; cListIt != (*(connectionsList)).end() ; ++cListIt)
		{
			set<unsigned int> outPath;
			dijkstra(nodesGraph,graphSize,cListIt,i,outPath);
			//copy(outPath.begin() , outPath.end() , usedNodes.end() - 1); // Copy nodes from path to collections of used nodes
			usedNodes.insert(usedNodes.end() , outPath.begin() , outPath.end()); // Copy nodes from path to collections of used nodes
		}

// Loop over all used nodes to increase nodeOccupancy in each node
		for_each(usedNodes.begin() , usedNodes.end() , [nodesGraph](unsigned int uNode){ nodesGraph[uNode].nodeOccupancy++; });

// Loop over all used nodes to update occupancyHustory in each node
		for_each(usedNodes.begin() , usedNodes.end() , [nodesGraph , i](unsigned int uNode){ nodesGraph[uNode].Hv(i); });
	}
}
int main(int argc , char* argv[]){

	size_t tableSize = 10;
	size_t graphSize = 9;
	NODE* nodesGraph = new NODE[graphSize + 1];
	unsigned int** initTable;
	initTable = (unsigned int **)malloc(tableSize * sizeof(unsigned int *));
	if(initTable == nullptr){
		cout << "Initial table memory allocation error" << endl;
		exit(1);
	}

	for (size_t i = 0 ; i < tableSize ; ++i)
		initTable[i] = (unsigned int*)malloc(3 * sizeof(unsigned int));

	unsigned int testTable[10][3] = { {1 , 4 , 2},
									{1 , 5 , 1},
									{2 , 5 , 2},
									{2 , 6 , 1},
									{3 , 6 , 1},
									{4 , 7 , 2},
									{5 , 7 , 1},
									{5 , 8 , 2},
									{6 , 8 , 1},
									{6 , 9 , 1} }; // first node , sec node , edge weight(bn)
	unsigned int connections[3][2] = { {1 , 7},
									{2 , 8},
									{3 , 9} };
	vector<vector<unsigned int> > connectionsList;

	// connectionsList init
	connectionsList.resize(3);
	connectionsList.reserve(3);
	for (size_t i = 0 ; i < 3 ; ++i)
	{
		for (size_t j = 0 ; j < 2 ; ++j)
		{
			connectionsList[i].push_back(connections[i][j]);
		}
	}

	initGraph(nodesGraph , testTable , tableSize);
	pathfinder(nodesGraph , 1 , graphSize , &connectionsList);
	return 0;
}