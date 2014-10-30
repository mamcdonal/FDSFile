//
//  FDSHeader.cpp
//  FDSFile
//
//  Created by Matt McDonald on 2014-08-27.
//  Copyright (c) 2014 Matt McDonald. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <vector>
#include "FDSHeader.h"

using namespace std;

FDSHeader::FDSHeader(){
};

FDSHeader::FDSHeader(string fdsFilename){
    name = fdsFilename;
    readHeader();
};

FDSHeader::~FDSHeader(){
};

void FDSHeader::readHeader(){
    ifstream fdsStream(name);

    if (fdsStream.is_open()){

        string line;
        vector<string> splitLine;
        int bytesRead=0;
        int lineNum = 0;

        getline(fdsStream,line);
        splitLine = splitLineAtEquals(line);
        FDSVersion = stoi(splitLine[1]);

        bytesRead += line.length()+1;
        lineNum++;

        getline(fdsStream,line);
        splitLine = splitLineAtEquals(line);
        HeaderSizeBytes = stoi(splitLine[1]);

        bytesRead += line.length()+1;
        lineNum++;

        getline(fdsStream,line);
        splitLine = splitLineAtEquals(line);
        HeaderSectionSizesBytes = parseVectorOfInts(splitLine[1]);

        bytesRead += line.length()+1;
        lineNum++;

        int bytesToRead = HeaderSectionSizesBytes[0]+HeaderSectionSizesBytes[1];

        while ( getline(fdsStream,line) && bytesRead < bytesToRead)
        {
            splitLine = splitLineAtEquals(line);

            keys.push_back(deblank(splitLine[0]));
            values.push_back(deblank(splitLine[1]));

            bytesRead += line.length()+1;
            lineNum++;
        }

        fdsStream.close();
    }
};

string FDSHeader::getValue(string key){
    string value;

    for (size_t i=0; i<keys.size(); ++i) {
        if (key.compare(keys[i]) == 0){
            value = values[i];
            break;
        }
    }

    return value;
};

void FDSHeader::printHeader(){
    for (size_t i=0; i<keys.size(); ++i){
        cout << keys[i] + " = " + values[i] << endl;
    }
}

vector<string> FDSHeader::splitLineAtEquals(string line){
    vector<string> splitLine(2);

    size_t i = 0;
    while (i<line.length() && line[i]!='=') {
        splitLine[0] += line[i];
        ++i;
    }

    for (size_t j=i+1; j<line.length(); j++){
        splitLine[1] += line[j];
    }

    return splitLine;
};

vector<int> FDSHeader::parseVectorOfInts(string line){
    vector<int> parsedLine;

    for (size_t i=0; i<line.length();){
        if (isdigit(line[i])){
            int j=i;
            string num;
            while (isdigit(line[j])) {
                num += line[j];
                j++;
            }
            parsedLine.push_back(stoi(num));
            i=j+1;
        }else{
            i++;
        }
    }

    return parsedLine;
};

string FDSHeader::deblank(string str){
    string retStr;
    for (size_t i=0; i<str.length(); ++i){
    	if (str[i] != ' '){
            retStr += str[i];
        }
    }
    return retStr;
};


