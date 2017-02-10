#if !defined( PATHFINDER_H )
#define PATHFINDER_H

#include <map>
#include <vector>
#include <set>
using namespace std;

__declspec(align(64)) class NODE{
public:
	unsigned int itsDestination;
	unsigned int ID;
	unsigned int nodeCapacity;
	unsigned int nodeOccupancy;
	unsigned int used;
	float occupancyHistory;
	float occupancyMult;
	float minWeight;
	unsigned int prevNode;
	//unsigned int padding[4];
	vector<pair<unsigned int , float> > baseNeighboursWeights; // neighbor ID <-> base neighbor weight
	NODE();
	inline void setPv(const size_t&);
	inline void setHv(const size_t&);
	inline float getWeightToThisNode(const float& );
};

class PATHFINDER
{
public:
	bool directionalGraph;
	size_t graphSize; // Max ID of nodes
	size_t maxIter; // Max iterations of pathfinder algorithm
	NODE* nodesGraph;
	vector<vector<unsigned int> > connectionsList;
	vector<vector<unsigned int> >* routedPaths;
	void initGraphAndConnections( const string& graphPath, const string& connectionsPath, bool directionalGraphFlag);
	void buildPath( const NODE* , const unsigned int& , const unsigned int& , vector<unsigned int>* );
	void dijkstra(const unsigned int&, const int&, set<unsigned int>*);
	void pathfinder(const float& FvhParam, const float& FvpParam, const size_t& maxIter);
};
#endif