#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <map>
#include <valarray>
#include <vector>
#include "formula.h"

struct Settings
{
    static Settings& get()
    {
        static Settings instance;
        return instance;
    }

	std::string folder;
    unsigned int cores;
    bool customZoom;
    float xMinCustom;
    float xMaxCustom;
    float yMinCustom;
    float yMaxCustom;
	bool showAscending;
	bool showDescending;
    unsigned int filterXWidth;
    float filterXMaximum;
    float filterXSlope;
    unsigned int filterYWidth;
    float filterYMaximum;
    float filterYSlope;
    unsigned int changeSampling;
    float increaseThreshold;
    float decreaseThreshold;
    bool filterEnabled;
    unsigned int points;
    bool proceedFromLoadToDerivatives;
    float startDerivativeFrom;
    float endDerivativeBefore;
    float derivativeSmoothingWidth;
    float derivativeSmoothingSlope;
    float intoleranceThreshold;
	float popinSkip;
	bool proceedFromDerivativesToIntegrals;
	bool integrateSmoothed;
	std::vector<std::string> downVariables;
	std::vector<std::string> upVariables;
	formula<std::valarray<float>> downFormula;
	formula<std::valarray<float>> upFormula;
	unsigned int fitGuessSteps;
	unsigned int fitGuessPoints;
	unsigned int fitPoints;

	std::map<std::string, std::map<std::string, std::string>> profiles;
	void loadProfile(const std::string& name);
	std::string currentProfile;
	void setup();
	void synchChanges();

	static void readFormulaVal(formula<std::valarray<float>>& into,
			std::vector<std::string>& vars, std::string& source, char* ifAbsent);

private:
    Settings();
    ~Settings();

    Settings(Settings const&) = delete;
    void operator=(Settings const&) = delete;
};

#endif // SETTINGS_H
