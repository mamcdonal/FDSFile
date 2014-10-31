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

//#include <thrust/host_vector.h>
//#include <thrust/device_vector.h>
//#include <cufft.h>

using namespace std;
using namespace cv;
//using namespace thrust;

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

	startBin = fmin(fmax(0,startBin),numBins-1);
	endBin = fmin(fmax(0,endBin),numBins-1);

	startShot = fmin(fmax(0,startShot),numShots-1);
	endShot = fmin(fmax(0,endShot),numShots-1);

	int numBinsToRead = fmin(numBins,endBin - startBin + 1);
	int numShotsToRead = fmin(numShots,endShot - startShot + 1);

	int numBinsToSkip = numBins - numBinsToRead;

	int startOfData = fdsHeader.HeaderSizeBytes + (startBin + numBins*startShot)*sizeof(uint16_t);

    FILE *fdsStream = fopen (fdsHeader.name.c_str(), "rb" );

    if (fdsStream==NULL) {
    	fputs ("File error",stderr);
    	exit (1);
    }

    Mat data(numShotsToRead,numBinsToRead,DataType<uint16_t>::type);
    Mat buffer(1,numBinsToRead,DataType<uint16_t>::type);

    fseek (fdsStream, startOfData, SEEK_SET);
    for (int i=0; i<numShotsToRead; ++i){
		fread (buffer.ptr(),sizeof(uint16_t),numBinsToRead,fdsStream);
		buffer.copyTo(data.row(i));
//    	fread(data.ptr(i,0),sizeof(uint16_t),numBinsToRead,fdsStream);
    	fseek(fdsStream,numBinsToSkip*sizeof(uint16_t),SEEK_CUR);
    }

    fclose(fdsStream);

	return data;
};

void FDSFile::debiasRows(Mat &data){

	data.convertTo(data,CV_32F,1,-pow(2,15));

	for (int i=0; i<data.rows; ++i)
			data.row(i) -= mean(data.row(i));
}

Mat FDSFile::getPSD(Mat &data){
	Mat psdData;

	Mat dftData;

	dft(data,dftData,DFT_ROWS | DFT_SCALE | DFT_COMPLEX_OUTPUT);

	vector<Mat> ReImDFT(2);

	split(dftData,ReImDFT);

	magnitude(ReImDFT[0],ReImDFT[1],psdData);

	return psdData.colRange(0,(int)ceil(psdData.cols/2)+1);
};

//Mat FDSFile::getPSDGPU(Mat data){
//	int numRows = data.rows;
//	int numCols = data.cols;
//
//	device_vector<float> deviceData(data.begin<float>(),data.end<float>());
//
//	cufftComplex *fftData;
//	cudaMalloc((void**)&fftData,numRows*(numCols/2+1)*sizeof(cufftComplex));
//
//	cufftHandle cufftPlan;
//
//	int rank = 1;
//	int n[1] = {numCols};
//	int idist = numCols;
//	int odist = numCols/2+1;
//	int inembed[] = {numCols};
//	int onembed[] = {numCols/2+1};
//	int istride = 1;
//	int ostride = 1;
//	int batch = numRows;
//
//	cufftPlanMany(&cufftPlan,
//			rank,
//			n,
//			inembed,istride, idist,
//			onembed, ostride,odist,
//			CUFFT_R2C,
//			batch);
//
//	cufftExecR2C(cufftPlan, raw_pointer_cast(deviceData.data()), fftData);
//
//	Mat psdData(numRows,numCols/2+1,CV_32F);
//
//	cudaMemcpy(psdData.data,fftData,sizeof(float)*numRows*(numCols/2+1),cudaMemcpyDeviceToHost);
//
//	cufftDestroy(cufftPlan);
//	cudaFree(fftData);
//
//	psdData = abs(psdData);
//
//	return psdData;
//};

