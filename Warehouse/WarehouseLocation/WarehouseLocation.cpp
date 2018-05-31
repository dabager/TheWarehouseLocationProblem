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
	double cost;
};

Individual* population;
Individual* matingPool;
Individual* newGeneration;

float **customerWarehouseCosts;
int warehouseCount = 0, customerCount = 0, populationSize = 0, generationCount = 0;
float totalFitness = 0;

const int RANDOM_SEED = 3961, MAX_POPULATION_SIZE = 2000, MAX_GENERATION_COUNT = 4000;
const float CROSSOVER_RATE = 0.8;

void readFile(string);
float getRandomProb();
float randomFloat(float, float);
void GA();
void GA_InitializePopulation();
void GA_CalculateFitness();
bool IndividualSort(Individual const& i1, Individual const& i2) { return i1.cost < i2.cost; }
void GA_Reproduction();
void output();

int main(int argc, char *argv[])
{
	ofstream output;
	output.open("output.txt");
	output << ("%d\n\n", argc);

	for (int i = 0; i < argc; i++)
	{

		output << ("%s\n", argv[i]);
	}

	output.close();
	if (argc == 2)
	{
		readFile(argv[1]);
		//GA ile yap genoptip warehouse sayýsý kadar gen, population size / customer size ters orantýlý mutasyon.

		// TO-DO : Cross-over, infeasible check, mutasyon

		srand(RANDOM_SEED);
		//float prob1 = getRandomProb();

		GA();

	}
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

			if (populationSize % 2 == 1) populationSize++;
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

void output()
{
	ofstream output;
	output.open("sol.txt");

	output << ("%f", population[0].cost);
	output << ("\n");

	printf("%f", population[0].cost);
	printf("\n");
	for (int i = 0; i < customerCount; i++)
	{
		printf("%d", population[0].genotype[i]);
		printf(" ");
		output << ("%d", population[0].genotype[i]); output << (" ");
	}

	output.close();
}


void GA()
{
	GA_InitializePopulation();
	GA_CalculateFitness();
	while (generationCount < MAX_GENERATION_COUNT)
	{
		sort(population, population + populationSize, IndividualSort);
		printf("Gen %2d : Min Cost (%f) - Max Cost (%f) // Best Fitness (%f) - Worst Fitness (%f) // Total Fitness (%f)\n", generationCount, population[0].cost, population[populationSize - 1].cost, population[0].fitness, population[populationSize - 1].fitness, totalFitness);
		GA_Reproduction();
		GA_CalculateFitness();
		generationCount++;
	}
	sort(population, population + populationSize, IndividualSort);
	output();
}

void GA_InitializePopulation()
{
	population = new Individual[populationSize];
	matingPool = new Individual[populationSize];
	newGeneration = new Individual[populationSize];
	for (int i = 0; i < populationSize; i++)
	{
		population[i] = {
			new int[customerCount],  // genotype
			0,						 // fitness
			0,						 // selectionProbability
			0,						 // expectedCopies
			new bool[warehouseCount],// warehouseList
			0,						 // cost
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
	totalFitness = 0;
	for (int i = 0; i < populationSize; i++)
	{
		population[i].fitness = 0;
		for (int j = 0; j < customerCount; j++)
		{
			int warehouseIndex = (population[i].genotype[j]);

			if (!population[i].warehouseList[warehouseIndex])
			{
				population[i].warehouseList[warehouseIndex] = true;
				population[i].cost += warehouses[warehouseIndex].setupcost;
			}

			population[i].cost += customerWarehouseCosts[j][warehouseIndex];
		}

		population[i].fitness = pow(populationSize, 2) / population[i].cost;

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

	for (int i = 0; i < populationSize; i++)
	{
		int randomParentIndex = (rand() % totalCopies);
		matingPool[i] = population[matingCandidates[randomParentIndex]];
	}

	for (int i = 0; i < populationSize / 2; i++)
	{
		int firstParentIndex = (rand() % populationSize);
		int secondParentIndex = (rand() % populationSize);

		Individual firstParent = matingPool[firstParentIndex];
		Individual secondParent = matingPool[secondParentIndex];

		float crossoverRandom = randomFloat(0, 1);

		if (crossoverRandom > CROSSOVER_RATE)
		{
			Individual* firstChild = new Individual{
				new int[customerCount],  // genotype
				0,						 // fitness
				0,						 // selectionProbability
				0,						 // expectedCopies
				new bool[warehouseCount],// warehouseList
				0,						 // cost
			};
			Individual* secondChild = new Individual{
				new int[customerCount],  // genotype
				0,						 // fitness
				0,						 // selectionProbability
				0,						 // expectedCopies
				new bool[warehouseCount],// warehouseList
				0,						 // cost
			};

			int crossoverPoint = (rand() % customerCount);

			for (int i = 0; i < customerCount; i++)
			{
				if (i < crossoverPoint)
				{
					(*firstChild).genotype[i] = firstParent.genotype[i];
					(*secondChild).genotype[i] = secondParent.genotype[i];
				}
				else
				{
					(*firstChild).genotype[i] = secondParent.genotype[i];
					(*secondChild).genotype[i] = firstParent.genotype[i];
				}
			}

			newGeneration[i * 2] = (*firstChild);
			newGeneration[(i * 2) + 1] = (*secondChild);

			delete firstChild;
			delete secondChild;
		}
		else
		{
			newGeneration[i * 2] = firstParent;
			newGeneration[(i * 2) + 1] = secondParent;
		}
	}

	*population = *newGeneration;
}

float randomFloat(float min, float max) {
	return  (max - min) * ((((float)rand()) / (float)RAND_MAX)) + min;
}