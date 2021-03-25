#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <curand.h>
#include <cuda.h>
#include <iomanip>

#pragma comment(lib, "curand.lib")

using namespace std;
#define SHOW_INITIAL_SIZE 16
#define SHOW_REORDERED_WIDTH  32
#define SHOW_REORDERED_HEIGHT  8
#define N 8000
#define M 2000
#define ELEMENTS_RANGE 5
#define BLOCK_DIM_X 32
#define BLOCK_DIM_Y 32
#define GRID_DIM_X  ((M + (BLOCK_DIM_X * 2 - 1))/(2*BLOCK_DIM_X) + 1)
#define GRID_DIM_Y (((N + (BLOCK_DIM_Y * 2 - 1))/(2*BLOCK_DIM_Y) + 1)

int hostTransitMatrix[N][M];
char hostInput[N][M];
char deviceInitial[N][M];
char hostCPUResult[N / 2][M * 2];
char deviceGPUResult[N / 2][M * 2];

void handleError(cudaError_t error);
void handleStatus(curandStatus_t error);
void init(unsigned char* input, unsigned int* arr);
void mainCuda(unsigned char* devInput);
void reorder();
void checkEquality();
void showGPUResult(float time);

__global__ void kernel(unsigned char* input, unsigned char* result, unsigned int width, unsigned int height)
{
	int indexX = 2 * (blockIdx.x * blockDim.x + threadIdx.x);
	int indexY = 2 * (blockIdx.y * blockDim.y + threadIdx.y);

	if (indexY < height && indexX < width)
	{
		char first = input[indexY *  width + indexX];
		char second = input[indexY *  width + indexX + 1];
		char third = input[(indexY + 1)*  width + indexX];
		char fourth = input[(indexY + 1)*  width + indexX + 1];

		result[indexY *  width + indexX * 2] = fourth;
		result[indexY *  width + indexX * 2 + 1] = second;
		result[indexY *  width + indexX * 2 + 2] = first;
		result[indexY *  width + indexX * 2 + 3] = third;
	}
}

int main()
{
	unsigned char* deviceInput;
	unsigned int* arr;
	handleError(cudaMalloc((void**)& arr, N * M * sizeof(unsigned int)));
	handleError(cudaMalloc((void**)& deviceInput, N * M * sizeof(unsigned char)));
	init(deviceInput, arr);
	mainCuda(deviceInput);


	reorder();
	checkEquality();
	handleError(cudaFree(deviceInput));
	
	return 0;
}

void mainCuda(unsigned char* input)
{
	float GPUTime;
	cudaEvent_t start, end;
	unsigned char * result;
	handleError(cudaMalloc((void**)&result, N * M * sizeof(unsigned char)));
	dim3 _blockDim(BLOCK_DIM_X, BLOCK_DIM_Y);
	dim3 _gridDim(GRID_DIM_X, GRID_DIM_Y);


	handleError(cudaEventCreate(&start));
	handleError(cudaEventCreate(&end));
	handleError(cudaEventRecord(start));

	kernel << <_gridDim, _blockDim >> > (input, result, M, N);

	handleError(cudaEventRecord(end));
	handleError(cudaEventSynchronize(end));
	handleError(cudaEventElapsedTime(&GPUTime, start, end));


	handleError(cudaMemcpy(deviceGPUResult, result, N * M * sizeof(char), cudaMemcpyDeviceToHost));
	handleError(cudaEventDestroy(start));
	handleError(cudaEventDestroy(end));
	handleError(cudaFree(result));
	showGPUResult(GPUTime);
}

