#include <iostream>
#include <chrono>
#include <fstream>

using namespace std;
using namespace std::chrono;

const int MAX_N = 14;

const int SIZE_L = 32 * 1024;

const int ASSOCIATIVITY = 8;

const int OFFSET = SIZE_L / ASSOCIATIVITY;

const int BLOCK_ELEMENTS_QUANTITY = 32;

const int NUMBER_OF_REPEATS = 10000000;

struct ELEMENT
{
	float a;
	float b;
	float c;
	float d;
	float e;
	float f;
	float g;
	float h;
	float i;
	float j;
	float k;
	float l;
	float m;
	float n;
	float o;
	float p;
};

ELEMENT arr[MAX_N * OFFSET];

void initialize(int N);

int main()
{	
	ofstream out;
	out.open("D:\\Java\\CacheAssociativity\\file.txt");
	if (out.is_open())
	{
		ELEMENT t;
		high_resolution_clock::time_point start, end;
		duration<double> time;
		for (int N = 2; N < MAX_N; N++)
		{
			initialize(N);
			start = high_resolution_clock::now();
			t.a = 0;
			for (int k = 0; k < BLOCK_ELEMENTS_QUANTITY * NUMBER_OF_REPEATS; k++)
			{
				t = arr[(int)t.a];
			}
			end = high_resolution_clock::now();
			time = duration_cast<duration<double>>(end - start);
			out << N << " " << time.count() << endl;
			cout << "For " << N << " time is - " << time.count() << endl;
		}
	}
	else
	{
		cout << "Something wrong" << endl;
	}
	
	system("pause");
	return 0;
}

void initialize(int N)
{
	 ELEMENT t;
	 __int32 i;

	for (int j = 0; j < BLOCK_ELEMENTS_QUANTITY / N; j++)
	{
		for (i = 0; i < N - 1; i++)
		{
			t.a = i * OFFSET + j;
			arr[(int)t.a].a = t.a + OFFSET;
		}
		t.a = i * OFFSET + j;
		if (j == BLOCK_ELEMENTS_QUANTITY / N - 1)
		{
			arr[(int)t.a].a = 0;
		}
		else
		{
			arr[(int)t.a].a = j + 1;
		}
		
	}
}