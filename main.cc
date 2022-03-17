#include <iostream>
#include <fstream>
#include <string>
#include <math.h> 
#include <map> 
#include <list>
#include <algorithm>
#include <stdio.h>

#include <unordered_map>



/*
 WB Write back : 
What happens when a line previously written to needs to be
replaced?
1. Need to have a “dirty bit” (D) with each block in the cache –
set it when block is written to
2. When a dirty block is replaced, need to write entire block
back to next level of memory (“writeback”)
//WHEN BLOCK IS WRITTEN TO, SET DIRTY

Write-Allocate (WA) 
Bring the block into the cache if the write misses (handled just
like a read miss)
– Typically, used with write-back policy: WBWA
READ MISS ALSO BRINGS BLOCK INTO CACHE

*/

using namespace std;

string BinToHex(string bin);

int counter = 0;

list<string> traceList;
list<string> optList;

struct TreeNode {
	bool bit;
    string tag;
	TreeNode* parent;
    TreeNode* left;
    TreeNode* right;

	TreeNode() {
        this->bit = false;
 
		//if both null, leaf node!!
        left = nullptr;
        right = nullptr;
    }
	TreeNode(string tag) {
        this->tag = tag;
 
		//if both null, leaf node!!
        left = nullptr;
        right = nullptr;
    }
};

struct PLRU {

	TreeNode* root;
	//1, 2, 4
	void add() {

		TreeNode* temp = root;
		while (true) {

			if (temp->bit == false) {
				temp = temp->left;
			}
			if (temp->bit == true) {
				temp = temp->right;
			}

		}


	}

	void replace() {


	}

	PLRU() {

	}

};

struct Block {
	string tag;
	int lrucount;
	int optcount;
	bool dirty;
	TreeNode* parent;
	Block (string tag, bool dirty = false) {
		this->tag = tag;
		this->lrucount = counter;
		counter++;
		this->dirty = dirty;
	}
};

struct Set {
	list<Block> blocks; //list of blocks in order
	PLRU tree; //plru tree
	Set(){}
};

struct Cache {
	int num;
	int assoc;
	int size = 0;

	map<string, Set> table; //index and tag list in order of print
	Cache () {
	}
	Cache (int num) {
		this->num = num;
	}
	void setVals(int size, int assoc) {
		this->size = size;
		this->assoc = assoc;
	} 
};

void findTag (Cache &L, string index, string tag, string line);
void findIndex (Cache &L, string index, string tag, string line);
void replace(Cache &L, string tag, list<Block> *tagsList, string line);

enum command {
	R, W
};

//Read IN ----------------------
int blockSize = 0;
int L1Size = 0;
int L1Assoc = 0;
int L2Size = 0;
int L2Assoc = 0;
int replacementPolicy = 0;
int inclusionPolicy = 0;
char* traceFileName = "";

int L1NumSets = 0;
int L2NumSets = 0;

//Write OUT ----------------------
int L1Reads = 0;
int L1ReadMisses = 0;
int L1Writes = 0;
int L1WriteMisses = 0;
float L1MissRate = 0;
int L1Writebacks = 0;

int L2Reads = 0;
int L2ReadMisses = 0;
int L2Writes = 0;
int L2WriteMisses = 0;
float L2MissRate = (L2Reads == 0) ? 0 : (float)(L2ReadMisses + L2WriteMisses) / (float)(L2Reads + L2Writes);
int L2Writebacks = 0;

int totalMemTraffic = 0; //number of blocks transferred to/from memory


Cache L1 = Cache(1);
Cache L2 = Cache(2);

command state;


//L1
int L1indexBits = log2(L1NumSets); //
int L1blockOffsetBits = log2(blockSize); //
int L1tagBits = 32 - L1indexBits - L1blockOffsetBits; //

//L2
int L2indexBits = log2(L2NumSets); //
int L2blockOffsetBits = log2(blockSize); //
int L2tagBits = 32 - L2indexBits - L2blockOffsetBits; //

// --------------------------------

