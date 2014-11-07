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
#include "grapher.h"

#include <argparser.h>
#include "FDSFile.h"

using namespace cv;
using namespace std;

typedef enum displayType {
	HEADER,
	RAW,
	PSD,
	SOUNDFIELD,
	SPECTROGRAM,
	UNKNOWN
} displayType_e;

displayType_e getDisplayType(string input){
	displayType_e displayType;

	if (input.compare("header") == 0){
		displayType = HEADER;
	}else if (input.compare("raw") == 0){
		displayType = RAW;
	}else if (input.compare("psd") == 0){
		displayType = PSD;
	}else if (input.compare("soundfield") == 0 ){
		displayType = SOUNDFIELD;
	}else if (input.compare("spectrogram") == 0 ){
		displayType = SPECTROGRAM;
	}else{
		displayType = UNKNOWN;
	}

	return displayType;
}

struct myArgParser:argparse::argparser{
	string fdsFilename;
	string displayType;
	int startBin, endBin,startShot,endShot,fftSize,overlap;
	double fLow,fHigh;

	myArgParser():fdsFilename(),displayType(),startBin(0),endBin(0),startShot(0),endShot(0),fftSize(0),overlap(0),fLow(0),fHigh(0){
		register_argument('f',"FDS Filename","Name of FDS file to process.",fdsFilename,false,true);
		register_argument('d',"Display Type","Display type:\"header\", \"raw\", \"psd\", \"soundfield\" or \"spectrogram\".",displayType,false,true);
		register_argument('a',"Start Bin","First bin to process.",startBin,false,false);
		register_argument('b',"End Bin","Last bin to process.",endBin,false,false);
		register_argument('x',"Start Shot","First shot to process.",startShot,false,false);
		register_argument('y',"End Shot","Last shot to process.",endShot,false,false);
		register_argument('n',"FFT Size","Number of shots per fft for soundfield or spectrogram.",fftSize,false,false);
		register_argument('o',"Overlap","Number of shot to overlap for soundfield or spectrogram.",overlap,false,false);
		register_argument('l',"Low-cut","Low frequency cutoff for soundfield display",fLow,false,false);
		register_argument('m',"High-cut","high frequency cutoff for soundfield display",fHigh,false,false);
	}

	void assign_values(){
		assign_value(fdsFilename,'f');
		assign_value(displayType,'d');
		assign_value(startBin,'a');
		assign_value(endBin,'b');
		assign_value(startShot,'x');
		assign_value(endShot,'y');
		assign_value(fftSize,'n');
		assign_value(overlap,'o');
		assign_value(fLow,'l');
		assign_value(fHigh,'m');
	}

//	TODO handle post parse
	void post_parse(){
//		if (getDisplayType(displayType) == RAW){
//			if (was_used('f') && was_used('v')) {
//				if (!was_used('b')) {
//				}
//			}
//		}else
		if (getDisplayType(displayType) == UNKNOWN){
			throw_error("Invalid display type.");
		}
	}
};

int main(int argc, char * argv[]){

	myArgParser args;

	args.safe_parse(argc,argv);

	displayType_e displayType = getDisplayType(args.displayType);

	FDSFile fdsfile = FDSFile(args.fdsFilename);

	int firstBin = args.startBin;
	int lastBin = args.endBin;
	int firstShot = args.startShot;
	int lastShot = args.endShot;

	float m0 = stof(fdsfile.fdsHeader.getValue("PositionOfFirstSample_m"));
	float t0 = 0;
	float f0 = 0;

	int numSamples_m;
	int numSamples_t;
	int numSamples_f;

	float Fs = stof(fdsfile.fdsHeader.getValue("TimeStepFrequency_Hz"));

	float spacing_m = stof(fdsfile.fdsHeader.getValue("DataLocusSpacing_m"));
	float spacing_t;
	float spacing_f = Fs/(2.0*numSamples_f);

	vector<float> meters;

	vector<float> time;

	vector<float> frequency;

	Mat data;

	if (displayType == HEADER){
		fdsfile.fdsHeader.printHeader();
	}else{
		if (displayType == RAW || displayType == PSD){

			data = fdsfile.getData(firstBin, lastBin, firstShot, lastShot);

			data = data.t();
			fdsfile.debiasRows(data);

			numSamples_m = data.rows;
			numSamples_t = data.cols;

			spacing_t = 1.0/Fs;

			meters = fdsfile.getSamplePoints(m0,numSamples_m,spacing_m);
			time = fdsfile.getSamplePoints(t0,numSamples_t,spacing_t);

//			int len = numSamples_t*numSamples_m;
//			vector<float> vData(len);
//			float* pData = reinterpret_cast<float*>(data.data);
//			std::copy(pData,pData + len, vData.begin());

			grapher::imagesc(time.data(),meters.data(),(float*)data.data,numSamples_m,numSamples_t);
		}

		if (displayType == PSD){

			data = fdsfile.getPSD(data);

			numSamples_m = data.rows;
			numSamples_f = data.cols;

			spacing_f = Fs/(2.0*numSamples_f);

			meters = fdsfile.getSamplePoints(m0,numSamples_m,spacing_m);
			frequency = fdsfile.getSamplePoints(f0,numSamples_f,spacing_f);

			grapher::imagesc(frequency.data(),meters.data(),(float*)data.data,numSamples_m,numSamples_f);
		}

		if (displayType == SOUNDFIELD){

			data = fdsfile.getSoundfield(args.startBin,args.endBin,args.startShot,args.endShot,args.fftSize,args.overlap,args.fLow,args.fHigh);

			numSamples_m = data.rows;
			numSamples_t = data.cols;

			spacing_t = (args.fftSize-args.overlap)/Fs;

			meters = fdsfile.getSamplePoints(m0,numSamples_m,spacing_m);
			time = fdsfile.getSamplePoints(t0,numSamples_t,spacing_t);

			grapher::imagesc(time.data(),meters.data(),(float*)data.data,numSamples_m,numSamples_t);
		}

		if (displayType == SPECTROGRAM){

//			vector<float> frequency = fdsfile.getSamplePoints(f0,numSamples_f,spacing_f);
//			vector<float> time = fdsfile.getSamplePoints(t0,numSamples_t,spacing_t);

			data = fdsfile.getSpectrogram(args.startBin,args.endBin,args.startShot,args.endShot,args.fftSize,args.overlap);
			log(1+data,data);
			resize(data,data, Size(1280,720), 0, 0, INTER_CUBIC);
		}

		fdsfile.scaleForImage(data);

		applyColorMap(data,data,COLORMAP_JET);

		namedWindow( "Display Data", CV_WINDOW_NORMAL);
		imshow("Display Data", data);

		waitKey(0);
	}
    return 0;
}

