//
//  main.cpp
//  FDSFile
//
//  Created by Matt McDonald on 2014-08-27.
//  Copyright (c) 2014 Matt McDonald. All rights reserved.
//

#include <iostream>
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

        FDSFile fdsfile = FDSFile(argv[1]);

        cout <<  fdsfile.fdsHeader.name << endl;

        cout << "FDSVersion = " << fdsfile.fdsHeader.FDSVersion << endl;

        cout << "HeaderSectionSize = " << fdsfile.fdsHeader.HeaderSizeBytes << endl;

        for (size_t i=0; i<fdsfile.fdsHeader.keys.size(); ++i){
            cout << fdsfile.fdsHeader.keys[i] + " = " + fdsfile.fdsHeader.values[i] << endl;
        }
//
//        Mat data = fdsfile.getData(0,1000,0,1023);
//
//        fdsfile.debias(data);
//
//        Mat psd = fdsfile.getPSD(data);
//
//        log(1+psd,psd);

//        cout << "Processing..." << endl;
//
        Mat soundfield = fdsfile.getSoundfield(0,10,0,1e99,1024,10,500);

        Mat dataToShow = fdsfile.scaleForImage(soundfield);

        //resize(dataToShow, dataToShow, Size(1280,720), 0, 0, INTER_LINEAR);

        applyColorMap(dataToShow,dataToShow,COLORMAP_JET);

        namedWindow( "Display Data", CV_WINDOW_NORMAL);
        imshow("Display Data", dataToShow);

        cout << "Done!" << endl;

        waitKey(0);
    }

    return 0;
}

