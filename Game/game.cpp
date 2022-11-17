#include "game.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <iostream>
#include <fstream>
using namespace std;

static void printMat(const glm::mat4 mat)
{
	std::cout<<" matrix:"<<std::endl;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			std::cout<< mat[j][i]<<" ";
		std::cout<<std::endl;
	}
}

Game::Game() : Scene()
{
}

Game::Game(float angle ,float relationWH, float near1, float far1) : Scene(angle,relationWH,near1,far1)
{ 	
}

void Game::Init()
{		

	AddShader("../res/shaders/pickingShader");	
	AddShader("../res/shaders/basicShader");
	
	int width, height, numComponents;
	std::string fileName = "../res/textures/lena256.jpg";
	unsigned char* data = stbi_load((fileName).c_str(), &width, &height, &numComponents, 4);
	AddTexture(width, height, data);//greyScale image
	
	int size = width * height * 4;
	double degree = 3.14 / 2;
	int gaussian[] = { 1,2,1,
					   2,4,2,
					   1,2,1 };

	int xMatrix[] = { -1,0,1,
					  -2,0,2,
					  -1,0,1 };

	int yMatrix[] = { -1,-2,-1,
					  0,0,0,
				      1,2,1 };

	//gaussian filter
	vector<vector<unsigned char>>* gauFilter = new vector<vector<unsigned char>>();
	for (int row = 0; row < height; row++) {
		vector<unsigned char> temp;
		for (int count = 0; count < 4*width; count = count + 4) {
			if (0 < row && row < height - 1 && 0 < count && count < 4*width-4) {
				int matrix[] = { data[(row - 1) * 4 * width + count-4], data[(row - 1) * 4 * width + count], data[(row - 1) * 4 * width + count + 4],
								 data[row * 4 * width + count - 4], data[row * 4 * width + count], data[row * 4 * width + count + 4],
								 data[(row+1) * 4 * width + count - 4], data[(row + 1) * 4 * width + count], data[(row + 1) * 4*width + count + 4] };
				int strength = 0;
				for (int i = 0; i < 9; i++)
					strength = strength + matrix[i] * gaussian[i];
				strength = round(0.0625 * strength);
				temp.push_back((unsigned char)strength);

			}
			else {
				temp.push_back((unsigned char)0);
			}
		}
		gauFilter->push_back(temp);
	}
	//gaussian filter

	//sobel operator
	vector<vector<unsigned char>>* sobel = new vector<vector<unsigned char>>();
	vector<vector<double>>* direction = new vector<vector<double>>();
	for (int row = 0; row < height; row++) {
		vector<unsigned char> tempStrength;
		vector<double> tempDirection;
		for (int count = 0; count < width; count++) {
			if (0 < row && row < height-1 && 0 < count && count < width-1) {
				unsigned char matrix[] = { (* gauFilter)[row - 1][count-1], (*gauFilter)[row - 1][count], (*gauFilter)[row - 1][count+1],
								 (*gauFilter)[row][count-1], (*gauFilter)[row][count], (*gauFilter)[row][count+1],
								 (*gauFilter)[row + 1][count-1], (*gauFilter)[row + 1][count], (*gauFilter)[row + 1][count+1] };
				int x = 0;
				int y = 0;
				for (int i = 0; i < 9; i++) {
					x = x + xMatrix[i] * matrix[i];
					y = y + yMatrix[i] * matrix[i];
				}
				int strength = round(sqrt(x * x + y * y));
				int place = std::floor((row * 1024 + count) / 4);
				if(x!=0)
					tempDirection.push_back(atan(y/x));
				else
					tempDirection.push_back(degree);
				tempStrength.push_back(strength);


			}
			else {
				tempStrength.push_back((unsigned char)0) ;
				tempDirection.push_back((unsigned char)0);
			}

		}
		sobel->push_back(tempStrength);
		direction->push_back(tempDirection);
	}
	//sobel operator

	//non maximum suppression
	vector<vector<unsigned char>>* nonMaxSupp = new vector<vector<unsigned char>>();
	for (int row = 0; row < height; row++) {
		vector<unsigned char> temp;
		for (int count = 0; count < width; count++) {
			if (0 < row && row < height - 1 && 0 < count && count < width - 1) {
				if (-1 * degree / 4 < (*direction)[row][count] && (*direction)[row][count] <= degree / 4) {
					if ((*sobel)[row][count] < (*sobel)[row][count+1] || (*sobel)[row][count] < (*sobel)[row][count-1]) {
						temp.push_back((unsigned char)0);
					}
					else {
						temp.push_back((*sobel)[row][count]);
					}
				}
				else if (degree/4 < (*direction)[row][count] && (*direction)[row][count] <= 3*degree / 4) {
					if ((*sobel)[row][count] < (*sobel)[row+1][count-1] || (*sobel)[row][count] < (*sobel)[row-1][count+1]) {
						temp.push_back((unsigned char)0);
					}
					else {
						temp.push_back((*sobel)[row][count]);
					}
				}

				else if (3 * degree / 4 < (*direction)[row][count] && (*direction)[row][count] <= degree || -1 * degree < (*direction)[row][count] && (*direction)[row][count] <= -3 * degree / 4) {
					if ((*sobel)[row][count] < (*sobel)[row+1][count] || (*sobel)[row][count] < (*sobel)[row-1][count]) {
						temp.push_back((unsigned char)0);
					}
					else {
						temp.push_back((*sobel)[row][count]);
					}
				}
				else {
					if ((*sobel)[row][count] < (*sobel)[row + 1][count+1] || (*sobel)[row - 1][count] < (*sobel)[row - 1][count-1]) {
						temp.push_back((unsigned char)0);
					}
					else {
						temp.push_back((*sobel)[row][count]);
					}
				}
			}
			else {
				temp.push_back((unsigned char)0);
			}
		}
		nonMaxSupp->push_back(temp);
	}

	//non maximum suppression

	//thresholding
	vector<vector<unsigned char>>* thresholding = new vector<vector<unsigned char>>();
	int low = 110;
	int high = 170;
	for (int row = 0; row < height; row++) {
		vector<unsigned char> temp;
		for (int count = 0; count < width; count++) {
			if ((*nonMaxSupp)[row][count] < high) {
				if (low <= (*nonMaxSupp)[row][count]) {
					unsigned char matrix[] = { (*nonMaxSupp)[row-1][count-1], (*nonMaxSupp)[row-1][count], (*nonMaxSupp)[row-1][count+1],
								 (*nonMaxSupp)[row][count-1], (*nonMaxSupp)[row][count], (*nonMaxSupp)[row][count+1],
								 (*nonMaxSupp)[row+1][count-1], (*nonMaxSupp)[row+1][count], (*nonMaxSupp)[row+1][count+1] };
					bool check = false;
					for (int i = 0; i < 9; i++)
						if (matrix[i] >= high)
							check = true;
					if (check) {
						temp.push_back((unsigned char)255);
					}
					else {
						temp.push_back((unsigned char)0);
					}
				}
				else {
					temp.push_back((unsigned char)0);
				}

			}
			else {
				temp.push_back((unsigned char)255);
			}
		}
		thresholding->push_back(temp);
	}
	//thresholding

	ofstream myfile("./img4.txt");
	if (myfile.is_open())
	{
		for (int row = 0; row < height; row++) {
			for (int count = 0; count < width; count++) {
				int temp = round(((*thresholding)[row][count] + 1)/256);
				myfile << temp << ",";
			}
			myfile << "\n";
		}
		myfile.close();
	}


	//restoring to array
	unsigned char* image = (unsigned char*)(malloc(4 * (width) * (height)));
	for (int row = 0; row < height; row++) {
		for (int count = 0; count < width; count++) {
			image[4 * (row * width + count)] = (*thresholding)[row][count];
			image[4 * (row * width + count) + sizeof(unsigned char)] = (*thresholding)[row][count];
			image[4 * (row * width + count) + 2 * sizeof(unsigned char)] = (*thresholding)[row][count];
		}
	}
	AddTexture(width, height, image);//sobel operator image


	// halftoning
	vector<vector<unsigned char>>* halftoning = new vector<vector<unsigned char>>();
	for (int row = 0; row < height; row++) {
		vector<unsigned char> temp1;
		vector<unsigned char> temp2;
		for (int count = 0; count < 4*width; count = count+4) {
			int val = data[4*row*width+count];
			if (val < 50) {
				temp1.push_back((unsigned char)0);
				temp1.push_back((unsigned char)0);
				temp2.push_back((unsigned char)0);
				temp2.push_back((unsigned char)0);
			}
			else if (val < 101) {
				temp1.push_back((unsigned char)0);
				temp1.push_back((unsigned char)0);
				temp2.push_back((unsigned char)255);
				temp2.push_back((unsigned char)0);
			}
			else if (val < 152) {
				temp1.push_back((unsigned char)0);
				temp1.push_back((unsigned char)255);
				temp2.push_back((unsigned char)255);
				temp2.push_back((unsigned char)0);
			}
			else if (val < 203) {
				temp1.push_back((unsigned char)0);
				temp1.push_back((unsigned char)255);
				temp2.push_back((unsigned char)255);
				temp2.push_back((unsigned char)255);
			}
			else {
				temp1.push_back((unsigned char)255);
				temp1.push_back((unsigned char)255);
				temp2.push_back((unsigned char)255);
				temp2.push_back((unsigned char)255);
			}
		}
		halftoning->push_back(temp1);
		halftoning->push_back(temp2);
	}

	int newWidth = 2 * width;
	int newHeight = 2 * height;
	ofstream myfile2("./ img5.txt");
	if (myfile2.is_open())
	{
		for (int row = 0; row < newHeight; row++) {
			for (int count = 0; count < newWidth; count++) {
				int temp = round(((*halftoning)[row][count] + 1) / 256);
				myfile2 << temp << ",";
			}
			myfile2 << "\n";
		}
		myfile2.close();
	}


	unsigned char* image1 = (unsigned char*)(malloc(4 * (newWidth) * (newHeight)));
	for (int row = 0; row < newHeight; row++) {
		for (int count = 0; count < newWidth; count++) {
			int place = 4 * (row * newWidth + count);
			image1[place] = (*halftoning)[row][count];
			image1[place + 1] = (*halftoning)[row][count];
			image1[place + 2] = (*halftoning)[row][count];
		}
	}
	AddTexture(newWidth, newHeight, image1);//halftone image
	//halftoning

	//floyd steisnberg algorithm
	vector<vector<unsigned char>>* floydSteinberg = new vector<vector<unsigned char>>();
	double alpha = 7.0 / 16.0;
	double beta = 3.0 / 16.0;
	double gama = 5.0 / 16.0;
	double delta = 1.0 / 16.0;
	for (int row = 0; row < height; row++) {
		vector<unsigned char> temp;
		for (int count = 0; count < 4 * width; count = count + 4) {
			int place = 4 * row * width + count;
			unsigned char val = data[place];
			unsigned char range = 16;
			unsigned char newVal = round(range * val / 255) * round(255 / range);
			temp.push_back(newVal);
			unsigned char error = val - newVal;
			if (row < height - 1) {
				if (count == 0) {
					data[place + 1] = data[place + 1] + round(alpha * error);
					data[place + width] = data[place + width] + round(gama * error);
					data[place + width+1] = data[place + width + 1] + round(delta * error);
				}
				else if (count == 4 * width - 4) {
					data[place + width - 1] = data[place + width - 1] + round(beta * error);
					data[place + width] = data[place + width] + round(gama * error);
				}
				else {
					data[place + 1] = data[place + 1] + round(alpha * error);
					data[place + width - 1] = data[place + width - 1] + round(beta * error);
					data[place + width] = data[place + width] + round(gama * error);
					data[place + width + 1] = data[place + width + 1] + round(delta * error);
				}
			}
			else if (count < 4 * width - 4) {
				data[place + 1] = data[place + 1] + round(alpha * error);
			}

		}
		floydSteinberg->push_back(temp);
	}

	ofstream myfile3("./img6.txt");
	if (myfile3.is_open())
	{
		for (int row = 0; row < height; row++) {
			for (int count = 0; count < width; count++) {
				int temp = round(((*floydSteinberg)[row][count] + 1) / 16);
				myfile3 << temp << ",";
			}
			myfile3 << "\n";
		}
		myfile3.close();
	}

	unsigned char* image2 = (unsigned char*)(malloc(4 * width * height));
	for (int row = 0; row < height; row++) {
		for (int count = 0; count < width; count++) {
			int place = 4 * (row * width + count);
			image2[place] = (*floydSteinberg)[row][count];
			image2[place + 1] = (*floydSteinberg)[row][count];
			image2[place + 2] = (*floydSteinberg)[row][count];
		}
	}
	AddTexture(width, height, image2);//Floyd-Steinberg image


	AddShape(Plane,-1,TRIANGLES);
	
	pickedShape = 0;
	
	SetShapeTex(0,0);
	MoveCamera(0,zTranslate,10);
	pickedShape = -1;
	
	//ReadPixel(); //uncomment when you are reading from the z-buffer
}

void Game::Update(const glm::mat4 &MVP,const glm::mat4 &Model,const int  shaderIndx)
{
	Shader *s = shaders[shaderIndx];
	int r = ((pickedShape+1) & 0x000000FF) >>  0;
	int g = ((pickedShape+1) & 0x0000FF00) >>  8;
	int b = ((pickedShape+1) & 0x00FF0000) >> 16;
	s->Bind();
	s->SetUniformMat4f("MVP", MVP);
	s->SetUniformMat4f("Normal",Model);
	s->SetUniform4f("lightDirection", 0.0f , 0.0f, -1.0f, 0.0f);
	if(shaderIndx == 0)
		s->SetUniform4f("lightColor",r/255.0f, g/255.0f, b/255.0f,1.0f);
	else 
		s->SetUniform4f("lightColor",0.7f,0.8f,0.1f,1.0f);
	s->Unbind();
}

void Game::WhenRotate()
{
}

void Game::WhenTranslate()
{
}

void Game::Motion()
{
	if(isActive)
	{
	}
}

Game::~Game(void)
{
}
