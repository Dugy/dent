#ifndef FITTER_H
#define FITTER_H

#include <string>
#include "structs.h"

struct curve;
struct graph;

std::string fit(curve& from, curve& result, graph* data, curveDirection dir);

#endif // FITTER_H