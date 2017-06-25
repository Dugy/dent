#include "integrator.h"
#include "structs.h"
#include <iostream>
#include <cmath>
#include "settings.h"

std::string integrate(curve& from, curve& result, graph* data) {
	if (result.points) delete result.points;
	result.start = from.start;
	result.end = from.end;
	float step = (from.end - from.start) / from.length;
	//Settings& settings = Settings::get();
	result.points = new float[from.length];
	result.length = from.length;
	float sum = 0;
	if (step >= 0) {
		for (unsigned int i = 0; i < from.length; i++) {
			sum += from.points[i] * step;
			result.points[i] = sum;
		}
	} else {
		for (unsigned int i = 0; i < from.length; i++) {
			sum -= from.points[i] * step;
			result.points[i] = sum;
		}
	}
	return ""; // Not failed
}
