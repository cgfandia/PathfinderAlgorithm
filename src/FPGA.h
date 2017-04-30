#ifndef FPGA_H 
#define FPGA_H

#include <vector>
#include <unordered_map>
using namespace std;

class CHANNEL{
public:
	bool itsDestination;
	bool used;
	bool inQueue;
	unsigned int ID;
	unsigned int channelCapacity;
	unsigned int channelOccupancy;
	unsigned int coords[2]; // 0 - x, 1 -y
	long long int prevChannel;
	float occupancyHistory;
	float occupancyMult;
	float minWeight;
	vector<pair<unsigned int, float> > baseNeighboursWeights; // neighbor ID <-> base neighbor weight
	CHANNEL();
	inline void setPv(const size_t&);
	inline void setHv(const size_t&);
	inline float getWeightToThisChannel(const float&);
};

enum class blockType{ CLB, OUTPUT, INPUT };

class CLB_IO{
public:
	string name;
	unsigned int ID;
	blockType type;
	unsigned int coords[2]; // 0 - x, 1 -y
	long long int inpBlocks[4];
	vector<unsigned int>destBlocks;
	vector<vector<unsigned int> >* channelsPaths;
	pair<vector<unsigned int>, vector<unsigned int> > channelsConnections; // pair<vector of src, vector of dest>
	CLB_IO(const string& nameParam, const unsigned int& XParam, const unsigned int& YParam, const unsigned int& IDParam);
	CLB_IO();

	~CLB_IO() = default;
};

class FPGA{
public:
	unsigned int blocksCount;
	size_t blocks2DArrayWH;
	unsigned int maxWH;
	unordered_map<string, CLB_IO> LUTsAndIO; // name of CLB or I/O block <-> CLB or I/O block
	CLB_IO* blocksArray;
	CLB_IO*** blocks2DArray;
	virtual ~FPGA();
	void initFPGA(const string& placeFile, const string& netsFile);
private:
	void init2DArrayBlocks();
	void parsePlaceFile(const string& filename);
	void parseNetsFile(const string& filename);
};

class PATHFINDER : public FPGA
{
public:
	bool update;
	size_t maxPathLength;
	unsigned int channelCapacity;
	unsigned int MaxOccupancy;
	unsigned int currentMaxOccupancy;
	unsigned int averageOccupancy;
	size_t channels2DArrayWH;
	CHANNEL*** channels2DArray;
	void init(const string& placeFile, const string& netsFile, const float& edgeWeightParam, const float& channelCapacityParam);
	void pathfinder(const float& FvhParam, const float& FvpParam, const size_t& maxIter);
	virtual ~PATHFINDER();
private:
	size_t graphSize; // Max ID of channels
	CHANNEL* channelsGraph;
	vector<vector<vector<unsigned int> > > routedChannels;
	bool directionalGraph;
	vector<unsigned int> buildPath(const CHANNEL*, const vector<unsigned int>&, const unsigned int&);
	void dijkstra(const CLB_IO&);
};

class channelComp
{
public:
	bool operator() (const pair<float, CHANNEL*> lhs, const pair<float, CHANNEL*> rhs) const
	{
		return lhs.first > rhs.first;
	}
};
#endif