void testInputs() {
	blockSize = 16;
	L1Size = 1024;
	L1Assoc = 2;
	L1.setVals(L1Size, L1Assoc);
	L2Size = 0;
	L2Assoc = 0;
	if (L2Size != 0) {
		L2.setVals(L2Size, L2Assoc);
	}

	replacementPolicy = 0;
	inclusionPolicy = 0;
	traceFileName = "gcc_trace.txt"; //"test.txt";//"perl_trace.txt";//

	L1NumSets = L1Size / (L1Assoc * blockSize);
	L2NumSets = L2Size / (L2Assoc * blockSize);

	//L1
	L1indexBits = log2(L1NumSets); //
	L1blockOffsetBits = log2(blockSize); //
	L1tagBits = 32 - L1indexBits - L1blockOffsetBits; //

	//L2
	if (L2Size != 0) {
		L2indexBits = log2(L2NumSets); //
		L2blockOffsetBits = log2(blockSize); //
		L2tagBits = 32 - L2indexBits - L2blockOffsetBits; //
	}
}



string HexToBinary(string hex) {
    
	long int i = 0;
	string binary = "";

    while (hex[i]) {
 
        switch (hex[i]) {
        case '0':
            binary += "0000";
            break;
        case '1':
            binary += "0001";
            break;
        case '2':
            binary += "0010";
            break;
        case '3':
            binary += "0011";
            break;
        case '4':
            binary += "0100";
            break;
        case '5':
            binary += "0101";
            break;
        case '6':
            binary += "0110";
            break;
        case '7':
            binary += "0111";
            break;
        case '8':
            binary += "1000";
            break;
        case '9':
            binary += "1001";
            break;
        case 'A':
        case 'a':
            binary += "1010";
            break;
        case 'B':
        case 'b':
            binary += "1011";
            break;
        case 'C':
        case 'c':
            binary += "1100";
            break;
        case 'D':
        case 'd':
            binary += "1101";
            break;
        case 'E':
        case 'e':
            binary += "1110";
            break;
        case 'F':
        case 'f':
            binary += "1111";
            break;
        default:
            cout << "\nInvalid hexadecimal digit " << hex[i];
        }
        i++;
    }

	return binary;
}

bool validInputs(int argc, char *argv[]) {

    //if (argc < 9) {
       // fprintf(stderr, "Not enough arguments \n");
        //return false; //exit(EXIT_FAILURE);
    //}

	blockSize = atoi(argv[1]);
	L1Size = atoi(argv[2]);
	L1Assoc = atoi(argv[3]);
	L2Size = atoi(argv[4]);
	L2Assoc = atoi(argv[5]);
	replacementPolicy = atoi(argv[6]);
	inclusionPolicy = atoi(argv[7]);
	traceFileName = argv[8];

	L1NumSets = L1Size / L1Assoc * blockSize;
	L2NumSets = L2Size / L2Assoc * blockSize;


	return true;
}

string expand(string hex) {

	if (hex.size() < 8) {
		hex = "0" + hex;
		return expand(hex);
	}
	return hex;
}

string condense(string hex) {

	if (hex[0] == '0') {
		hex = hex.substr(1);
		return condense(hex);
	}
	return hex;
}

