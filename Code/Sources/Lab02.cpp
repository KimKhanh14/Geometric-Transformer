// Lab02.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include "GeometricTransformer.h"

int main(int argc, char* argv[])
{
	string option, inputPath, outputPath, interp;
	GeometricTransformer* temp = new GeometricTransformer();
	PixelInterpolate* i;
	Mat desImg;
	int check = -1;

	if (argc == 5) {
		option = argv[1];
		interp = argv[2];
		inputPath = argv[3];
		outputPath = argv[4];

		Mat sourceImg = imread(inputPath);
		if (interp == "-bl") {
			i = new BilinearInterpolate();
		}
		else {
			i = new NearestNeighborInterpolate();
		}

		if (option == "-flipV") {
			check = temp->Flip(sourceImg, desImg, 0, i);
			imwrite(outputPath, desImg);
		}
		else {
			check = temp->Flip(sourceImg, desImg, 1, i);
		}
		imwrite(outputPath, desImg);
		if (check == 1) cout << "Done\n"; else cout << "Fail\n";
	}
	else if (argc == 6) {
		option = argv[1];
		interp = argv[2];
		float angle = stof(argv[3]);
		inputPath = argv[4];
		outputPath = argv[5];

		Mat sourceImg = imread(inputPath);

		if (interp == "-bl") {
			i = new BilinearInterpolate();
		}
		else {
			i = new NearestNeighborInterpolate();
		}

		if (option == "-rotK") {
			check = temp->RotateUnkeepImage(sourceImg, desImg, angle, i);
		}
		else {
			check = temp->RotateKeepImage(sourceImg, desImg, angle, i);
		}
		imwrite(outputPath, desImg);
		if (check == 1) cout << "Done\n"; else cout << "Fail\n";
	}
	else if (argc == 7) {
		option = argv[1];
		interp = argv[2];
		float a = stof(argv[3]);
		float b = stof(argv[4]);
		inputPath = argv[5];
		outputPath = argv[6];

		Mat sourceImg = imread(inputPath);

		if (interp == "-bl") {
			i = new BilinearInterpolate();
		}
		else {
			i = new NearestNeighborInterpolate();
		}

		if (option == "-zoom") {
			check = temp->Scale(sourceImg, desImg, a, b, i);
		}
		else {
			check = temp->Resize(sourceImg, desImg, a, b, i);
		}
		imwrite(outputPath, desImg);
		if (check == 1) cout << "Done\n"; else cout << "Fail\n";
	}
}
