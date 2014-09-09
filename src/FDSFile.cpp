//
//  FDSFile.cpp
//  FDSFile
//
//  Created by Matt McDonald on 2014-08-27.
//  Copyright (c) 2014 Matt McDonald. All rights reserved.
//

#include "FDSFile.h"
#include <string>
#include <opencv2/core/core.hpp>
#include <stdio.h>
#include <iostream>
#include "math.h"

using namespace std;
using namespace cv;

FDSFile::FDSFile(){
};

FDSFile::FDSFile(string fdsFilename){
    fdsHeader = FDSHeader(fdsFilename);
};

FDSFile::~FDSFile(){
};

Mat FDSFile::getData(int startBin, int endBin, int startShot, int endShot){

	int numBins = stoi(fdsHeader.getValue("DataLocusCount"));
	int numShots = stoi(fdsHeader.getValue("TimeStepCount"));

	startBin = fmin(fmax(0,startBin),numBins);
	endBin = fmin(fmax(0,endBin),numBins);

	startShot = fmin(fmax(0,startShot),numShots);
	endShot = fmin(fmax(0,endShot),numShots);

	int numBinsToRead = fmin(numBins,endBin - startBin + 1);
	int numShotsToRead = fmin(numShots,endShot - startShot + 1);

	int numBinsToSkip = numBins - numBinsToRead;

	uint16_t *buffer = new uint16_t[numBinsToRead];

	int startOfData = fdsHeader.HeaderSizeBytes + (startBin + numBins*startShot)*sizeof(uint16_t);

    FILE *fdsStream = fopen (fdsHeader.name.c_str(), "rb" );

    if (fdsStream==NULL) {
    	fputs ("File error",stderr);
    	exit (1);
    }

    Mat data(numShotsToRead,numBinsToRead,DataType<uint16_t>::type);

    fseek (fdsStream, startOfData, SEEK_SET);

    for (int i=0; i<numShotsToRead; ++i){
    	fread (buffer,sizeof(uint16_t),numBinsToRead,fdsStream);
    	for (int j=0; j<numBinsToRead; ++j)
    		data.at<uint16_t>(i,j) = buffer[j];
    	fseek(fdsStream,numBinsToSkip*sizeof(uint16_t),SEEK_CUR);
    }

    fclose(fdsStream);

    delete buffer;

	return data;
};

Mat FDSFile::debias(Mat data){
//	data.convertTo(data,DataType<float>::type,1,-pow(2,15));

	data.convertTo(data,CV_32F,1,-pow(2,15));

	for (int i=0; i<data.cols; ++i)
		data.col(i) -= mean(data.col(i));

	return data;
}

Mat FDSFile::getPSD(int startBin,int endBin, int startShot, int endShot){
	Mat psdData;

	Mat data = getData(startBin,endBin,startShot,endShot);

	data = debias(data);

	Mat dftData;

	dft(data.t(),dftData,DFT_ROWS | DFT_SCALE | DFT_COMPLEX_OUTPUT);

	data.release();

	vector<Mat> ReImDFT(2);

	split(dftData,ReImDFT);

	magnitude(ReImDFT[0],ReImDFT[1],psdData);

	return psdData;
};

Mat FDSFile::getSoundfield(int startBin,int endBin, int startShot, int endShot, int fftSize, double fLow, double fHigh){
	Mat soundfield;

	Mat data = getData(startBin,endBin,startShot,endShot);

	data = debias(data);

	return soundfield;
};

Mat FDSFile::scaleForImage(Mat data){
    double min, max;
    minMaxLoc(data, &min, &max);

    Mat adjData;

    convertScaleAbs(data,data,255/(max-min),-255*min/(max-min));

    return data;
};