void Print(Cache L1, Cache L2) {

	cout << "===== Simulator configuration =====" << endl;
	cout << "BLOCKSIZE:             " << blockSize << endl;
	cout << "L1_SIZE:               " << L1Size << endl;
	cout << "L1_ASSOC:              " << L1Assoc << endl;
	cout << "L2_SIZE:               " << L2Size << endl;
	cout << "L2_ASSOC:              " << L2Assoc << endl;

	cout << "REPLACEMENT POLICY:    ";
	
		switch (replacementPolicy) {
			case 0:
				cout << "LRU" << endl;
				break;
			case 1:
				cout << "Pseudo-LRU" << endl;
				break;
			case 2:
				cout << "OPT" << endl;
				break;
		}

	
	cout << "INCLUSION PROPERTY:    ";
	
		switch (inclusionPolicy) {
			case 0:
				cout << "non-inclusive" << endl;
				break;
			case 1:
				cout << "inclusive" << endl;
				break;
		}
	
	cout << "trace_file:            " << traceFileName << endl;
	
	if (L1Size != 0) {
		cout << "===== L1 contents =====" << endl;

		map<string, Set>::iterator it;
		int count = 0;
		for (it = L1.table.begin(); it != L1.table.end(); it++)
		{
			cout << "Set     " << count << ":";
			count++;

			list<Block> temp = it->second.blocks;
			list<Block>::iterator it2;
			for (it2 = temp.begin(); it2 != temp.end(); it2++) {
				
				cout << "\t" << condense(BinToHex(it2->tag)) << " ";
				
				if (it2->dirty) {
					cout << "D";
				}
				else {
					cout << " ";
				}
				if (condense(BinToHex(it2->tag)).size() < 6) {
					cout << "\t";
				} 
			}
			
			cout << " \t";
			cout << endl;
		}

		L1MissRate = (float)(L1ReadMisses + L1WriteMisses) / (float)(L1Reads + L1Writes);
	}

	if (L2Size != 0) {
		cout << "===== L2 contents =====" << endl;

		map<string, Set>::iterator it;
		int count = 0;
		for (it = L2.table.begin(); it != L2.table.end(); it++)
		{
			cout << "Set     " << count << ":";
			count++;

			list<Block> temp = it->second.blocks;
			list<Block>::iterator it2;
			for (it2 = temp.begin(); it2 != temp.end(); it2++) {
				
				cout << "\t" << condense(BinToHex(it2->tag)) << " ";
				
				if (it2->dirty) {
					cout << "D";
				}
				else {
					cout << " ";
				}
				if (condense(BinToHex(it2->tag)).size() < 6) {
					cout << "\t";
				} 
			}
			
			cout << " \t";
			cout << endl;
		}
		L2MissRate = (float)(L2ReadMisses + L2WriteMisses) / (float)(L2Reads + L2Writes);
	}

	if (L2Size != 0) {
		totalMemTraffic = L2ReadMisses + L2WriteMisses + L2Writebacks;
	}
	else {
		totalMemTraffic = L1ReadMisses + L1WriteMisses + L1Writebacks;
	}

	cout << "===== Simulation results (raw) =====" << endl;
	cout << "a. number of L1 reads:        " << L1Reads << endl;
	cout << "b. number of L1 read misses:  " << L1ReadMisses << endl;
	cout << "c. number of L1 writes:       " << L1Writes << endl;
	cout << "d. number of L1 write misses: " << L1WriteMisses << endl;
	cout << "e. L1 miss rate:              ";
	printf("%0.6lf", L1MissRate);
	cout << endl;
	cout << "f. number of L1 writebacks:   " << L1Writebacks << endl;
	cout << "g. number of L2 reads:        " << L2Reads << endl;
	cout << "h. number of L2 read misses:  " << L2ReadMisses << endl;
	cout << "i. number of L2 writes:       " << L2Writes << endl;
	cout << "j. number of L2 write misses: " << L2WriteMisses << endl;
	cout << "k. L2 miss rate:              " << L2MissRate << endl;
	cout << "l. number of L2 writebacks:   " << L2Writebacks << endl;
	cout << "m. total memory traffic:      " << totalMemTraffic << endl;

}

string tag1_prev = "";
string tag2_prev = "";


/////////////////////////////////////////////////////////////////////////////////

void createMap(unordered_map<string, char> *um)
{
    (*um)["0000"] = '0';
    (*um)["0001"] = '1';
    (*um)["0010"] = '2';
    (*um)["0011"] = '3';
    (*um)["0100"] = '4';
    (*um)["0101"] = '5';
    (*um)["0110"] = '6';
    (*um)["0111"] = '7';
    (*um)["1000"] = '8';
    (*um)["1001"] = '9';
    (*um)["1010"] = 'a';
    (*um)["1011"] = 'b';
    (*um)["1100"] = 'c';
    (*um)["1101"] = 'd';
    (*um)["1110"] = 'e';
    (*um)["1111"] = 'f';
}

