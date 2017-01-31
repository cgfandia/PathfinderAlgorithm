#include <vector>
#include <queue>
#include <deque>
#include <set>
#include <map>
#include <cstring>
#include <cstdlib>
#include <iostream>
using namespace std;
bool directionalGraph = true;
float Fvp = 0.1;
float Fvh = 0.1;
unsigned int maxNodeCapacity = 10;
class NODE{
public:
	bool used;
	unsigned int ID;
	unsigned int nodeCapacity;
	unsigned int nodeOccupancy;
	unsigned int occupancyHistory;
	unsigned int minDistance;
	unsigned int prevNode;
	map<unsigned int , unsigned int> neighbors; // neighbor ID <-> neighbor weight
	NODE() : used(false) , minDistance(0xFFFFFFFF), nodeOccupancy(0), nodeCapacity(maxNodeCapacity), prevNode(0) {}
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
			return occupancyHistory + Fvh * nodeOccupancy;
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
void buildPath(const NODE* finalGraph , const unsigned int& srcNode , const unsigned int& dstNode , deque<unsigned int>& outPath){
	unsigned int tempId = dstNode;
	while (tempId != srcNode)
	{
		outPath.push_front(tempId);
		tempId = finalGraph[tempId].prevNode;
		if (tempId == 0)
		{
			std::cout << "Cant find path\n";
			break;
		}
	}
	outPath.push_front(srcNode);
}
void dijkstra(const NODE* nodesGraph , const unsigned int& graphSize, const vector<int>& connections, const unsigned int& iterN ){

// Init temp graph
	NODE* tempGraph = (NODE*)malloc(sizeof(NODE) * ( graphSize + 1 ));
	if(tempGraph == nullptr){
		cout << "Temporary graph memory allocation error" << endl;
		exit(1);
	}
	memcpy(tempGraph , nodesGraph , sizeof(NODE) * ( graphSize + 1 ));

	unsigned int initNode = connections[0]; // Source node = connections[0]
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

// Build path
	deque<unsigned int> outPath;
	buildPath(tempGraph , 1 , 9 , outPath);
	for (auto it = outPath.begin() ; it != outPath.end() ; ++it)
	{
		cout << *it << " ";
	}	cout << endl;
	free(tempGraph);
}
void pathfinder(NODE* nodesGraph, const unsigned int& maxIter , const unsigned int& graphSize, const vector<vector<int> >& connectionsList){
	for (size_t i = 0 ; i < maxIter ; ++i)
	{

	}
}
int main(int argc , char* argv[]){

	unsigned int fstNode = 1;
	unsigned int lstNode = 5;
	size_t tableSize = 10;
	size_t graphSize = 9;
	NODE* nodesGraph = new NODE[graphSize + 1];
	unsigned int** initTable;
	initTable = (unsigned int **)malloc(tableSize * sizeof(unsigned int *));
	if(initTable == nullptr){
		cout << "Inital table memory allocation error" << endl;
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
	vector<vector<int> > connectionsList;

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
	dijkstra(nodesGraph , fstNode, graphSize);
	return 0;
}