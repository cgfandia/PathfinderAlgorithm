#ifndef FPGA_H 
#define FPGA_H

#include <vector>
#include <unordered_map>
using namespace std;

__declspec(align(64)) class CHANNEL{
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
	inline float getWeightToThisChannel(const float&) const;
};

struct CHANNEL_TEMP{
public:
	bool itsDestination;
	bool used;
	bool inQueue;
	int prevChannel;
	float minWeight;
};

enum class blockType{ CLB, OUTPUT, INPUT };

class LUT_IO_BLOCK{
public:
	string name;
	unsigned int ID;
	blockType type;
	unsigned int coords[2]; // 0 - x, 1 -y
	long long int inpBlocks[4];
	vector<unsigned int>destBlocks;
	vector<vector<unsigned int> >* channelsPaths;
	pair<vector<unsigned int>, vector<unsigned int> > channelsConnections; // pair<vector of src, vector of dest>
	LUT_IO_BLOCK(const string& nameParam, const unsigned int& XParam, const unsigned int& YParam, const unsigned int& IDParam);
	LUT_IO_BLOCK();

	~LUT_IO_BLOCK() = default;
};

class FPGA{
public:
	unsigned int blocksCount;
	size_t blocks2DArrayWH;
	unsigned int maxWH;
	unordered_map<string, LUT_IO_BLOCK> LUTsAndIO; // name of LUT or I/O block <-> LUT or I/O block
	LUT_IO_BLOCK* blocksArray;
	LUT_IO_BLOCK*** blocks2DArray;
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
	unsigned int currentMaxOccupancy;
	size_t channels2DArrayWH;
	CHANNEL*** channels2DArray;
	void init(const string& placeFile, const string& netsFile);
	void pathfinder(const float& FvhParam, const float& FvpParam, const size_t& maxIter);
	virtual ~PATHFINDER();
private:
	size_t graphSize; // Max ID of channels
	CHANNEL* channelsGraph;
	CHANNEL_TEMP** channelsTempMemoryPool;
	vector<vector<vector<unsigned int> > > routedChannels;
	bool directionalGraph;
	vector<unsigned int> buildPath(const CHANNEL_TEMP*, const vector<unsigned int>&, const unsigned int&);
	void dijkstra(const unsigned int&, const unsigned int&);
};

class channelComp
{
public:
	bool operator() (const CHANNEL* lhs, const CHANNEL* rhs) const
	{
		return (lhs->minWeight > rhs->minWeight);
	}
};
#endif