string BinToHex(string bin) {
    int l = bin.size();
    int t = bin.find_first_of('.');
     
    int len_left = t != -1 ? t : l;
     
    for (int i = 1; i <= (4 - len_left % 4) % 4; i++) {
        bin = '0' + bin;
    }
       
    if (t != -1) {
        int len_right = l - len_left - 1;
         
        for (int i = 1; i <= (4 - len_right % 4) % 4; i++) {
            bin = bin + '0';
        }
    }
     
    unordered_map<string, char> bin_hex_map;
    createMap(&bin_hex_map);
     
    int i = 0;
    string hex = "";
     
    while (1) {
        hex += bin_hex_map[bin.substr(i, 4)];
        i += 4;
        if (i == bin.size()) {
            break;
        }
             
        if (bin.at(i) == '.') {
            hex += '.';
            i++;
        }
    }
     
    // required hexadecimal number
    return hex;   
}

/////////////////////////////////////////////////////////////////////////////////


void replace(Cache &L, string index, string tag, list<Block> *tagsList, string line) {

	list<Block>::iterator toReplace = tagsList->begin();

	if (replacementPolicy == 0) {
		list<Block>::iterator itFind;

		for (itFind = tagsList->begin(); itFind != tagsList->end(); itFind++) { //find LRU and replace
			if (itFind->lrucount < toReplace->lrucount) {
				toReplace = itFind;
			}
		}

	}
	else if (replacementPolicy == 1  && L.assoc != 1) { //PLRU
		//TODO










	}
	else if (replacementPolicy == 2) { //OPT
		
		list<string>::iterator tagFind;
		list<list<Block>::iterator> opt;

		bool breakout = false;
		for (tagFind = optList.begin(); tagFind != optList.end() && !breakout; tagFind++) { //find OPT 

			string indexComp;
			string tagComp;
			string addressComp = HexToBinary(expand(tagFind->substr(tagFind->find(" ") + 1)));;

			if (L.num == 1) {
				tagComp = addressComp.substr(0, L1tagBits);
				indexComp = addressComp.substr(L1tagBits, L1indexBits);
			}
			else if (L.num == 2) {
				tagComp = addressComp.substr(0, L2tagBits);
				indexComp = addressComp.substr(L2tagBits, L2indexBits);
			}

			if (indexComp == index) {
				list<Block>::iterator itList;
				for (itList = tagsList->begin(); itList != tagsList->end() && !breakout; itList++) {
					if (tagComp == itList->tag && itList->optcount != 1) {
						opt.push_back(itList);
						itList->optcount = 1;
						if (opt.size() == tagsList->size()) {
							toReplace = itList;
							breakout = true;
						}
					}
				}		
				if (opt.size() < tagsList->size() && opt.size() != 0) {
					toReplace = opt.front();
				}
			}
		}

	}

	if (toReplace->dirty == true) {
		if (L.num == 1) {
			if (L2.size != 0) {
				//WRITE TO L2!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				string address = HexToBinary(expand(line.substr(line.find(" ") + 1)));
				
				//L2
				string L2tag = address.substr(0, L2tagBits);
				string L2index = address.substr(L2tagBits, L2indexBits);
				string L2blockOffset = address.substr(L2tagBits + L2indexBits);
	
				if ((line.find("w") != std::string::npos) || (line.find("W") != std::string::npos)) {
				//	state = W;
					L2Writes++;
				}
				if ((line.find("r") != std::string::npos) || (line.find("R") != std::string::npos)) {
				//	state = R;
					L2Reads++;
				}

				findIndex(L2, L2index, L2tag, line);
			}
			L1Writebacks++;
		}
		else if (L.num == 2){
			//WRITE TO MAIN MEMORY
			L2Writebacks++;
		}
	}

	Block &blk(*toReplace);

	if ((line.find("r") != std::string::npos) || (line.find("R") != std::string::npos))  {
		blk = Block(tag, false); //CORRECT
	}
	else if ((line.find("w") != std::string::npos) || (line.find("W") != std::string::npos)) {
		blk = Block(tag, true); //CORRECT
	}

}
	
