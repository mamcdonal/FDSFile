//
//  FDSFile.h
//  FDSFile
//
//  Created by Matt McDonald on 2014-08-27.
//  Copyright (c) 2014 Matt McDonald. All rights reserved.
//

#ifndef __FDSFile__FDSFile__
#define __FDSFile__FDSFile__

//#include <fstream>
#include "FDSHeader.h"
#include <opencv2/core/core.hpp>

class FDSFile
{
public:
    FDSHeader fdsHeader;

    FDSFile();
    FDSFile(std::string fdsFilename);
    ~FDSFile();

    cv::Mat getData(int startBin, int endBin, int startShot, int endShot);

    cv::Mat getPSD(cv::Mat data);

    cv::Mat getSoundfield(int startBin,int endBin, int startShot, int endShot, int fftSize, double fLow, double fHigh);

    void debias(cv::Mat &data);

    cv::Mat scaleForImage(cv::Mat data);

};

#endif /* defined(__FDSFile__FDSFile__) */
