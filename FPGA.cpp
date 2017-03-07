#include <cstring>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <regex>
#include "FPGA.h"

void checkFileOpening(const ifstream& file, const string& filename){
	if (!file)
	{
		throw runtime_error(string("Failed to open ") + filename);
	}
}

LUT_IO_BLOCK::LUT_IO_BLOCK(const string& nameParam, const unsigned int& XParam, const unsigned int& YParam, const unsigned int& IDParam){
	name = nameParam;
	ID = IDParam;
	coords[0] = XParam;
	coords[1] = YParam;
	memset(inpBlocks, -1, sizeof(int) * 4);
}

LUT_IO_BLOCK::LUT_IO_BLOCK(){
	name = "none";
	ID = 0xFFFFFFFF;
	coords[0] = 0;
	coords[1] = 0;
	type = blockType::CLB;
	memset(inpBlocks, -1, sizeof(int) * 4);
}

FPGA::~FPGA(){
	delete[] blocksArray;
	blocksArray = nullptr;
	for (size_t i = 0; i < blocks2DArrayWH; i++)
	{
		delete[] blocks2DArray[i];
		blocks2DArray[i] = nullptr;
	}
	delete[] blocks2DArray;
	blocks2DArray = nullptr;
}


void FPGA::parsePlaceFile(const string& filename){
	blocks2DArrayWH = 0;
	ifstream file(filename, ifstream::in);
	checkFileOpening(file, filename);
	regex arraySizeRegEx("Array size: (\\d+) x (\\d+) logic blocks"); // Array size: 33 x 33 logic blocks
	regex blockInfoStringRegEx("(\\S+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+#(\\d+)"); // block_name x y subblk block number (i_7_ 18 34 0 #0)
	smatch matches; // store matched elements in string by regexp
	string lineString; // buffer for read line from file
	blocksCount = 0;
	while (!file.eof())
	{
		getline(file, lineString);
		if (regex_match(lineString, matches, blockInfoStringRegEx)){
			string name = matches[1];
			unsigned int X = stoi(matches[2]) * 2;
			unsigned int Y = stoi(matches[3]) * 2;
			unsigned int ID = stoi(matches[5]);
			LUTsAndIO.insert(pair<string, LUT_IO_BLOCK>(name, LUT_IO_BLOCK(name, X, Y, ID)));
			blocksCount = max(blocksCount, ID); // find biggest ID
		}
		else if (regex_match(lineString, matches, arraySizeRegEx)){
			maxWH = (stoi(matches[1]) + 1) * 2;
			blocks2DArrayWH = (stoi(matches[1]) + 2) * 2;
		}
	}
	file.close();
	blocksCount++;
	if (blocks2DArrayWH == 0){
		throw logic_error("Can't define logic blocks array size");
	}
}

void FPGA::parseNetsFile(const string& filename){
	ifstream file(filename, ifstream::in);
	checkFileOpening(file, filename);
	//regex globalRegEx("\.global\\s+(\\S+)\\s*$"); // .global clk_name # Don't route clk net.
	regex inputRegEx("\.input\\s+(\\S+)\\s*$"); // .input my_input_name
	regex outputRegEx("^\.output\\s+(\\S+)\\s*$"); // .output my_out_name
	regex pinlistIORegEx("^pinlist:\\s+(\\S+)\\s*$"); // pinlist: my_some_net_name
	regex pinlistRegEx("^pinlist:\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+).*$"); // pinlist:  in_a in_b in_c in_d out_net clk
	regex clbRegEx("^\.clb\\s+(\\S+)\\s+.*$"); // .clb my_logic_block_name
	smatch matches; // store matched elements in string by regexp
	string lineString; // buffer for read line from file
	while (!file.eof())
	{
		getline(file, lineString);
		if (regex_match(lineString, matches, clbRegEx)){
			string currentClbBlockName = matches[1];

			auto& currentClb = LUTsAndIO[currentClbBlockName];
			currentClb.type = blockType::CLB;

			getline(file, lineString);
			if (regex_match(lineString, matches, pinlistRegEx))
			{
				// for all pins: find in map LUT by its name and add to this LUT ID of clb block
				for (size_t i = 1; i < 5; i++)
				{
					if (matches[i] != "open"){
						auto& inpClb = LUTsAndIO[matches[i]];
						currentClb.inpBlocks[i - 1] = inpClb.ID;
						inpClb.destBlocks.push_back(currentClb.ID);
					}
				}
			}
			else{
				throw logic_error(string("Can't find pinlist for ") + currentClbBlockName);
			}
		}else if (regex_match(lineString, matches, outputRegEx)){
			string currentBlockName(matches[1]);
			auto& currentBlock = LUTsAndIO[matches[1]];
			currentBlock.type = blockType::OUTPUT;
			auto currentBlockID = currentBlock.ID;

			getline(file, lineString); // pinlist string
			if (regex_match(lineString, matches, pinlistIORegEx))
			{
				// find in map LUT by its name, which bound with input block, and add to this LUT ID of input block
				LUTsAndIO[matches[1]].destBlocks.push_back(currentBlockID);
			}
			else{
				throw logic_error(string("Can't find pinlist for ") + currentBlockName);
			}
		}
		else if (regex_match(lineString, matches, inputRegEx)){
			string currentBlockName(matches[1]);
			auto& currentBlock = LUTsAndIO[matches[1]];
			currentBlock.type = blockType::INPUT;
			auto currentBlockID = currentBlock.ID;

			/*getline(file, lineString); // pinlist string
			if (regex_match(lineString, matches, pinlistIORegEx))
			{
				// find in map LUT by its name, which bound with input block, and add to this LUT ID of input block
				LUTsAndIO[matches[1]].channelsConnections.second.push_back(currentBlockID);
			}
			else{
				throw logic_error(string("Can't find pinlist for ") + currentBlockName);
			}*/
		}
	}
	file.close();
}

void FPGA::init2DArrayBlocks(){

	// Init blocksArray from LUTsAndIO unordered_map

	blocksArray = new LUT_IO_BLOCK[blocksCount];
	for (auto blockIt = LUTsAndIO.begin(); blockIt != LUTsAndIO.end(); blockIt++)
	{
		blocksArray[blockIt->second.ID] = blockIt->second;
	}
	LUTsAndIO.clear();

	// Init blocks2DArray pointers from blocksArray

	blocks2DArray = new LUT_IO_BLOCK**[blocks2DArrayWH];
	for (size_t i = 0; i < blocks2DArrayWH; i++)
	{
		blocks2DArray[i] = new LUT_IO_BLOCK*[blocks2DArrayWH];
	}
	for (size_t i = 0; i < blocks2DArrayWH; i++)
	{
		for (size_t j = 0; j < blocks2DArrayWH; j++)
		{
			blocks2DArray[i][j] = nullptr;
		}
	}
	for (size_t i = 0; i < blocksCount; i++)
	{
		LUT_IO_BLOCK* currentBlock = &blocksArray[i];
		blocks2DArray[currentBlock->coords[1]][currentBlock->coords[0]] = currentBlock;
	}

}

void FPGA::initFPGA(const string& placeFile, const string& netsFile){
	try{
		parsePlaceFile(placeFile);
		parseNetsFile(netsFile);
		init2DArrayBlocks();
	}
	catch (exception& e){
		cerr << "error: " << e.what() << endl;
		exit(1);
	}
	catch (...){
		cerr << "error: initFPGA()" << endl;
		exit(1);
	}
}