void replaceSame(Cache &L, string index, string tag, list<Block> *tagsList, string line) {

	list<Block>::iterator toReplace = tagsList->begin();
	
	if (replacementPolicy == 0) {

		list<Block>::iterator itFind;

		for (itFind = tagsList->begin(); itFind != tagsList->end(); itFind++) { //find LRU and replace
			if (itFind->tag == tag) {
				toReplace = itFind;
			}
		}

		if (toReplace->dirty == true) {
			if (L.num == 1) {
				/*
				if (L2.size != 0) {
					//WRITE TO L2!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					string address = HexToBinary(expand(line.substr(line.find(" ") + 1)));
					
					//L2
					string L2tag = address.substr(0, L2tagBits);
					string L2index = address.substr(L2tagBits, L2indexBits);
					string L2blockOffset = address.substr(L2tagBits + L2indexBits);

					if ((line.find("w") != std::string::npos) || (line.find("W") != std::string::npos)) {
					//	state = W;
						L2Writes++;
					}
					if ((line.find("r") != std::string::npos) || (line.find("R") != std::string::npos)) {
					//	state = R;
						L2Reads++;
					}

					findIndex(L2, L2index, L2tag, line);
				}
				*/
				//L1Writebacks++; //IDFK NO WRITRBACK BECAUSE NOT ACTUALLY REPLACED??? up by 3k
			}
			else if (L.num == 2){
				//WRITE TO MAIN MEMORY
				// L2Writebacks++; IDFK 
			}
		}
	}
	else if (replacementPolicy == 1  && L.assoc != 1) {
		//TODO
	}
	else if (replacementPolicy == 2) {
		//TODO
	}

	Block &blk(*toReplace);
	blk = Block(tag, true);	//CORRECT counter updated
}

string t1 = "";
string t2 = "";
string t1_bin = "";
string t2_bin = "";
string t1_count = "";
string t2_count = "";
string t1_prev = " ";
string t2_prev = " ";


void findTag (Cache &L, string index, string tag, string line) {

	list<Block>* tagsList = &L.table[index].blocks; //grab already existing tag list

	bool tagHit = false;
	list<Block>::iterator it;

	for (it = tagsList->begin(); it != tagsList->end(); ++it) { //check if tag exists 
   		if (it->tag == tag) {
			tagHit = true;
			break;
		}					
	}
			
	if(tagHit) { //if tag was found
		if (state == R) {
			it->lrucount = counter; //refresh LRU //CORRECTT
			counter++;
		}
		else if (state == W) {
			replaceSame(L, index, tag, tagsList, line);	
				
		}
	}
	else { //if tag was not found

		if ((line.find("r") != std::string::npos) || (line.find("R") != std::string::npos)) {
			if (L.num == 1) {
				L1ReadMisses++;
			}
			else if (L.num == 2) {
				L2ReadMisses++;
			}
		}
		else if ((line.find("w") != std::string::npos) || (line.find("W") != std::string::npos)) { 
			if (L.num == 1) {
				L1WriteMisses++;
			}
			else if (L.num == 2) {
				L2WriteMisses++;
			}
		}

		if (tagsList->size() < L.assoc) {
			if ((line.find("r") != std::string::npos) || (line.find("R") != std::string::npos)) {
				tagsList->push_back(Block(tag, false));
			}
			else if ((line.find("w") != std::string::npos) || (line.find("W") != std::string::npos)) { 
				tagsList->push_back(Block(tag, true));
			}
		}
		else { //REPLACE
			replace(L, index, tag, tagsList, line);
		}			
	}			
}

