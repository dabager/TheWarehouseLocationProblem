// WarehouseLocation.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

using namespace std;

struct Warehouse
{
	int id;
	int capacity;
	int remainingCapacity;
	float setupcost;
	bool selected;
};

Warehouse *warehouses;

struct Customer
{
	int id;
	int demand;
	int assignedWarehouseId;
};

Customer *customers;

struct Individual
{
	int *genotype;
	double fitness;
	double selectionProbability;
	int expectedCopies;
	bool *warehouseList;
};

Individual *population;

float **customerWarehouseCosts;
int warehouseCount = 0, customerCount = 0, populationSize = 0;
float totalFitness = 0;

const int RANDOM_SEED = 3961, MAX_POPULATION_SIZE = 2000;

void readFile(string);
float getRandomProb();
float randomFloat(float, float);
void GA();
void GA_InitializePopulation();
void GA_CalculateFitness();
bool IndividualSort(Individual const& i1, Individual const& i2) { return i1.fitness < i2.fitness; }
void GA_Reproduction();

int main()
{
	readFile("wl_50_1");
	//GA ile yap genoptip warehouse sayýsý kadar gen, population size / customer size ters orantýlý mutasyon.

	// TO-DO : Cross-over, infeasible check, mutasyon

	srand(RANDOM_SEED);
	//float prob1 = getRandomProb();

	GA();

	system("PAUSE");
	return 0;
}

float getRandomProb()
{
	return ((float)(rand() % 101) / 100);
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
			populationSize = min(pow(customerCount, warehouseCount) / 2, (double)MAX_POPULATION_SIZE);
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
				i - 1,     // id
				capacity,  // capacity
				capacity,  // remainingCapacity
				setupCost, // setupCost
				false,     // selected
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

void GA()
{
	GA_InitializePopulation();
	GA_CalculateFitness();
	int generationCount = 0;
	while (generationCount <= 0)
	{
		sort(population, population + populationSize, IndividualSort);
		printf("Gen %2d : Min Cost (%f) - Max Cost (%f) \n", generationCount, population[0].fitness, population[populationSize - 1].fitness);
		GA_Reproduction();
		generationCount++;
	}
}

void GA_InitializePopulation()
{
	population = new Individual[populationSize];
	for (int i = 0; i < populationSize; i++)
	{
		population[i] = {
			new int[customerCount],  // genotype
			0,						 // fitness
			0,						 // selectionProbability
			0,						 // expectedCopies
			new bool[warehouseCount],// warehouseList
		};

		for (int j = 0; j < customerCount; j++)
		{
			while (true)
			{
				int selectedWarehouseIndex = rand() % (warehouseCount);

				if (warehouses[selectedWarehouseIndex].remainingCapacity > customers[j].demand)
				{
					population[i].genotype[j] = selectedWarehouseIndex;
					warehouses[selectedWarehouseIndex].remainingCapacity = warehouses[selectedWarehouseIndex].remainingCapacity - customers[j].demand;
					break;
				}
			}

		}
		for (int j = 0; j < warehouseCount; j++)
		{
			population[i].warehouseList[j] = false;
			warehouses[j].remainingCapacity = warehouses[j].capacity;
		}
	}
}

void GA_CalculateFitness()
{
	for (int i = 0; i < populationSize; i++)
	{
		population[i].fitness = 0;
		for (int j = 0; j < customerCount; j++)
		{
			int warehouseIndex = (population[i].genotype[j]);

			if (!population[i].warehouseList[warehouseIndex])
			{
				population[i].warehouseList[warehouseIndex] = true;
				population[i].fitness += warehouses[warehouseIndex].setupcost;
			}

			population[i].fitness += customerWarehouseCosts[j][warehouseIndex];
		}

		totalFitness += population[i].fitness;
	}
}

void GA_Reproduction()
{
	int totalCopies = 0;
	for (int i = 0; i < populationSize; i++)
	{
		population[i].selectionProbability =  population[i].fitness / totalFitness;
		population[i].expectedCopies = ceil(population[i].selectionProbability * populationSize);
		totalCopies += population[i].expectedCopies;
	}

	int *matingCandidates = new int[totalCopies];
	int candidateCounter = 0;
	for (int i = 0; i < populationSize; i++)
	{
		for (int j = 0; j < population[i].expectedCopies; j++)
		{
			matingCandidates[candidateCounter] = i;
			candidateCounter++;
		}
	}

	Individual *matingPool = new Individual[populationSize];
	for (int i = 0; i < populationSize; i++)
	{
		int randomParentIndex = (rand() % totalCopies);
		matingPool[i] = population[matingCandidates[randomParentIndex]];
	}
}

float randomFloat(float min, float max) {
	return  (max - min) * ((((float)rand()) / (float)RAND_MAX)) + min;
}