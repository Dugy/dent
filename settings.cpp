#include "settings.h"
#include <fstream>
//#include <iostream>

#pragma GCC diagnostic ignored "-Wwrite-strings"

Settings::Settings()
{
	std::ifstream file("settings.ini");
	std::map<std::string, std::string>* currentlySet = nullptr;
	if (file.is_open()) {
		std::string line;
		while (std::getline(file, line)) {
			if (line[0] == '[') {
				const std::string& profileName =
						line.substr(1, line.find_last_of("]") - 1);
				currentlySet = &(profiles[profileName]);
				if (currentProfile.empty())
					currentProfile = profileName;
			} else {
				if (!currentlySet)
					throw(std::runtime_error("Missing profile name"));
				std::string entryName =
						line.substr(0, line.find_first_of("="));
				std::string entryValue =
						line.substr(line.find_first_of("=") + 1);
				(*currentlySet)[entryName] = entryValue;
			}
		}
	} else {
		currentProfile = "Default";
	}
	setup();
}

Settings::~Settings()
{
	synchChanges();
}

std::string boolToString(bool val) {
	return val ? "true" : "false";
}

void Settings::setup() {
	std::map<std::string, std::string>& current = profiles[currentProfile];
	auto getStringVal = [&] (std::string& into, const char* key, char* ifAbsent) {
		std::string& found = current[key];
		if (found.empty()) {
			found = ifAbsent;
			into = ifAbsent;
		} else into = found;
	};
	auto getFloatVal = [&] (float& into, const char* key, float ifAbsent) {
		std::string& found = current[key];
		if (found.empty()) {
			found = std::to_string(ifAbsent);
			into = ifAbsent;
		} else into = std::stof(found);
	};
	auto getIntVal = [&] (unsigned int& into, const char* key, unsigned int ifAbsent) {
		std::string& found = current[key];
		if (found.empty()) {
			found = std::to_string(ifAbsent);
			into = ifAbsent;
		} else into = std::stoi(found);
	};
	auto getBoolVal = [&] (bool& into, const char* key, bool ifAbsent) {
		std::string& found = current[key];
		if (found.empty()) {
			found = boolToString(ifAbsent);
			into = ifAbsent;
		} else into = (found == "true");
	};
	getStringVal(folder, "folder", "");
	getIntVal(cores, "cores", 4);
	getBoolVal(customZoom, "custom_zoom", false);
	getFloatVal(xMinCustom, "x_min_custom", 0);
	getFloatVal(xMaxCustom, "x_max_custom", 100);
	getFloatVal(yMinCustom, "y_min_custom", 0);
	getFloatVal(yMaxCustom, "y_max_custom", 15000);
	getBoolVal(showDescending, "show_descending", true);
	getBoolVal(showAscending, "show_ascending", false);
	getIntVal(filterXWidth, "filter_x_width", 50);
	getFloatVal(filterXMaximum, "filter_x_maximum", 1);
	getFloatVal(filterXSlope, "filter_x_slope", 1.5);
	getIntVal(filterYWidth, "filter_y_width", 50);
	getFloatVal(filterYMaximum, "filter_y_maximum", 25);
	getFloatVal(filterYSlope, "filter_y_slope", 1.5);
	getFloatVal(filterYSlope, "filter_y_slope", 1.5);
	getIntVal(changeSampling, "change_sampling", 50);
    getFloatVal(increaseThreshold, "increase_threshold", 2);
    getFloatVal(decreaseThreshold, "decrease_threshold", 0.5);
	getBoolVal(filterEnabled, "reading_filter_enabled", true);
	getIntVal(points, "points", 500);
	getBoolVal(proceedFromLoadToDerivatives,
			   "proceed_from_load_to_derivatives", false);
	getFloatVal(startDerivativeFrom, "start_derivative_from", 20);
	getFloatVal(endDerivativeBefore, "end_derivative_before", 20);
	getFloatVal(derivativeSmoothingWidth, "derivative_smoothing_width", 3);
	getFloatVal(derivativeSmoothingSlope, "derivative_smoothing_slope", 1.8);
	getFloatVal(intoleranceThreshold, "intolerance_threshold", 4);
	getFloatVal(popinSkip, "popin_skip", 4);
	getBoolVal(proceedFromDerivativesToIntegrals,
			   "proceed_from_derivatives_to_integrals", false);
	getBoolVal(integrateSmoothed, "integrate_smoothed", false);
	readFormulaVal(downFormula, downVariables, current["down_formula"], "$var2*$x^2");
	readFormulaVal(upFormula, upVariables, current["up_formula"], "$var1+$var1*$x^2");
	getIntVal(fitGuessSteps, "fit_guess_steps", 1000000);
	getIntVal(fitGuessPoints, "fit_guess_points", 15);
	getIntVal(fitPoints, "fit_points", 100);
}

