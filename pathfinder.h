#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <vector>
#include <unordered_set>
#include <unordered_map>
using namespace std;

__declspec(align(64)) class CHANNEL{
public:
	unsigned int itsDestination;
	unsigned int ID;
	unsigned int channelCapacity;
	unsigned int channelOccupancy;
	unsigned int used;
	float occupancyHistory;
	float occupancyMult;
	float minWeight;
	unsigned int prevChannel;
	unsigned int padding[4];
	vector<pair<unsigned int , float> > baseNeighboursWeights; // neighbor ID <-> base neighbor weight
	CHANNEL();
	inline void setPv(const size_t&);
	inline void setHv(const size_t&);
	inline float getWeightToThisChannel(const float& );
};

class PATHFINDER
{
public:
	size_t graphSize; // Max ID of channels
	vector<vector<unsigned int> > connectionsList;
	vector<vector<unsigned int> >* routedPaths;
	void initGraphAndConnections( const string& graphPath, const string& connectionsPath, bool directionalGraphFlag);
	void pathfinder(const float& FvhParam, const float& FvpParam, const size_t& maxIter);
	~PATHFINDER();
private:
	CHANNEL* channelsGraph;
	bool directionalGraph;
	bool buildPath(const CHANNEL*, const unsigned int&, const unsigned int&, vector<unsigned int>*);
	void dijkstra(const unsigned int&, const int&, unordered_set<unsigned int>*);
};
#endif