void findIndex (Cache &L, string index, string tag, string line) {

	if (L.table.find(index) == L.table.end()) { //if index doesnt exist //TODO: THERE NEEDS TO BE A CAP I THINK

		if (state == R) {
			if (L.num == 1) {
				L1ReadMisses++;
			}
			else if (L.num == 2) {
				L2ReadMisses++;
			}
		}
		else if (state == W) {			
			if (L.num == 1) {
				L1WriteMisses++;
			}
			else if (L.num == 2) {
				L2WriteMisses++;
			}
		}	

		list<Block> blocks;
		blocks.push_back(Block(tag)); //create list for blocks and add current tag
				
		Set set = Set();
		L.table[index] = set;
		L.table[index].blocks = blocks;
		
		if (replacementPolicy == 1 && L.assoc != 1) {

			PLRU plru = PLRU();
			L.table[index].tree = plru;
			TreeNode tree = TreeNode();
			TreeNode tagNode = TreeNode(tag);
			
			if (L.assoc == 2) {
				L.table[index].tree.root = &tree;
				L.table[index].tree.root->left = &tagNode;
			}
			if (L.assoc == 4) {

				L.table[index].tree.root = &tree;

				TreeNode left = TreeNode();
				TreeNode right = TreeNode();

				left.left = &tagNode;

				L.table[index].tree.root->left = &left;
				L.table[index].tree.root->right = &right;
			}
		}
	}
			
	else { //if index found
		findTag(L, index, tag, line);
	}

}


int main(int argc, char *argv[]) {

	testInputs(); //REMOVE later

	//if (!validInputs(argc, argv)) {
	//	return 1;
	//}
	
	//READ FOR OPT
	string line;
	ifstream trace_file(traceFileName);

	if (trace_file.is_open()) {
		while (getline(trace_file, line)) {
			traceList.push_back(line);
		}
		trace_file.close();
	}
	else {
		cout << "Cant find file!!" << endl;
		return 1;
	}

	optList = traceList;

	list<string>::iterator it;
	for (it = traceList.begin(); it != traceList.end(); it++){

		string command = it->substr(0, 1);
		string address = HexToBinary(expand(it->substr(it->find(" ") + 1)));
		
		//L1
		string L1tag = address.substr(0, L1tagBits);
		string L1index = address.substr(L1tagBits, L1indexBits);
		string L1blockOffset = address.substr(L1tagBits + L1indexBits);

		if ((it->find("w") != std::string::npos) || (it->find("W") != std::string::npos)) {
			state = W;
			L1Writes++;
		}
		if ((it->find("r") != std::string::npos) || (it->find("R") != std::string::npos)) {
			state = R;
			L1Reads++;
		}
		
		findIndex(L1, L1index, L1tag, it->c_str());
		
		//cout << optList.size() << endl;
		optList.pop_front();


		/////////////////////////////////////////////////////// DELETE LATER
		/*
		map<string, Set>::iterator it;
		int count = 0;
		for (it = L1.table.begin(); it != L1.table.end(); it++)
		{

			if (it->first == "00000") {
				//cout << "Set     " << count << ":" << " " << it->first;
				count++;
				list<Block> temp = it->second;
				list<Block>::iterator it2;
				int tagcount = 0;
				for (it2 = temp.begin(); it2 != temp.end(); it2++) {
					
					//cout << "\t" << condense(BinToHex(it2->tag)) << " (" << it2->count << ")" ;
					if (tagcount == 0) {
						t1 = condense(BinToHex(it2->tag)) + " ";
						t1_count = to_string(it2->count) + "";
						t1_bin = it2->tag + " ";
					}
					else if (tagcount == 1) {
						t2 = condense(BinToHex(it2->tag)) + " ";//("; + to_string(it2->count) + ")  ";
						t2_count = to_string(it2->count) + "";
						t2_bin = it2->tag + " ";
					}
					tagcount++;
					
					if (it2->dirty) {
						//cout << "D";
					} 
					//cout << "(" << it2->count << ")";
				}
				
				if (t1 != t1_prev || t2 != t2_prev) {
					//cout << "\t" << t1 << "(" << t1_count << ")     " << t2 << "(" << t2_count << ")";
					if (t1 == t2) {
						cout << " UH OH!! " << t1_bin << " " << t2_bin;
					}
					//cout << endl;
					t1_prev = t1;
					t2_prev = t2;
				}

				//cout << " \t";
				//cout << it->second.size();
				//cout << endl;
			}
		}
		*/
		//////////////////////////////////////////////////////////////////////
	}

	Print(L1, L2);


	return 0;
}
