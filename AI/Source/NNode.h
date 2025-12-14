#ifndef NNODE_H
#define NNODE_H

#include <vector>

struct NNode
{
	std::vector<float> weights{}; //list of weights each corresponding to a different input node
	float output{};
	float z{}; //sum of all input*corresponding weights

	//for back-propagation. we will skip this and instead borrow from genetic algorithm to train the bots
	//read up on this yourself as an extra-curricular activity
	float alpha{};
};

#endif