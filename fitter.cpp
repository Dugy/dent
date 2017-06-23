#include "fitter.h"
#include "structs.h"
#include <iostream>
#include <cmath>
#include "settings.h"
#include "functionfitter.h"
#include "log.h"

std::string fit(curve& from, curve& result, graph* data, curveDirection dir) {
	// Prepare
	if (result.points) delete result.points;
	result.start = from.start;
	result.end = from.end;
	Settings& settings = Settings::get();

	// The fitting
	std::vector<float> values(dir == DOWN ? settings.downVariables.size()
										  : settings.upVariables.size());
	std::vector<float> stepAdd(values.size());
	std::vector<float> stepMultiply(values.size());
	formula<std::valarray<float>>& func = (dir == UP)
			? settings.upFormula : settings.downFormula;
	functionFitter fitter(func, values, from, stepAdd, stepMultiply);
	fitter.setPointsQuantity(settings.fitPoints, settings.fitGuessPoints);
	fitter.guess(settings.fitGuessSteps);
	fitter.fit();
	data->fittedVars[dir] = values;

	// Compute data from the fitted formula
	result.points = new float[from.length];
	result.length = from.length;
	float step = (from.end - from.start) / from.length;
	float pos = from.start;
	std::valarray<float>* input = new std::valarray<float>[values.size()];
	for (unsigned int i = 0; i < values.size(); i++) {
		input[i] = std::valarray<float>(values[i], 1);
	}
	for (unsigned int i = 0; i < result.length; i++) {
		input[0][0] = pos;
		std::valarray<float> got = func(&input[0]);
		result.points[i] = got[0];
		pos += step;
	}
	delete[] input;

	std::string report = dir == DOWN ? "Fitted values for descending curve: "
									 : "Fitted values for ascending curve: ";
	for (unsigned int i = 1; i < values.size(); i++)
		report += " " + (dir == DOWN ? settings.downVariables[i] : settings.upVariables[i])
				+ "=" + to_string(values[i]);
	report += "\n";
	Log::write(report);
	return ""; // Not failed
}
