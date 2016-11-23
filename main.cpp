#include <vector>
#include <queue>
#include <deque>
#include <map>
#include <cstring>
#include <cstdlib>
#include <iostream>
using namespace std;
class NODE{
public:
	bool used;
	unsigned int ID;
	unsigned int edgeWeight;
	unsigned int minDistance;
	unsigned int prevNode;
	map<unsigned int , unsigned int> neighbors; // neighbor ID <-> neighbor weight
	NODE() : used(false) , minDistance(0xFFFFFFFF) {}
	void setIdWeight(unsigned int Id , unsigned int Weight){
		ID = Id;
		edgeWeight = Weight;
	}
};
//void initGraph(vector<NODE>* emptyGraph , const unsigned int** initTable , unsigned int tableSize){
void initGraph(NODE* emptyGraph , const unsigned int initTable[][3] , const size_t& tableSize){
	for (size_t i = 0 ; i < tableSize ; ++i)
	{
		emptyGraph[initTable[i][0]].setIdWeight(initTable[i][0] , initTable[i][2]);
		emptyGraph[initTable[i][0]].neighbors.insert(pair<unsigned int , unsigned int>(initTable[i][1] , initTable[i][2]));

		emptyGraph[initTable[i][1]].setIdWeight(initTable[i][1] , initTable[i][2]);
		emptyGraph[initTable[i][1]].neighbors.insert(pair<unsigned int , unsigned int>(initTable[i][0] , initTable[i][2]));
	}
}
void dijkstra(NODE* tempGraph , const unsigned int fstNode ,  size_t elementsNumber ){
	unsigned int initNode = fstNode;
	queue<unsigned int> queueOfNodes;
	
	tempGraph[initNode].prevNode = initNode;
	tempGraph[initNode].minDistance = 0;

	do 
	{
		tempGraph[initNode].used = true;

		for (auto neighborsIt = tempGraph[initNode].neighbors.begin() ; neighborsIt != tempGraph[initNode].neighbors.end() ; ++neighborsIt)
		{
			unsigned int currentNeighborId = neighborsIt->first;
			if(!tempGraph[currentNeighborId].used){
				queueOfNodes.push(currentNeighborId);
				unsigned int weight = neighborsIt->second;
				if (tempGraph[currentNeighborId].minDistance > tempGraph[initNode].minDistance + weight)
				{
					tempGraph[currentNeighborId].minDistance = tempGraph[initNode].minDistance + weight;
					tempGraph[currentNeighborId].prevNode = initNode;
				}
			}
		}

		initNode = queueOfNodes.front();
		queueOfNodes.pop();
	} while (queueOfNodes.size() != 0);
}
void buildPath(const NODE* finalGraph , const unsigned int& fstNode , const unsigned int& lastNode , deque<unsigned int>& outPath){
	unsigned int tempId = lastNode;
	while (tempId != fstNode)
	{
		outPath.push_front(tempId);
		tempId = finalGraph[tempId].prevNode;
	}
	outPath.push_front(fstNode);
}
int main(int argc , char* argv[]){

	unsigned int fstNode = 1;
	unsigned int lstNode = 5;
	size_t tableSize = 9;
	size_t elementsNumber = 6;
	NODE* nodesGraph = new NODE[elementsNumber + 1];
	deque<unsigned int> outPath;
	unsigned int** initTable;
	initTable = (unsigned int **)malloc(tableSize * sizeof(unsigned int *));
	if(initTable == nullptr){
		cout << "Inital table memory allocation error" << endl;
		exit(1);
	}

	for (size_t i = 0 ; i < tableSize ; ++i)
		initTable[i] = (unsigned int*)malloc(3 * sizeof(unsigned int));

	unsigned int testTable[9][3] = { {1 , 6 , 14},
									{1 , 3 , 9},
									{1 , 2 , 7},
									{6 , 3 , 2},
									{6 , 5 , 9},
									{3 , 4 , 11},
									{2 , 3 , 10},
									{2 , 4 , 15},
									{4 , 5 , 6} }; // first node , sec node , edge weight

	initGraph(nodesGraph , testTable , tableSize);

	NODE* tempGraph = (NODE*)malloc(sizeof(NODE) * ( elementsNumber + 1 ));
	if(tempGraph == nullptr){
		cout << "Temporary graph memory allocation error" << endl;
		exit(1);
	}
	memcpy(tempGraph , nodesGraph , sizeof(NODE) * ( elementsNumber + 1 ));

	dijkstra(tempGraph , fstNode , elementsNumber);
	buildPath(tempGraph , fstNode , lstNode , outPath);
	for (auto it = outPath.begin() ; it != outPath.end() ; ++it)
	{
		cout << *it << " ";
	}	cout << endl;
	return 0;
}