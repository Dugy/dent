#include "functionfitter.h"
#include <iostream>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <cfloat>
#include <valarray>

functionFitter::functionFitter(const formula<std::valarray<float>>& func,
				std::vector<float>& values,
				curve& data, std::vector<float>& addStep,
				std::vector<float>& multiplyStep) :
	values_(values),
	data_(data),
	func_(func),
	addStep_(addStep),
	multiplyStep_(multiplyStep),
	error_(999999999999999),
	fitPoints_(100),
	guessPoints_(15)
{

}

functionFitter::~functionFitter()
{

}

float functionFitter::fit() {
	if (values_.empty()) return -1; // Cannot
	int divisors[values_.size()];
	divisors[0] = 1;
	// 5 is somewhat a magic number, see the mutate function why is it so
	for (unsigned int i = 1; i < values_.size(); i++)
		divisors[i] = divisors[i - 1] * 5; // Pow is too slow and not really needed
	// Step size starts at some value attempting to be reasonable
	if (addStep_.size() != values_.size() || multiplyStep_.size() != values_.size()) {
		addStep_.resize(values_.size());
		multiplyStep_.resize(values_.size());
		for (unsigned int i = 1; i < values_.size(); i++) { // Yes, the zeroth element is the variable which isn't fitted
			addStep_[i] = std::max<float>(1, std::abs(values_[i] * 1.5));
			multiplyStep_[i] = 1.2;
		}
	} else {
		for (unsigned int i = 1; i < values_.size(); i++) {
			if (addStep_[i] > 1.83671e-40 && addStep_[i] < -1.83671e-40)
				addStep_[i] = 0.001;
			if (multiplyStep_[i] <= 1)
				multiplyStep_[i] = 1.2;
		}
	}

	int iIncrease = data_.length / fitPoints_;
	float step = (data_.end - data_.start) / fitPoints_;
	auto mutate = [&] (const float& mutating, const short int& mod,
			const short int& index) -> float {
		switch (mod) {
			case 1: return mutating + addStep_[index];
			case 2: return mutating - addStep_[index];
			case 3: return mutating * multiplyStep_[index];
			case 4: return mutating / multiplyStep_[index];
			default: return mutating;
		}
	};
	while (true) {
		unsigned int valarraySize = divisors[values_.size() - 1];
		std::vector<std::valarray<float>> toTry(values_.size(),
												std::valarray<float>(valarraySize));
		std::vector<float> currentValues(values_.size());
		std::vector<unsigned char> combinations;
		int index = 0;
		combinations.push_back(0);
		// Prepare values to test
		while (combinations[0] == 0) {
			while (combinations.size() < values_.size()) {
				combinations.push_back(0);
				currentValues[combinations.size() - 1] = values_[combinations.size() - 1];
			}
			for (unsigned int i = 1; i < values_.size(); i++)
				toTry[i][index] = currentValues[i];
			combinations.back()++;
			currentValues[combinations.size() - 1] = mutate(values_[combinations.size() - 1],
					combinations.back(), combinations.size() - 1);
			index++;
			while (combinations.back() == 5) {
				combinations.pop_back();
				combinations.back()++;
				currentValues[combinations.size() - 1] = mutate(values_[combinations.size() - 1],
						combinations.back(), combinations.size() - 1);
			}
		}
		std::function<std::valarray<float>(float)> valueMaker = [=]
				(float constant) -> std::valarray<float> {
			return std::valarray<float>(constant, valarraySize);
		};
		func_.setValueMaker(valueMaker);
		// Compute how well they fit
		std::valarray<float> differences(valarraySize);
		float pos = data_.start;
		for (unsigned int i = 0; i < data_.length; i += iIncrease) {
			for (unsigned int j = 0; j < valarraySize; j++)
				toTry[0][j] = pos;
			std::valarray<float> result = func_(&toTry[0]) - data_.points[i];
			differences += result * result;
			pos += step;
		}

		// Find minima - if there are more of them, some values have no effect and should make larger steps
		error_ = differences[0];
		int minPos = 0;
		unsigned short int touchedAdds[values_.size()];
		unsigned short int touchedMultiplies[values_.size()];
		for (unsigned int i = 0; i < values_.size(); i++) {
			touchedAdds[i] = 0;
			touchedMultiplies[i] = 0;
		}
		for (unsigned int i = 0; i < differences.size(); i++) {
			if (differences[i] < differences[0]) {
				if (differences[i] < error_) {
					error_ = differences[i];
					minPos = i;
				}
				index = i;
				for (unsigned int pos = values_.size() - 1; pos > 0; pos--) {
					unsigned char order = index % 5;
					if (order == 1 || order == 2) touchedAdds[pos]++;
					else if (order == 3 || order == 4) touchedMultiplies[pos]++;
					index = (index - order) / 5;
				}
			}
		}
		// Now, we simply commit the change
		index = minPos;
		for (unsigned int pos = values_.size() - 1; pos > 0; pos--) {
			unsigned char order = index % 5;
			values_[pos] = mutate(values_[pos], order, pos);
			index = (index - order) / 5;
		}
		bool makingSteps = false;
		for (unsigned int i = 1; i < values_.size(); i++) {
			// Increase those that participate in the improvement, reduce those that don't
			if (touchedAdds[i] == 0) {
				if (addStep_[i] > 1.83671e-40 || addStep_[i] < -1.83671e-40) {
					makingSteps = true;
					addStep_[i] *= 0.75;
				}
			} else {
				addStep_[i] *= 1 + (float)touchedAdds[i] / (float)valarraySize;
			}
			if (touchedMultiplies[i] == 0) {
				if (multiplyStep_[i] > 1.0001) {
					makingSteps = true;
					multiplyStep_[i] = 0.25 + multiplyStep_[i] * 0.75;
				}
			} else {
				float importance = (float)touchedMultiplies[i] / (float)valarraySize;
				multiplyStep_[i] = (1.0 - importance) * multiplyStep_[i]
						+ multiplyStep_[i] * multiplyStep_[i] * importance;
				if (multiplyStep_[i] != multiplyStep_[i]) multiplyStep_[i] = 1.2;
			}
		}
//		std::cerr << "Fitted values:";
//		for (unsigned int i = 1; i < values_.size(); i++) std::cerr << " " << values_[i];
//		std::cerr << std::endl;
//		std::cerr << "Fitting status " << minPos << " " << makingSteps << " " << error_ << std::endl;
		if (minPos == 0 && !makingSteps) break; // No change and steps are neglectful
	}
	return error_;
}

