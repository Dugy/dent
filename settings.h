#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <map>
#include <valarray>
#include <vector>
#include "formula.h"

#define USER_FORMULAS 6

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
	std::vector<std::string> areaVariables;
	formula<std::valarray<float>> downFormula;
	formula<std::valarray<float>> upFormula;
	formula<float> areaFormula;
    formula<float> userFormula[USER_FORMULAS];
	unsigned int fitGuessSteps;
	unsigned int fitGuessPoints;
	unsigned int fitPoints;

	std::map<std::string, std::map<std::string, std::string>> profiles;
	void loadProfile(const std::string& name);
	std::string currentProfile;
	void setup();
	void synchChanges();

    static std::vector<std::string> generateVarNames(const Settings &settings);
    static void readStatFormula(const Settings &settings, formula<float>& into, std::__cxx11::string source, char* ifAbsent);

    static float A(float var);

	template<typename scalar = FORMULA_FLOATING_TYPE>
	static void readFormulaVal(formula<scalar>& into,
							   std::vector<std::string>& vars, std::string& source, char* ifAbsent) {
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
		into = formula<scalar>::parseFormula(sourceStr, vars);
	}

private:
    Settings();
    ~Settings();

    Settings(Settings const&) = delete;
    void operator=(Settings const&) = delete;
};

#endif // SETTINGS_H
