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

#include <argparser.h>
#include "FDSFile.h"
//#include "FDSHeader.h"

using namespace cv;
using namespace std;

typedef enum displayType {
	RAW,
	PSD,
	SOUNDFIELD,
	SPECTROGRAM,
	UNKNOWN
} displayType_e;

displayType_e getDisplayType(string input){
	displayType_e displayType;

	if (input.compare("raw") == 0){
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
		register_argument('d',"Display Type","Display type: \"raw\", \"psd\", \"soundfield\" or \"spectrogram\".",displayType,false,true);
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

//	void post_parse(){
//		if (getDisplayType(displayType) == RAW){
//			if (was_used('f') && was_used('v')) {
//				if (!was_used('b')) {
//				}
//			}
//		}else if (getDisplayType(displayType) == UNKNOWN){
//			throw_error("Invalid display type.");
//		}
//	}
};

int main(int argc, char * argv[]){

	myArgParser args;

	args.safe_parse(argc,argv);

	displayType_e displayType = getDisplayType(args.displayType);

	FDSFile fdsfile = FDSFile(args.fdsFilename);

	fdsfile.fdsHeader.printHeader();

	Mat data;

	if (displayType == RAW || displayType == PSD){
		data = fdsfile.getData(args.startBin,args.endBin,args.startShot,args.endShot);
	}

	if (displayType == PSD){
		data = data.t();
		fdsfile.debiasRows(data);
		data = fdsfile.getPSD(data);
//		data = fdsfile.getPSDGPU(data);
	}

	if (displayType == SOUNDFIELD){
		data = fdsfile.getSoundfield(args.startBin,args.endBin,args.startShot,args.endShot,args.fftSize,args.overlap,args.fLow,args.fHigh);
	}

	if (displayType == SPECTROGRAM){
		data = fdsfile.getSpectrogram(args.startBin,args.endBin,args.startShot,args.endShot,args.fftSize,args.overlap);
		log(1+data,data);
		resize(data,data, Size(1280,720), 0, 0, INTER_CUBIC);
	}

	fdsfile.scaleForImage(data);

	applyColorMap(data,data,COLORMAP_JET);

	namedWindow( "Display Data", CV_WINDOW_NORMAL);
	imshow("Display Data", data);

	cout << "Done!" << endl;

	waitKey(0);

    return 0;
}

