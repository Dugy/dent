#include "derivator.h"
#include "structs.h"
#include <iostream>
#include <cmath>
#include "settings.h"
#include "log.h"
#include <time.h>

std::string differentiate(curve& from, curve& result, graph* data) {
	//time_t start = time(NULL);
	//std::cerr << "Derivative " << &from << " point 0 time " << difftime(time(NULL), start) << std::endl;
    if (result.points) delete result.points;
    float pos = from.end;
	float step = (from.end - from.start) / from.length;
    int starting = 0;
    int ending = from.length;
	Settings& settings = Settings::get();
	float startPoint = settings.startDerivativeFrom;
	float endPoint = from.end - settings.endDerivativeBefore;

	//std::cerr << "Derivative " << &from << " point 1 time " << difftime(time(NULL), start) << std::endl;
	for (unsigned int i = from.length - 1;
		 ((step > 0) ? pos > endPoint : pos < endPoint); i--) {
		// Starting the beginning is bad because values change there a lot
		pos -= step;
		ending--;
	}
	//std::cerr << "Derivative " << &from << " point 1.5 time " << difftime(time(NULL), start) << std::endl;
    result.end = pos;
	pos = from.start;
	if (step > 0.00001) for (unsigned int i = 0; i < 3
							 || pos < startPoint; i++) {
		// Starting the beginning is bad because values change there a lot
		//std::cerr << i << " " << pos << " " << startPoint << " " << step << std::endl;
        pos += step;
		starting++;
	} else {
		Log::write("Warning: derivative of data where end < start, starting crop may fail\n");
	}

    int length = ending - starting; // Last points will be imprecise too
    if (length <= 0) return "No points after beginning of derivative set";
    result.length = length;
    result.start = pos;
    float points[length];

    float sum = 0;
    for (int i = 0; i < length; i++) {
        points[i] = (from.points[i + starting] - from.points[i + starting + 1])
                / ((pos - step) * (pos - step) - pos * pos);
        pos += step;
        sum += points[i] * points[i];
    }
	//std::cerr << "Derivative " << &from << " point 2 time " << difftime(time(NULL), start) << std::endl;

    // Correct values that are gone too far from average for some reasons
    // ruining the scale of graphs
	float average = sqrt(sum / length);
	float intoleranceThreshold = settings.intoleranceThreshold;
	for (int i = 0; i < length; i++) {
		if (points[i] > average * intoleranceThreshold
				|| points[i] < average * -1 * intoleranceThreshold) {
			if (i != 0) points[i] = points[i - 1];
			else if (points[1] > average * intoleranceThreshold
					 || points[1] < average * -1 * intoleranceThreshold) {
				points[0] = points[1];
			} else points[0] = 0;
		}
	}
	//std::cerr << "Derivative " << &from << " point 3 time " << difftime(time(NULL), start) << std::endl;

    // Smooth the data
	float lastCrack = -9999999;
	float nextPopin = (data->popins.empty()) ? 9999999 : data->popins.front();
	unsigned int nextPopinPos = 0;
	int smoothingWidth = fabsf(settings.derivativeSmoothingWidth
                               / (endPoint - startPoint)) * length;
    float mask[smoothingWidth * 2 + 1];
    float* maskCentre = &(mask[smoothingWidth]);
	float maxMaskSum = 0;
    for (int i = 0; i <= smoothingWidth; i++) {
		maskCentre[i] = pow(abs(smoothingWidth - i),
							settings.derivativeSmoothingSlope);
		maxMaskSum += maskCentre[i];
    }
    for (int i = -1 * smoothingWidth; i < 0; i++) {
        maskCentre[i] = maskCentre[-1 * i];
    }
	//std::cerr << "Derivative " << &from << " point 4 time " << difftime(time(NULL), start) << std::endl;
    result.points = new float[length];
	for (int i = 0; i < length; i++) {
		if (startPoint + i * step > nextPopin) {
			lastCrack = nextPopin;
			nextPopin = (++nextPopinPos >= data->popins.size()) ? 9999999 :
												data->popins[nextPopinPos];
		}
		float sum = 0;
		float maskSum = 0;
		for (int j = std::max(0, i - smoothingWidth);
			 j < std::min(length, i + smoothingWidth); j++) {
			float pos = startPoint + step * j;
			if (pos > lastCrack + settings.popinSkip
					&& pos < nextPopin - settings.popinSkip) {
				sum += points[j] * maskCentre[j - i];
				maskSum += maskCentre[j - i];
			}
		}
		if (maskSum > maxMaskSum / 10)
			result.points[i] = sum / maskSum;
		else
			result.points[i] = NAN;
	}
	//std::cerr << "Derivative " << &from << " point 5 time " << difftime(time(NULL), start) << std::endl;

    return ""; // Not failed
}
