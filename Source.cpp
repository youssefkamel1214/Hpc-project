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

int* inputImage(int* w, int* h, System::String^ imagePath) //put the size of image in w & h
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
	input = new int[BM.Height * BM.Width];
	for (int i = 0; i < BM.Height; i++)
	{
		for (int j = 0; j < BM.Width; j++)
		{
			System::Drawing::Color c = BM.GetPixel(j, i);
			input[i * BM.Width + j] = ((c.R + c.B + c.G) / 3); //gray scale value equals the average of RGB values
		}

	}
	return input;
}








int* outputImage(int* w, int* h,int*mean,int*saveimage) //put the size of image in w & h
{
	int* input;


	int OriginalImageWidth, OriginalImageHeight, backgroundImageWidth, backgroundImageHeight;

	//*********************************************************Read Image and save it to local arrayss*************************	
	//Read Image and save it to local arrayss


	input = new int[(*w)*(*h)];
	for (int i = 0; i < *h; i++)
	{
		for (int j = 0; j <*w; j++)
		{
	
			//cout<< ((c.R + c.B + c.G) / 3)<<"                 B is   " << ((B.R + B.B + B.G) / 3) << endl;
			input[i * (*w) + j] = mean[i * (*w) + j]- saveimage[i * (*w) + j]; //gray scale value equals the average of RGB values

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
	MyNewImage.Save("D:\\study\\third year\\second term\\high perfomance\\projects\\Hpc project\\output\\"
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
	MyNewImage.Save("D:\\study\\third year\\second term\\high perfomance\\projects\\Hpc project\\output\\"
		+ marshal_as<System::String^>(index) + ".jpg");
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
		img = "D:\\study\\third year\\second term\\high perfomance\\projects\\Hpc project\\input\\in";
		img += strstream.str() + ".jpg";
		saveimg[i % imgework] = inputImage(&w, &h, marshal_as<System::String^>(img));
		
	}
	MPI_Barrier(MPI_COMM_WORLD);
	start_s = clock();
	sum = new int[w * h]{};
	for (int i = 0; i < imgework; i++)
	{
		for (int j = 0; j < w * h; j++)
		{
			sum[j] += saveimg[i][j];
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
	else
	{
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j <w; j++)
			{
				//i * OriginalImageWidth + j
				if (mean[i * w + j] < 0)
				{
					mean[i * w + j] = 0;
				}
				if (mean[i * w + j] > 255)
				{
					mean[i * w + j] = 255;
				}
			}
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);

	for (int i = st_ind; i <= end_ind; i++)
	{
		stringstream strstream;
		strstream << setw(6) << setfill('0') << i;
		img = "D:\\study\\third year\\second term\\high perfomance\\projects\\Hpc project\\input\\in";
		img += strstream.str() + ".jpg";
		create_frontground_Image(outputImage(&w, &h, mean,saveimg[i%imgework]), w, h, i, tz, "fff");

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



