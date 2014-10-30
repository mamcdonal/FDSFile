//
//  FDSHeader.h
//  FDSFile
//
//  Created by Matt McDonald on 2014-08-27.
//  Copyright (c) 2014 Matt McDonald. All rights reserved.
//

#ifndef __FDSFile__FDSHeader__
#define __FDSFile__FDSHeader__

#include <string>
#include <vector>

class FDSHeader{

public:
    std::string name;
    int FDSVersion;
    int HeaderSizeBytes;
    std::vector<int> HeaderSectionSizesBytes;
    std::vector<std::string> keys;
    std::vector<std::string> values;

    FDSHeader();

    FDSHeader(std::string fdsFilename);

    ~FDSHeader();

    void readHeader();

    std::string getValue(std::string key);

    void printHeader();

private:

    std::vector<std::string> splitLineAtEquals(std::string line);

    std::vector<int> parseVectorOfInts(std::string line);

    std::string deblank(std::string str);
};

#endif /* defined(__FDSFile__FDSHeader__) */