float functionFitter::guess(unsigned int attempts) {
	int variables = values_.size() - 1;
	unsigned int attemptsPerVar = pow(attempts, 1.0 / variables);
	if (attemptsPerVar > 10000) attemptsPerVar = 10000;
	int totalAttempts = attemptsPerVar;
	for (unsigned int i = 2; i < values_.size(); i++) totalAttempts *= attemptsPerVar;
	float triedValues[attemptsPerVar];
	for (unsigned int i = 0; i < attemptsPerVar / 2; i++) {
		float base = (i - attemptsPerVar / 4.0) / (attemptsPerVar / 40.0);
		triedValues[i] = pow(2, base * fabs(base));
	}
	for (unsigned int i = attemptsPerVar / 2; i < attemptsPerVar; i++) {
		float base = (i - (attemptsPerVar / 2)
					  - attemptsPerVar / 4.0) / (attemptsPerVar / 40.0);
		triedValues[i] = -1.0 * pow(2, base * fabs(base));
	}
	std::vector<std::valarray<float>> toTry(values_.size(),
									std::valarray<float>(totalAttempts));
	std::vector<unsigned int> combinations;
	float currentValues[values_.size()];
	int index = 0;
	combinations.push_back(0);

	// Prepare values to test
	while (combinations[0] == 0) {
		while (combinations.size() < values_.size()) {
			combinations.push_back(0);
			currentValues[combinations.size() - 1] = triedValues[0];
		}
		for (unsigned int i = 1; i < values_.size(); i++)
			toTry[i][index] = currentValues[i];
		combinations.back()++;
		currentValues[combinations.size() - 1] = triedValues[combinations.back()];
		index++;
		while (combinations.back() == attemptsPerVar) {
			combinations.pop_back();
			combinations.back()++;
			currentValues[combinations.size() - 1] = triedValues[combinations.back()];
		}
	}
	// Make the test
	float pos = data_.start;
	std::valarray<float> differences(totalAttempts);
	int iIncrease = data_.length / guessPoints_;
	float step = (data_.end - data_.start) / guessPoints_;
	for (unsigned int i = 0; i < data_.length; i += iIncrease) {
		for (int j = 0; j < totalAttempts; j++)
			toTry[0][j] = pos;
		std::valarray<float> result = func_(&toTry[0]) - data_.points[i];
		differences += result * result;
		pos += step;
	}

	// Find the best one
	error_ = 9999999999999999;
	int bestIndex = totalAttempts / 2;
	for (int i = 0; i < totalAttempts; i++) {
//		std::cerr << "Error " << differences[i];
//		for (unsigned int j = 1; j < values_.size(); j++) std::cerr << " " << toTry[j][i];
//		std::cerr << std::endl;
		if (differences[i] < error_) {
			error_ = differences[i];
			bestIndex = i;
		}
	}

	// Save the best one
	for (unsigned int i = 1; i < values_.size(); i++) {
		values_[i] = toTry[i][bestIndex];
	}

	for (unsigned int i = 1; i < values_.size(); i++) { // Yes, the zeroth element is the variable which isn't fitted
		addStep_[i] = std::max<float>(1, std::abs(values_[i] * 1.5));
		multiplyStep_[i] = 1.2;
	}

	return error_;
}
