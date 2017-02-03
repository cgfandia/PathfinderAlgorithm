#if !defined( PATHFINDER_H )
#define PATHFINDER_H

#include <map>
#include <vector>
#include <set>
#include <cfloat>
using namespace std;

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
	NODE();
	inline float Pv(unsigned int);
	inline float Hv(unsigned int);
};

class PATHFINDER
{
public:
	bool directionalGraph;
	size_t graphSize; // Max ID of nodes
	size_t maxIter; // Max iterations of pathfinder algorithm
	NODE* nodesGraph;
	vector<vector<unsigned int> > connectionsList;
	void initGraphAndConnections( const string& , const string& , bool);
	void buildPath( const NODE* , const unsigned int& , const unsigned int& , set<unsigned int>& );
	void dijkstra( const vector<unsigned int>* , const unsigned int& , set<unsigned int>& );
	void pathfinder(const float& , const float& , const size_t&);
};
#endif