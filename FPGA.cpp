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
}

LUT_IO_BLOCK::LUT_IO_BLOCK(){
	name = "none";
	ID = 0xFFFFFFFF;
	coords[0] = 0;
	coords[1] = 0;
}

void FPGA::parsePlaceFile(const string& filename){
	blockArrayWH = 0;
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
			unsigned int X = stoi(matches[2]);
			unsigned int Y = stoi(matches[3]);
			unsigned int ID = stoi(matches[5]);
			LUTsAndIO.emplace(pair<string, LUT_IO_BLOCK>(name, LUT_IO_BLOCK(name, X, Y, ID)));
			blocksCount = max(blocksCount, ID); // find biggest ID
		}
		else if (regex_match(lineString, matches, arraySizeRegEx)){
			blockArrayWH = stoi(matches[1]);
		}
	}
	file.close();
	blocksCount++;
	if (blockArrayWH == 0){
		throw logic_error("Can't define logic blocks array size");
	}
}

void FPGA::parseNetsFile(const string& filename){
	ifstream file(filename, ifstream::in);
	checkFileOpening(file, filename);
	//regex globalRegEx("\.global\\s+(\\S+)\\s*$"); // .global clk_name # Don't route clk net.
	//regex inputRegEx("\.input\\s+(\\S+)\\s*$"); // .input my_input_name
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
			auto currentOutBlockID = LUTsAndIO[currentClbBlockName].ID;

			getline(file, lineString);
			if (regex_match(lineString, matches, pinlistRegEx))
			{
				// for all pins: find in map LUT by its name and add to this LUT ID of clb block
				for (size_t i = 1; i < 5; i++)
				{
					if (matches[i] != "open"){
						LUTsAndIO[matches[i]].destBlocks.push_back(currentOutBlockID);
					}
				}
			}
			else{
				throw logic_error(string("Can't find pinlist for ") + currentClbBlockName);
			}
		}else if (regex_match(lineString, matches, outputRegEx)){
			string currentOutBlockName = matches[1];
			auto currentOutBlockID = LUTsAndIO[currentOutBlockName].ID;

			getline(file, lineString);
			if (regex_match(lineString, matches, pinlistIORegEx))
			{
				// find in map LUT by its name, which bound with output block, and add to this LUT ID of output block
				LUTsAndIO[matches[1]].destBlocks.push_back(currentOutBlockID);
			}
			else{
				throw logic_error(string("Can't find pinlist for ") + currentOutBlockName);
			}
		}
	}
	file.close();

	// Init blocksArray from LUTsAndIO unordered_map
	blocksArray = new LUT_IO_BLOCK[blocksCount];
	for (auto blockIt = LUTsAndIO.begin(); blockIt != LUTsAndIO.end(); blockIt++)
	{
		//cout << blockIt->first << endl;
		blocksArray[blockIt->second.ID] = blockIt->second;
	}
	for (size_t blockIt = 0; blockIt < blocksCount; blockIt++)
	{
		for (size_t destIt = 0; destIt < blocksArray[blockIt].destBlocks.size(); destIt++)
		{
			cout << blocksArray[blockIt].destBlocks[destIt] << " ";
		}
		cout << endl;
	}
}

void FPGA::initFPGA(const string& placeFile, const string& netsFile){
	try{
		parsePlaceFile(placeFile);
		parseNetsFile(netsFile);
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