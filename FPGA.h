#ifndef FPGA_H 
#define FPGA_H

#include <vector>
#include <unordered_set>
#include <unordered_map>
using namespace std;

class LUT_IO_BLOCK{
public:
	string name;
	unsigned int ID;
	unsigned int coords[2]; // 0 - x, 1 -y
	vector<unsigned int> destBlocks;
	LUT_IO_BLOCK(const string& nameParam, const unsigned int& XParam, const unsigned int& YParam, const unsigned int& IDParam);
	LUT_IO_BLOCK();
};

class FPGA{
public:
	size_t blocksCount;
	size_t blockArrayWH;
	unordered_map<string, LUT_IO_BLOCK> LUTsAndIO; // name of LUT or I/O block <-> LUT or I/O block
	LUT_IO_BLOCK* blocksArray;
	~FPGA();
	void initFPGA(const string& placeFile, const string& netsFile);
private:
	void parsePlaceFile(const string& filename);
	void parseNetsFile(const string& filename);
};

#endif