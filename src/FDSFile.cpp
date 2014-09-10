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

	int startOfData = fdsHeader.HeaderSizeBytes + (startBin + numBins*startShot)*sizeof(uint16_t);

    FILE *fdsStream = fopen (fdsHeader.name.c_str(), "rb" );

    if (fdsStream==NULL) {
    	fputs ("File error",stderr);
    	exit (1);
    }

    Mat buffer(1,numBinsToRead,DataType<uint16_t>::type);

    Mat data(numShotsToRead,numBinsToRead,DataType<uint16_t>::type);

    fseek (fdsStream, startOfData, SEEK_SET);

    for (int i=0; i<numShotsToRead; ++i){
    	fread(buffer.ptr(),sizeof(uint16_t),numBinsToRead,fdsStream);
    	buffer.copyTo(data.row(i));
    	fseek(fdsStream,numBinsToSkip*sizeof(uint16_t),SEEK_CUR);
    }

    fclose(fdsStream);

	return data;
};

void FDSFile::debias(Mat &data){

	data.convertTo(data,CV_32F,1,-pow(2,15));

	for (int i=0; i<data.cols; ++i)
		data.col(i) -= mean(data.col(i));
}

Mat FDSFile::getPSD(Mat data){
	Mat psdData;

	Mat dftData;

	dft(data.t(),dftData,DFT_ROWS | DFT_SCALE | DFT_COMPLEX_OUTPUT);

	vector<Mat> ReImDFT(2);

	split(dftData,ReImDFT);

	magnitude(ReImDFT[0],ReImDFT[1],psdData);

	return psdData.colRange(0,(int)ceil(psdData.cols/2.0)+1);
};

Mat FDSFile::getSoundfield(int startBin,int endBin, int startShot, int endShot, int fftSize, double fLow, double fHigh){

	int numBins = stoi(fdsHeader.getValue("DataLocusCount"));
	int numShots = stoi(fdsHeader.getValue("TimeStepCount"));
	int Fs = stoi(fdsHeader.getValue("TimeStepFrequency_Hz"));

	startBin = fmin(fmax(0,startBin),numBins);
	endBin = fmin(fmax(0,endBin),numBins);

	startShot = fmin(fmax(0,startShot),numShots);
	endShot = fmin(fmax(0,endShot),numShots);

	fLow = fmin(fmax(0,fLow),Fs/2.0);
	fHigh = fmin(fmax(0,fHigh),Fs/2.0);

	double fStep = 2.0*Fs/(double)fftSize;

	int fLowBin = (int)floor(fLow/fStep);

	int fHighBin = (int)ceil(fHigh/fStep);

	int numBinsToRead = fmin(numBins,endBin - startBin + 1);
	int numShotsToRead = fmin(numShots,endShot - startShot + 1);

	int numBinsToSkip = numBins - numBinsToRead;

	int numWins = (int)floor(numShotsToRead/(double)fftSize);

	int startOfData = fdsHeader.HeaderSizeBytes + (startBin + numBins*startShot)*sizeof(uint16_t);

    FILE *fdsStream = fopen (fdsHeader.name.c_str(), "rb" );

    if (fdsStream==NULL) {
    	fputs ("File error",stderr);
    	exit (1);
    }

    Mat soundfield(numBinsToRead,numWins,CV_32F);

    Mat buffer(1,numBinsToRead,DataType<uint16_t>::type);

    Mat data(fftSize,numBinsToRead,DataType<uint16_t>::type);

    Mat psd;

    Scalar signal, noise;

    fseek (fdsStream, startOfData, SEEK_SET);

    for (int winNum = 0; winNum < numWins; ++winNum){
    	for (int i=0; i<fftSize; ++i){
    		fread (buffer.ptr(),sizeof(uint16_t),numBinsToRead,fdsStream);
    		buffer.copyTo(data.row(i));
    		fseek(fdsStream,numBinsToSkip*sizeof(uint16_t),SEEK_CUR);
    	}
    	debias(data);
    	psd = getPSD(data);
    	for (int i=0; i<numBinsToRead; ++i){
    		signal = mean(psd.row(i).colRange(fLowBin,fHighBin));
    		noise = mean(psd.row(i).colRange((int)(3*psd.cols/4.0),psd.cols));
    		soundfield.at<float>(i,winNum) = signal[0]/noise[0];
    	}
    }

    fclose(fdsStream);

	return soundfield;
};

Mat FDSFile::scaleForImage(Mat data){
    double min, max;
    minMaxLoc(data, &min, &max);

    Mat adjData;

    convertScaleAbs(data,data,255/(max-min),-255*min/(max-min));

    return data;
};



