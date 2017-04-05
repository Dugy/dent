#ifndef DERIVATOR
#define DERIVATOR
#include <string>

struct curve;
struct graph;

std::string differentiate(curve& from, curve& result, graph* data);

#endif // DERIVATOR

