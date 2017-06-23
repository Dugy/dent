#include "structs.h"

std::vector<float> graph::getVariables() {
    std::vector<float> vars;
    for (int i = 0; i < GRABBED_VARIABLES; i++)
        vars.push_back(grabbedParametres[i]);
    for (int i = 0; i < 2; i++)
        for (unsigned int j = 0; j < fittedVars[i].size(); j++)
            vars.push_back(fittedVars[i][j]);
    return vars;
}
