// WarehouseLocation.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

struct Warehouse
{
	int id;
	int capacity;
	float setupcost;
};

Warehouse *warehouses;

struct Customer
{
	int id;
	int demand;
	int assignedWarehouseId;
};

Customer *customers;

float **customerWarehouseCosts;
int warehouseCount = 0, customerCount = 0;
/*
3 4
100 100.123
100 100.456
500 100.789
50
100.1 200.2 2000.3
50
100.4 200.5 2000.6
75
200.7 100.8 2000.9
75
200.1 200.11 100.12

*/

void readFile(string);

int main()
{
	readFile("wl_3_1");
	//GA ile yap genoptip warehouse sayýsý kadar gen, population size / customer size ters orantolo mutasyon.
	system("PAUSE");
	return 0;
}

void readFile(string filename)
{
	ifstream infile(filename);
	string line;
	int i = 0, customerIndexCounter = 0;
	bool customerLineChecker = false;
	while (getline(infile, line))
	{
		std::istringstream iss(line);

		if (i == 0)
		{
			iss >> warehouseCount >> customerCount;
			warehouses = new Warehouse[warehouseCount];
			customers = new Customer[customerCount];
			customerWarehouseCosts = new float*[customerCount];
			for (int j = 0; j < customerCount; j++)
			{
				customerWarehouseCosts[j] = new float[warehouseCount];
			}
		}
		else if (i > 0 && i <= warehouseCount)
		{
			int capacity = 0;
			float setupCost = 0;

			iss >> capacity >> setupCost;

			warehouses[i - 1] = {
				i - 1,         // id
				capacity,  // capacity
				setupCost, // setupCost
			};
		}
		else
		{
			int demand = 0;
			if (!customerLineChecker)
			{
				customers[customerIndexCounter] = {
					customerIndexCounter, // id
					0,					  // demand
					-1,				  // assignedWarehouseId
				};
				iss >> customers[customerIndexCounter].demand;
			}
			else
			{
				for (int k = 0; k < warehouseCount; k++)
				{
					iss >> customerWarehouseCosts[customerIndexCounter][k];
				}
				customerIndexCounter++;
			}

			customerLineChecker = !customerLineChecker;
		}

		i++;
	}
}

