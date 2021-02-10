#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "immintrin.h"

using namespace std;


const unsigned int L = 256;
const unsigned int M = 256;
const unsigned int N = 256;
const unsigned int l = 8;
const unsigned int m = 8;
const unsigned int n = 4;

float matrixA[L][M][l][m], matrixB[M][N][m][n], matrixC[L][N][l][n], matrixD[L][N][l][n], matrixMath[L][N][l][n];


double timeStart, timeEnd, timeStartTwo, timeEndTwo, timeEndThree, timeStartThree;

void printMatrix(float matr[L][N][l][n]);
bool isEqualFloat(float a, float b);
void comuteMathMatrix();
void initAll();
void showMatrixA();
void showMatrixB();
void compareMatrixes(float first[L][N][l][n], float second[L][N][l][n]);
void FMAVectorization();
void autoVec();


int main()
{
	initAll();

	autoVec();

	FMAVectorization();

	comuteMathMatrix();

	compareMatrixes(matrixC, matrixMath);

	compareMatrixes(matrixC, matrixD);
	
	system("pause");
	return 0;
}



void printMatrix(float matr[L][N][l][n])
{
	for (int i = 0; i < L; i++)
	{
		for (int j = 0; j < N; j++)
		{
			for (int k = 0; k < l; k++)
			{
				for (int v = 0; v < n; v++)
				{
					cout << matr[i][j][k][v] << " ";
				}
				cout << endl;
			}
			cout << "_" << endl;
		}
		cout << "***" << endl;
	}
	cout << "==========================================" << endl;
}

bool isEqualFloat(float a, float b)
{
	return fabs(a - b) < numeric_limits<float>::epsilon();
}


void comuteMathMatrix()
{
	timeStartThree = GetTickCount();

	for (int i = 0; i < L; i++)
		for (int j = 0; j < N; j++)
			for (int k = 0; k < M; k++)
				for (int a = 0; a < l; a++)
					for (int s = 0; s < n; s++)
						for (int d = 0; d < m; d++)
						{
							matrixMath[i][j][a][s] += matrixA[i][k][a][d] * matrixB[k][j][d][s];
						}
	timeEndThree = GetTickCount();
	cout << "Math matrix time: " << (timeEndThree - timeStartThree) / 1000 << endl;

	//printMatrix(matrixMath);
}

void initAll()
{
	float random;
	for (int i = 0; i < L; i++)
		for (int k = 0; k < M; k++)
			for (int a = 0; a < l; a++)
				for (int d = 0; d < m; d++)
				{
					matrixA[i][k][a][d] = rand() % 21 - 10;
				}

	//showMatrixA();


	for (int k = 0; k < M; k++)
		for (int j = 0; j < N; j++)
			for (int d = 0; d < m; d++)
				for (int s = 0; s < n; s++)
				{
					matrixB[k][j][d][s] = rand() % 21 - 10;
				}

	//showMatrixB();


	for (int i = 0; i < L; i++)
		for (int j = 0; j < N; j++)
			for (int a = 0; a < l; a++)
				for (int s = 0; s < n; s++)
				{
					matrixC[i][j][a][s] = 0;
				}


	for (int i = 0; i < L; i++)
		for (int j = 0; j < N; j++)
			for (int a = 0; a < l; a++)
				for (int s = 0; s < n; s++)
				{
					matrixD[i][j][a][s] = 0;
				}

}

void showMatrixA()
{
	for (int i = 0; i < L; i++)
	{
		for (int j = 0; j < N; j++)
		{
			for (int k = 0; k < l; k++)
			{
				for (int v = 0; v < m; v++)
				{
					cout << matrixA[i][j][k][v] << " ";
				}
				cout << endl;
			}
			cout << "_" << endl;
		}
		cout << "***" << endl;
	}
	cout << "==========================================" << endl;

}

void showMatrixB()
{
	for (int i = 0; i < L; i++)
	{
		for (int j = 0; j < N; j++)
		{
			for (int k = 0; k < m; k++)
			{
				for (int v = 0; v < n; v++)
				{
					cout << matrixB[i][j][k][v] << " ";
				}
				cout << endl;
			}
			cout << "_" << endl;
		}
		cout << "***" << endl;
	}
	cout << "==========================================" << endl;

}

void autoVec()
{
	timeStart = GetTickCount();
	for (int i = 0; i < L; i++)
		for (int j = 0; j < M; j++)
			for (int k = 0; k < N; k++)
				for (int a = 0; a < l; a++)
					for (int s = 0; s < m; s++)
						for (int d = 0; d < n; d++)
						{
							matrixC[i][k][a][d] += matrixA[i][j][a][s] * matrixB[j][k][s][d];
						}
	timeEnd = GetTickCount();
	cout << "Auto vectorization " << (timeEnd - timeStart) / 1000 << endl;
	//printMatrix(matrixC);
}

void FMAVectorization()
{

	__m128 firstReg;
	__m128 secondReg;
	__m128 thirdReg;

	timeStartTwo = GetTickCount();

	for (int i = 0; i < L; i++)
	{
		for (int j = 0; j < M; j++)
		{
			for (int k = 0; k < N; k++)
			{
				for (int a = 0; a < l; a++)
				{
					for (int s = 0; s < m; s++)
					{
						secondReg = _mm_loadu_ps(matrixB[j][k][s]);
						thirdReg = _mm_loadu_ps(matrixD[i][k][a]);
						firstReg = _mm_set_ps1(matrixA[i][j][a][s]);
						firstReg = _mm_fmadd_ps(firstReg, secondReg, thirdReg);
						_mm_store_ps(matrixD[i][k][a], firstReg);
					}
				}
			}
		}
	}

	timeEndTwo = GetTickCount();
	cout << "Intrisics " << (timeEndTwo - timeStartTwo) / 1000 << endl;
	//printMatrix(matrixD);
}

void compareMatrixes(float first[L][N][l][n], float second[L][N][l][n])
{
	bool result = true;
	for (int i = 0; i < L; i++)
		for (int j = 0; j < N; j++)
			for (int a = 0; a < l; a++)
				for (int s = 0; s < n; s++)
				{
					if (!isEqualFloat(first[i][j][a][s], second[i][j][a][s]))
					{
						result = false;
					}
				}
	if (result)
	{
		cout << "True" << endl;
	}
	else
	{
		cout << "False" << endl;
	}
}
