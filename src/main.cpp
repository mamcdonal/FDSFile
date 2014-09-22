//
//  main.cpp
//  FDSFile
//
//  Created by Matt McDonald on 2014-08-27.
//  Copyright (c) 2014 Matt McDonald. All rights reserved.
//

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/contrib/contrib.hpp>

#include "FDSFile.h"
//#include "FDSHeader.h"

using namespace cv;
using namespace std;

int main(int argc, const char * argv[])
{
    if(argc > 1){

    	stringstream ss;
    	int startBin,endBin,startShot,endShot,fftSize,overlap;
    	double fLow,fHigh;

    	string fdsFilename = argv[1];

    	for (int i=2; i<argc; ++i)
    		ss << argv[i] << '\n';

    	ss >> startBin >> endBin >> startShot >> endShot >> fftSize >> overlap >> fLow >> fHigh;

        FDSFile fdsfile = FDSFile(fdsFilename);

        cout <<  fdsfile.fdsHeader.name << endl;

        cout << "FDSVersion = " << fdsfile.fdsHeader.FDSVersion << endl;

        cout << "HeaderSectionSize = " << fdsfile.fdsHeader.HeaderSizeBytes << endl;

        for (size_t i=0; i<fdsfile.fdsHeader.keys.size(); ++i){
            cout << fdsfile.fdsHeader.keys[i] + " = " + fdsfile.fdsHeader.values[i] << endl;
        }

//
//        Mat data = fdsfile.getData(startBin,endBin,startShot,endShot);
//
//        fdsfile.debias(data);
//
//        Mat psd = fdsfile.getPSD(data);
//
//        log(1+psd,psd);

//        Mat im = fdsfile.getSoundfield(startBin,endBin,startShot,endShot,fftSize,overlap,fLow,fHigh);

        Mat im = fdsfile.getSpectrogram(startBin,endBin,startShot,endShot,fftSize,overlap);

        log(1+im,im);

        fdsfile.scaleForImage(im);

        resize(im,im, Size(1280,720), 0, 0, INTER_CUBIC);

        applyColorMap(im,im,COLORMAP_JET);

        namedWindow( "Display Data", CV_WINDOW_NORMAL);
        imshow("Display Data", im);

        cout << "Done!" << endl;

        waitKey(0);
    }

    return 0;
}