Mat FDSFile::getSoundfield(int startBin,int endBin, int startShot, int endShot, int fftSize, int overlap, double fLow, double fHigh){

	Mat soundfield;
    Mat buffer;
    Mat data;
    Mat newData;
    Mat psd;

	int numBins = stoi(fdsHeader.getValue("DataLocusCount"));
	int numShots = stoi(fdsHeader.getValue("TimeStepCount"));
	int Fs = stoi(fdsHeader.getValue("TimeStepFrequency_Hz"));

	startBin = fmin(fmax(0,startBin),numBins-1);
	endBin = fmin(fmax(0,endBin),numBins-1);

	startShot = fmin(fmax(0,startShot),numShots-1);
	endShot = fmin(fmax(0,endShot),numShots-1);

	int numBinsToRead = fmin(numBins,endBin - startBin + 1);
	int numShotsToRead = fmin(numShots,endShot - startShot + 1);

	int numBinsToSkip = numBins - numBinsToRead;

	int numWins = (int)floor(numShotsToRead/(double)(fftSize-overlap));

	int hopSize = fftSize-overlap;

	int startOfData = fdsHeader.HeaderSizeBytes + (startBin + numBins*startShot)*sizeof(uint16_t);

	fLow = fmin(fmax(0,fLow),Fs/2.0);
	fHigh = fmin(fmax(0,fHigh),Fs/2.0);

	double fStep = Fs/(double)fftSize;

	int fLowBin = (int)floor(fLow/fStep);

	int fHighBin = (int)ceil(fHigh/fStep);

    FILE *fdsStream = fopen (fdsHeader.name.c_str(), "rb" );

    if (fdsStream==NULL) {
    	fputs ("File error",stderr);
    	exit (1);
    }

    soundfield.create(numBinsToRead,numWins,CV_32F);

    buffer.create(1,numBinsToRead,DataType<uint16_t>::type);

    data.create(fftSize,numBinsToRead,DataType<uint16_t>::type);

    newData.create(fftSize,numBinsToRead,DataType<uint16_t>::type);

    fseek (fdsStream, startOfData, SEEK_SET);

    /*Read first fftSize number of shots*/
	for (int i=0; i<fftSize; ++i){
		fread (buffer.ptr(),sizeof(uint16_t),numBinsToRead,fdsStream);
		buffer.copyTo(newData.row(i));
		fseek(fdsStream,numBinsToSkip*sizeof(uint16_t),SEEK_CUR);
	}

	newData.copyTo(data);

	data = data.t();
	debiasRows(data);
	psd = getPSD(data);
	psdToSFCol(psd,fLowBin,fHighBin).copyTo(soundfield.col(0));

    for (int winNum = 1; winNum < numWins; ++winNum){

    	if (overlap > 0)
    		newData.rowRange(hopSize,fftSize-1).copyTo(newData.rowRange(0,overlap-1));

    	for (int i=overlap; i<fftSize; ++i){
    		fread (buffer.ptr(),sizeof(uint16_t),numBinsToRead,fdsStream);
    		buffer.copyTo(newData.row(i));
    		fseek(fdsStream,numBinsToSkip*sizeof(uint16_t),SEEK_CUR);
    	}

    	newData.copyTo(data);

    	data = data.t();
    	debiasRows(data);
    	psd = getPSD(data);
    	psdToSFCol(psd,fLowBin,fHighBin).copyTo(soundfield.col(winNum));
    }

    fclose(fdsStream);

	return soundfield;
};