void showGPUResult(float time)
{
	cout << "===================================================================================================" << endl;
	cout << "GPU work time: " << time << endl;
	cout << "===================================================================================================" << endl;
	cout << "GPU initial part:" << endl;
	for (int i = 0; i < SHOW_INITIAL_SIZE; i++)
	{
		for (int j = 0; j < SHOW_INITIAL_SIZE; j++)
		{
			cout << setw(ELEMENTS_RANGE) << deviceInitial[i][j];
		}
		cout << endl;
	}
	cout << "===================================================================================================" << endl;	cout << "GPU result part:" << endl;
	for (int i = 0; i < SHOW_REORDERED_HEIGHT; i++)
	{
		for (int j = 0; j < SHOW_REORDERED_WIDTH; j++)
		{
			cout << setw(ELEMENTS_RANGE) << deviceGPUResult[i][j];
		}
		cout << endl;
	}
	cout << "===================================================================================================" << endl;
}

void reorder()
{
	long long start, end;
	start = GetTickCount64();
	for (int i = 0, l = 0; i < N / 2; i++, l += 2)
	{
		for (int j = 0, k = 0; j < M * 2; j += 4, k += 2)
		{
			hostCPUResult[i][j + 0] = hostInput[l + 1][k + 1];
			hostCPUResult[i][j + 1] = hostInput[l][k + 1];
			hostCPUResult[i][j + 2] = hostInput[l][k];
			hostCPUResult[i][j + 3] = hostInput[l + 1][k];
		}
	}
	end = GetTickCount64();
	cout << "CPU reorder time: " << end - start << endl;
	cout << "===================================================================================================" << endl;
	cout << "CPU initial matrix part:" << endl;
	for (int i = 0; i < SHOW_INITIAL_SIZE; i++)
	{
		for (int j = 0; j < SHOW_INITIAL_SIZE; j++)
		{
			cout << setw(ELEMENTS_RANGE) << hostInput[i][j];
		}
		cout << endl;
	}
	cout << "===================================================================================================" << endl;
	cout << "CPU reordered matrix part:" << endl;
	for (int i = 0; i < SHOW_REORDERED_HEIGHT; i++)
	{
		for (int j = 0; j < SHOW_REORDERED_WIDTH; j++)
		{
			cout << setw(ELEMENTS_RANGE) << hostCPUResult[i][j];
		}
		cout << endl;
	}
}

void checkEquality()
{
	cout << "===================================================================================================" << endl;
	cout << "===================================================================================================" << endl;

	bool result = true;
	for (int i = 0; i < N / 2; i++)
	{
		for (int j = 0; j < M * 2; j++)
		{
			if (hostCPUResult[i][j] != deviceGPUResult[i][j])
			{
				result = false;
				//cout << i << " + " << j << " = " << hostCPUResult[i][j] << " != " << deviceGPUResult[i][j] <<endl;
			}
		}
	}
	if (result)
	{
		cout << "Results are equal" << endl;
	}
	else
	{
		cout << "NOT equal results!" << endl;
	}
	cout << "===================================================================================================" << endl;

}

void init(unsigned char* input, unsigned int* arr)
{
	curandGenerator_t generator;
	handleStatus(curandCreateGenerator(&generator, CURAND_RNG_PSEUDO_DEFAULT));
	handleStatus(curandSetPseudoRandomGeneratorSeed(generator, 1234ULL));
	handleStatus(curandGeneratePoisson(generator, arr, N * M, 50));
	handleError(cudaMemcpy(hostTransitMatrix, arr, N * M * sizeof(unsigned int), cudaMemcpyDeviceToHost));
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < M; j++)
		{
			hostInput[i][j] = *(char*)&hostTransitMatrix[i][j];
			deviceInitial[i][j] = *(char*)&hostTransitMatrix[i][j];
		}
	}
	handleError(cudaMemcpy(input, hostInput, N * M * sizeof(unsigned char), cudaMemcpyHostToDevice));
	handleStatus(curandDestroyGenerator(generator));
}

void handleError(cudaError_t error)
{
	if (error != cudaSuccess)
	{
		cout << "Error: " << cudaGetErrorName(error) << endl;
		cout << "Error code - " << error << endl;
		cout << "Message - " << cudaGetErrorString(error) << endl;
		exit(-1);
	}
}

void handleStatus(curandStatus_t status)
{
	if (status != CURAND_STATUS_SUCCESS)
	{
		cout << "curand status: " << status << endl;
	}
}
