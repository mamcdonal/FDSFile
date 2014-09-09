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
#include "FDSHeader.h"

using namespace cv;
using namespace std;

int main(int argc, const char * argv[])
{
    if(argc > 1){

        FDSFile fdsfile = FDSFile(argv[1]);

        cout <<  fdsfile.fdsHeader.name << endl;

        cout << endl;

        cout << "FDSVersion = " << fdsfile.fdsHeader.FDSVersion << endl;

        cout << "HeaderSectionSize = " << fdsfile.fdsHeader.HeaderSizeBytes << endl;

        for (int i=0; i<fdsfile.fdsHeader.keys.size(); ++i){
            cout << fdsfile.fdsHeader.keys[i] + " = " + fdsfile.fdsHeader.values[i] << endl;
        }

        Mat psd = fdsfile.getPSD(0,1000,0,1023);

        Mat data = fdsfile.getData(0,1000,0,1023);

        log(1+psd,psd);

        data = fdsfile.debias(data);

        data = abs(data);

        data = fdsfile.scaleForImage(data);

        psd = fdsfile.scaleForImage(psd);

        applyColorMap(psd,psd,COLORMAP_JET);

        applyColorMap(data,data,COLORMAP_JET);

        resize(data, data, Size(720,1280), 0, 0, INTER_CUBIC);

        resize(psd, psd, Size(1280,720), 0, 0, INTER_CUBIC);

        namedWindow( "Display Data", CV_WINDOW_NORMAL);
        imshow("Display Data", data.t());

        namedWindow( "Display PSD", CV_WINDOW_NORMAL);
        imshow("Display PSD", psd);

        waitKey(0);
    }

    return 0;
}