Mat FDSFile::getSpectrogram(int startBin, int endBin, int startShot, int endShot, int fftSize, int overlap){
	Mat spectrogram;
    Mat buffer;
    Mat data;
    Mat newData;
    Mat psd;

	int numBins = stoi(fdsHeader.getValue("DataLocusCount"));
	int numShots = stoi(fdsHeader.getValue("TimeStepCount"));

	startBin = fmin(fmax(0,startBin),numBins-1);
	endBin = fmin(fmax(0,endBin),numBins-1);

	startShot = fmin(fmax(0,startShot),numShots-1);
	endShot = fmin(fmax(0,endShot),numShots-1);

	int numBinsToRead = fmin(numBins,endBin - startBin + 1);
	int numShotsToRead = fmin(numShots,endShot - startShot + 1);

	int numBinsToSkip = numBins - numBinsToRead;

	int numWins = (int)floor(numShotsToRead/(double)(fftSize-overlap));

	int hopSize = fftSize-overlap;

	int startOfData = fdsHeader.HeaderSizeBytes + (startBin + numBins*startShot)*sizeof(uint16_t);

	FILE *fdsStream = fopen (fdsHeader.name.c_str(), "rb" );

	if (fdsStream==NULL) {
		fputs ("File error",stderr);
		exit (1);
	}

    spectrogram.create(numWins,(int)ceil(fftSize/2)+1,CV_32F);

    buffer.create(1,numBinsToRead,DataType<uint16_t>::type);

    data.create(fftSize,numBinsToRead,DataType<uint16_t>::type);

    newData.create(fftSize,numBinsToRead,DataType<uint16_t>::type);

	fseek (fdsStream, startOfData, SEEK_SET);

	/*Read first fftSize number of shots*/
	for (int i=0; i<fftSize; ++i){
		fread (buffer.ptr(),sizeof(uint16_t),numBinsToRead,fdsStream);
		buffer.copyTo(newData.row(i));
		fseek(fdsStream,numBinsToSkip*sizeof(uint16_t),SEEK_CUR);
	}

	newData.copyTo(data);

	data = data.t();
	debiasRows(data);
	psd = getPSD(data);
	vecMean(psd,1).copyTo(spectrogram.row(numWins-1));

	for (int winNum = numWins-2; winNum >= 0; --winNum){

		if (overlap > 0)
			newData.rowRange(hopSize,fftSize-1).copyTo(newData.rowRange(0,overlap-1));

		for (int i=overlap; i<fftSize; ++i){
			fread (buffer.ptr(),sizeof(uint16_t),numBinsToRead,fdsStream);
			buffer.copyTo(newData.row(i));
			fseek(fdsStream,numBinsToSkip*sizeof(uint16_t),SEEK_CUR);
		}

		newData.copyTo(data);

		data = data.t();
		debiasRows(data);
		psd = getPSD(data);
		vecMean(psd,1).copyTo(spectrogram.row(winNum));
	}

	fclose(fdsStream);

	return spectrogram;
};

void FDSFile::scaleForImage(Mat &data){
    double min, max;
    minMaxLoc(data, &min, &max);

    convertScaleAbs(data,data,255/(max-min),-255*min/(max-min));
};

Mat FDSFile::psdToSFCol(Mat &psd, int fLowBin, int fHighBin){
	float log2 = log(2);
	int numShots = psd.cols;
	Mat sfCol;

	Mat signal = vecMean(psd.colRange(fLowBin,fHighBin),2);

	Mat noise = vecMedian(psd.colRange((int)(3*(numShots-1)/4.0),numShots-1),2)/log2;

	divide(signal,noise,sfCol);

	return sfCol;
};

Mat FDSFile::vecMedian(Mat data, int dim){
	Mat medMat;
	Mat sortedData;

	if (dim ==1){
		cv::sort(data,sortedData,SORT_ASCENDING | SORT_EVERY_COLUMN);

		if (data.rows % 2 == 0){
			medMat = 0.5*(sortedData.row(data.rows/2-1) + sortedData.row(data.rows/2));
		}else{
			medMat = sortedData.row((data.rows-1)/2);
		}
	}

	if(dim == 2){
		cv::sort(data,sortedData,SORT_ASCENDING | SORT_EVERY_ROW);

		if (data.cols % 2 == 0){
			medMat = 0.5*(sortedData.col(data.cols/2-1) + sortedData.col(data.cols/2));
		}else{
			medMat = sortedData.col((data.cols-1)/2);
		}
	}

	return medMat;
};

Mat FDSFile::vecMean(Mat data, int dim){
	Mat meanMat;

	//TODO: Handle error cases when dim != 1 || dim != 2
	if (dim ==1){
		meanMat.create(1,data.cols,CV_32F);
		for (int i=0; i < data.cols; ++i)
			meanMat.col(i) = mean(data.col(i));
	}

	if(dim == 2){
		meanMat.create(data.rows,1,CV_32F);
		for (int i=0; i < data.rows; ++i)
			meanMat.row(i) = mean(data.row(i));
	}

	return meanMat;
};

