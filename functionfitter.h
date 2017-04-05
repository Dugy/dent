#ifndef FUNCTIONFITTER_H
#define FUNCTIONFITTER_H

#include <vector>
#include "formula.h"
#include <chrono>
#include <valarray>
#include "structs.h"

class functionFitter
{
public:
	functionFitter(const formula<std::valarray<float> >& func, std::vector<float>& values, curve& data_,
				   std::vector<float>& addStep, std::vector<float>& multiplyStep);
    ~functionFitter();

	float fit();
	float guess(unsigned int attempts);
	float error() { return error_; }
	void setPointsQuantity(int fitPoints = 100, int guessPoints = 15) {
		fitPoints_ = fitPoints;
		guessPoints_ = guessPoints;
	}

private:
	std::vector<float>& values_;
	const curve& data_;
	const formula<std::valarray<float>>& func_;
	std::vector<float>& addStep_;
	std::vector<float>& multiplyStep_;
	float error_;
	int fitPoints_;
	int guessPoints_;
};

#endif // FUNCTIONFITTER_H
