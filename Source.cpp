#include <iostream>
#include <math.h>
#include <stdlib.h>
#include<string.h>
#include<msclr\marshal_cppstd.h>
#include<mpi.h>
#include <iomanip>

#include<sstream>
#include <ctime>// include this header 
#pragma once

#using <mscorlib.dll>
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
using namespace std;
using namespace msclr::interop;

int* inputImage(int* w, int* h, System::String^ imagePath, int* sum) //put the size of image in w & h
{
	int* input;


	int OriginalImageWidth, OriginalImageHeight;

	//*********************************************************Read Image and save it to local arrayss*************************	
	//Read Image and save it to local arrayss

	System::Drawing::Bitmap BM(imagePath);

	OriginalImageWidth = BM.Width;
	OriginalImageHeight = BM.Height;
	*w = BM.Width;
	*h = BM.Height;
	/*if (!inilized) {
		sum = new int[BM.Height * BM.Width]{};
		inilized = true;
	}*/
	input = new int[BM.Height * BM.Width];
	for (int i = 0; i < BM.Height; i++)
	{
		for (int j = 0; j < BM.Width; j++)
		{
			System::Drawing::Color c = BM.GetPixel(j, i);


			input[i * BM.Width + j] = ((c.R + c.B + c.G) / 3); //gray scale value equals the average of RGB values
			if (sum != nullptr)
				sum[i * BM.Width + j] += ((c.R + c.B + c.G) / 3);
		}

	}
	return input;
}








int* outputImage(int* w, int* h, System::String^ imagePath) //put the size of image in w & h
{
	int* input;


	int OriginalImageWidth, OriginalImageHeight, backgroundImageWidth, backgroundImageHeight;

	//*********************************************************Read Image and save it to local arrayss*************************	
	//Read Image and save it to local arrayss

	System::Drawing::Bitmap BM(imagePath);
	System::Drawing::Bitmap CR("C:\\Users\\Ahmed\\Desktop\\output\\estaamitedmean.png");
	backgroundImageWidth = CR.Width;
	backgroundImageHeight = CR.Height;
	OriginalImageHeight = BM.Height;
	OriginalImageWidth = BM.Width;
	/*if (!inilized) {
		sum = new int[BM.Height * BM.Width]{};
		inilized = true;
	}*/
	input = new int[BM.Height * BM.Width];
	for (int i = 0; i < BM.Height; i++)
	{
		for (int j = 0; j < BM.Width; j++)
		{
			System::Drawing::Color c = BM.GetPixel(j, i);
			System::Drawing::Color B = CR.GetPixel(j, i);

			//cout<< ((c.R + c.B + c.G) / 3)<<"                 B is   " << ((B.R + B.B + B.G) / 3) << endl;
			input[i * BM.Width + j] = (((B.R + B.B + B.G) / 3) - ((c.R + c.B + c.G) / 3)); //gray scale value equals the average of RGB values

		}

	}
	return input;
}











void create_frontground_Image(int* image, int width, int height, int counter, int th, string index)
{
	System::Drawing::Bitmap MyNewImage(width, height);


	for (int i = 0; i < MyNewImage.Height; i++)
	{
		for (int j = 0; j < MyNewImage.Width; j++)
		{
			//cout<< image[i * width + j] <<endl;
			//i * OriginalImageWidth + j
			if (image[i * width + j] < 0)
			{
				image[i * width + j] = ((image[i * width + j]) * (-1));
			}
			else if (image[i * width + j] > 255)
			{
				image[i * width + j] = 255;
			}
			if (image[i * width + j] < th)
			{
				image[i * width + j] = 0;
			}
			//cout << image[i * width + j] << endl;

			System::Drawing::Color c = System::Drawing::Color::FromArgb(image[i * MyNewImage.Width + j], image[i * MyNewImage.Width + j], image[i * MyNewImage.Width + j]);
			MyNewImage.SetPixel(j, i, c);
		}
	}
	MyNewImage.Save("C:\\Users\\Ahmed\\Desktop\\output\\"
		+ marshal_as<System::String^>(index) + counter + ".png");
}







void createImage(int* image, int width, int height, string index)
{
	System::Drawing::Bitmap MyNewImage(width, height);


	for (int i = 0; i < MyNewImage.Height; i++)
	{
		for (int j = 0; j < MyNewImage.Width; j++)
		{
			//i * OriginalImageWidth + j
			if (image[i * width + j] < 0)
			{
				image[i * width + j] = 0;
			}
			if (image[i * width + j] > 255)
			{
				image[i * width + j] = 255;
			}
			System::Drawing::Color c = System::Drawing::Color::FromArgb(image[i * MyNewImage.Width + j], image[i * MyNewImage.Width + j], image[i * MyNewImage.Width + j]);
			MyNewImage.SetPixel(j, i, c);
		}
	}
	MyNewImage.Save("C:\\Users\\Ahmed\\Desktop\\output\\"
		+ marshal_as<System::String^>(index) + ".png");
}


int main()
{
	MPI_Init(NULL, NULL);
	int size, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int nimage, tz;//tz==th
	if (rank == 0)
	{
		cin >> nimage >> tz;
	}
	MPI_Bcast(&nimage, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&tz, 1, MPI_INT, 0, MPI_COMM_WORLD);
	int start_s, stop_s, TotalTime = 0;
	System::String^ imagePath;
	std::string img;
	start_s = clock();
	int** saveimg, * sum = nullptr, * mean;
	int imgework = nimage / size, image_mod = nimage % size;
	int st_ind = (rank * imgework) + 1, end_ind = ((rank + 1) * imgework);
	int w, h;
	if (rank == size - 1) {
		end_ind += image_mod;
		imgework += image_mod;
	}
	saveimg = new int* [imgework];
	for (int i = st_ind; i <= end_ind; i++)
	{
		stringstream strstream;
		strstream << setw(6) << setfill('0') << i;
		img = "C:\\Users\\Ahmed\\Desktop\\BackGround\\in";
		img += strstream.str() + ".jpg";
		saveimg[i % imgework] = inputImage(&w, &h, marshal_as<System::String^>(img), sum);
		if (sum == nullptr) {
			sum = new int[w * h]{};
			for (int j = 0; j < w * h; j++)
				sum[j] += saveimg[i % imgework][j];
		}


	}
	mean = new int[w * h]{};
	MPI_Allreduce(sum, mean, w * h, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	for (int i = 0; i < w * h; i++)
		mean[i] /= nimage;
	if (rank == 0)
	{
		createImage(mean, w, h, "estaamitedmean");
	}
	MPI_Barrier(MPI_COMM_WORLD);

	for (int i = st_ind; i <= end_ind; i++)
	{
		stringstream strstream;
		strstream << setw(6) << setfill('0') << i;
		img = "C:\\Users\\Ahmed\\Desktop\\BackGround\\in";
		img += strstream.str() + ".jpg";
		create_frontground_Image(outputImage(&w, &h, marshal_as<System::String^>(img)), w, h, i, tz, "fff");

	}
	MPI_Barrier(MPI_COMM_WORLD);
	stop_s = clock();
	TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
	if (rank == 0)
		cout << "time: " << TotalTime << endl;

	free(saveimg);
	free(sum);
	free(mean);
	MPI_Finalize();
	return 0;

}