void Settings::synchChanges() {
	std::map<std::string, std::string>& current = profiles[currentProfile];
	current["folder"] = folder;
	current["cores"] = std::to_string(cores);
	current["custom_zoom"] = boolToString(customZoom);
	current["x_min_custom"] = std::to_string(xMinCustom);
	current["x_max_custom"] = std::to_string(xMaxCustom);
	current["y_min_custom"] = std::to_string(yMinCustom);
	current["y_max_custom"] = std::to_string(yMaxCustom);
	current["show_descending"] = boolToString(showDescending);
	current["show_ascending"] = boolToString(showAscending);
	current["filter_x_width"] = std::to_string(filterXWidth);
	current["filter_x_maximum"] = std::to_string(filterXMaximum);
	current["filter_x_slope"] = std::to_string(filterXSlope);
	current["filter_y_width"] = std::to_string(filterYWidth);
	current["filter_y_maximum"] = std::to_string(filterYMaximum);
	current["filter_y_slope"] = std::to_string(filterYSlope);
	current["change_sampling"] = std::to_string(changeSampling);
	current["increase_threshold"] = std::to_string(increaseThreshold);
	current["decrease_threshold"] = std::to_string(decreaseThreshold);
	current["filter_enabled"] = boolToString(filterEnabled);
	current["points"] = std::to_string(points);
	current["proceed_from_load_to_derivatives"] =
			boolToString(proceedFromLoadToDerivatives);
	current["start_derivative_from"] = std::to_string(startDerivativeFrom);
	current["end_derivative_before"] = std::to_string(endDerivativeBefore);
	current["derivative_smoothing_width"] = std::to_string(derivativeSmoothingWidth);
	current["derivative_smoothing_slope"] = std::to_string(derivativeSmoothingSlope);
	current["intolerance_threshold"] = std::to_string(intoleranceThreshold);
	current["popin_skip"] = std::to_string(intoleranceThreshold);
	current["proceed_from_derivatives_to_integrals"] =
			boolToString(proceedFromDerivativesToIntegrals);
	current["integrate_smoothed"] = boolToString(integrateSmoothed);
	current["up_formula"] = upFormula.print(upVariables);
	current["down_formula"] = downFormula.print(downVariables);
	current["fit_guess_steps"] = std::to_string(fitGuessSteps);
	current["fit_guess_points"] = std::to_string(fitGuessPoints);
	current["fit_points"] = std::to_string(fitPoints);
	std::ofstream file("settings.ini");
	auto printProfile = [&] (const std::string& name) {
		file << "[" << name << "]" << std::endl;
		for (std::pair<const std::string, std::string>& it : profiles[name]) {
			file << it.first << "=" << it.second << std::endl;
		}
	};
	printProfile(currentProfile);
	for (std::pair<const std::string, std::map<std::string, std::string>>& it : profiles) {
		if (it.first != currentProfile) printProfile(it.first);
	}
}

void Settings::readFormulaVal(formula<std::valarray<float>>& into,
		std::vector<std::string>& vars, std::string& source, char* ifAbsent) {
	vars.clear();
	vars.push_back("x");
	std::string readingVar;
	if (source.empty()) source = ifAbsent;
	for (unsigned int i = 0; i < source.size(); i++) {
		if (source[i] == '$') {
			i++;
			for ( ; (source[i] >= '0' && source[i] <= '9') || (source[i] >= 'a' && source[i] <= 'z')
					 || (source[i] >= 'A' && source[i] <= 'Z') || source[i] == '_'; i++) {
				readingVar.push_back(source[i]);
			}
			bool exists = false;
			for (unsigned int i = 0; i < vars.size(); i++)
				if (vars[i] == readingVar) exists = true;
			if (!exists) vars.push_back(readingVar);
			readingVar.clear();
		}
	}
	const char* sourceStr = source.c_str();
	into = formula<std::valarray<float>>::parseFormula(sourceStr, vars);
}
