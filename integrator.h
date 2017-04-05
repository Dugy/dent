#ifndef INTEGRATOR_H
#define INTEGRATOR_H
#include <string>

struct curve;
struct graph;

std::string integrate(curve& from, curve& result, graph* data);

#endif // INTEGRATOR_H