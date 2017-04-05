#include "parser.h"
#include "structs.h"
#include "settings.h"
#include "log.h"
#include "utils.h"
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <list>
#include <cmath>

std::string parseFile(graph& result) {
	logStream report;
	report << "Opening file " << result.name << std::endl;
	std::string fileName = result.name.substr(result.name.find_last_of("/\\") + 1);
	std::ifstream file(result.name);
	if (!file.is_open()) return "Could not open file";
	std::vector<float> parsed1;
	std::vector<float> parsed2;
	std::string line;
	float max1 = -99999;
	float max2 = -99999;
	while (std::getline(file, line))
	{
		std::istringstream iss(line);
		if ((line[0] >= '0' && line[0] <= '9')
				|| line[0] == '-') {
			// Line is a valid data point
			float first, second;
			iss >> first >> second;
			if (first > max1) max1 = first;
			if (second > max2) max2 = second;
			parsed1.push_back(first);
			parsed2.push_back(second);
		} else if (line.find(')') != std::string::npos) {
			int pos = line.find(')');
			result.axisX = ISO8819toUTF8(line.substr(0, pos + 1));
			line = line.substr(pos + 2);
			result.axisY = ISO8819toUTF8(line.substr(0, line.find(')') + 1));
			std::cerr << "Axis y name " << result.axisY << std::endl;
		}
	}
	file.close();
	if (parsed2.empty()) return "Could not find data in file";

	// Find popins
	Settings& settings = Settings::get();
	std::list<float> recent1;
	std::list<float> recent2;
	std::vector<unsigned int> popins;
	for (unsigned int i = 0; i < parsed1.size(); i++) {
		recent1.push_back(parsed1[i]);
		recent2.push_back(parsed2[i]);
		if (fabsf(recent2.front() - parsed2[i]) > settings.filterYMaximum) {
			recent1.pop_front();
			recent2.pop_front();
		}
		//std::cerr << "Difference is " << fabsf(recent1.front() - parsed1[i]) << " width " << fabsf(recent2.front() - parsed2[i]) << " size " << recent2.size() << std::endl;
		if (fabsf(recent1.front() - parsed1[i]) > settings.filterXMaximum) {
			popins.push_back(i);
			recent1.clear();
			recent2.clear();
		}
	}

	for (char doSmooth = false; doSmooth <= true; doSmooth++) {
		// Smooth the data, but respect a maximum for smoothing -
		// beyond the maximum, it's a popin
		unsigned int nextPopin = 0;
		if (doSmooth && settings.filterEnabled) {
			auto filter = [&] (std::vector<float>& parsed, unsigned int width,
					bool max, float slope) -> void {
				// Precompute a mask
				float mask[width * 2 + 1];
				float* maskCentre = &(mask[width]);
				for (unsigned int i = 0; i <= width; i++) {
					maskCentre[i] = pow(width - i, slope);
				}
				for (int i = -1 * width; i < 0; i++) {
					maskCentre[i] = maskCentre[-1 * i];
				}

				std::vector<float> output;
				output.reserve(parsed.size());
				// Watch changes too large for smoothing,
				// as they may have physical bases
				int leapMin = -1;
				int leapMax = -1;
				int maxLeapCheck = -1;
				for (unsigned int i = 0; i < parsed.size(); i++) {
					float factor = 0;
					float total = 0;
					for (unsigned int j = std::max(i - width,
										(unsigned int)0); j < std::min<unsigned int>(
							 i + width + 1, parsed.size()); j++) {
						if (leapMin < (int)i && (int)j < leapMin) j = leapMin;
						if (leapMax > (int)i && (int)j > leapMax) break;
						if ((int)j > maxLeapCheck && j - 1 < parsed.size()) {
							if (max && nextPopin < popins.size()) {
								if (parsed[j] > parsed[popins[nextPopin]]) {
									leapMin = leapMax;
									leapMax = j;
									nextPopin++;
								}
							}
							maxLeapCheck = j;
						}
						// Actually add the values
						total += maskCentre[(int)i - (int)j] * parsed[j];
						factor += maskCentre[(int)i - (int)j];
					}
					if (factor != 0) output.push_back(total / factor);
					else if (i > 0) output.push_back(output.back());
					else output.push_back(parsed[i]); // Something failed
					//std::cout << "Filtered " << output[i] << " to " << parsed[i] << std::endl;
				}
				parsed.swap(output);
			};
			filter(parsed1, settings.filterYWidth,
				   true, settings.filterXSlope);
			filter(parsed2, settings.filterXWidth,
				   false, settings.filterYSlope);
		}

		int changeSampling = settings.changeSampling;
		int changeMinimum = changeSampling * 0.75;
		float increaseNeeded = max2 / parsed2.size() * settings.increaseThreshold;
		float decreaseNeeded = max2 / parsed2.size() * settings.decreaseThreshold;

		int beginning = -1;
		int middle = -1;
		int reverse = -1;
		int end = -1;

		//std::cerr << "Finding stuff in file\n";
		// The beginning is where the increasing starts
		int changes = 0;
		std::list<bool> change;
		for (unsigned int i = 1; i < parsed2.size(); i++) {
			if (parsed2[i] > parsed2[i - 1] + increaseNeeded) {
				change.push_back(true);
				changes++;
			}
			else change.push_back(false);

			if ((int)change.size() > changeSampling) {
				if (change.front()) changes--;
				change.pop_front();
				if (changes > changeMinimum) {
					beginning = i - changeMinimum;
					break;
				}
			}
		}
		if (beginning == -1)
			return "Could not find the beginning of the curve";

		// Adjust the origin
		float start1 = parsed1[beginning];
		float start2 = parsed2[beginning];
		for (unsigned int i = 0; i < parsed2.size(); i++) {
			parsed1[i] -= start1;
			parsed2[i] -= start2;
		}

		for (unsigned int i = 0; i < popins.size(); i++) {
			if ((int)popins[i] < beginning) continue;
			result.popins.push_back(parsed2[popins[i]]);
			report << fileName << ": found a pop in at "
				   << parsed2[popins[i]] << std::endl;
		}

		//std::cerr << "Beginning found at " << beginning << std::endl;
		// Middle is when less than changeMinimum points
		// out of changeSampling change
		changes = 0;
		change.clear();
		for (unsigned int i = beginning + 1; i < parsed2.size(); i++) {
			if (parsed2[i] > parsed2[i - 1] + increaseNeeded) {
				change.push_back(true);
				changes++;
			}
			else change.push_back(false);

			if ((int)change.size() > changeSampling) {
				if (change.front()) changes--;
				change.pop_front();
				if (changes < changeMinimum) {
					middle = i;
					break;
				}
			}
		}
		if (middle < beginning)
			return "Could not determine the end of the increasing part";

		//std::cerr << "Middle found at " << middle << std::endl;
		// Now find when it starts dropping
		changes = 0;
		change.clear();
		for (unsigned int i = middle + 1; i < parsed2.size(); i++) {
			if (parsed2[i] < parsed2[i - 1] - decreaseNeeded) {
				change.push_back(true);
				changes++;
			}
			else change.push_back(false);

			if ((int)change.size() > changeSampling) {
				if (change.front()) changes--;
				change.pop_front();
				if (changes > changeMinimum) {
					reverse = i - changeMinimum;
					break;
				}
			}
		}
		// If reverse < middle, let it be
		if (reverse == -1)
			return "Could not find the beginning of the decreasing part";

		//std::cerr << "Reverse found at " << reverse << std::endl;
		// Now, find where it drops below zero
		for (unsigned int i = reverse; i < parsed2.size(); i++) {
			if (parsed2[i] < 0.001) {
				end = i;
				break;
			}
		}
		if (end < reverse)
			end = parsed2.size() - 1; // The end is cut

		//std::cerr << "Curve " << result.name << " beginning " << beginning << " " << parsed1[beginning] << " middle " << middle << " " << parsed1[middle] << " reverse " << reverse << " " << parsed1[reverse] << " end " << end << " " << parsed1[end] << " size " << parsed1.size() << std::endl;
		//std::cerr << "End found at " << end << std::endl;
		auto fillCurve = [&] (curve& data, int front, int back) {
			data.start = parsed1[front];
			data.end = parsed1[back];
			float step = (data.end - data.start) / settings.points;
			data.length = settings.points;
			data.points = new float[settings.points];
			float pos = data.start;
			unsigned int index = front;
			if (index == 0) index = 1; // Avoid overflow
			for (unsigned int i = 0; i < settings.points; i++) {
				float value;
				if (step > 0) {
					while (parsed1[index] < pos && index < parsed1.size())
						index++;
					value = parsed2[index - 1] + (parsed2[index - 1]
							- parsed2[index]) * (pos - parsed1[index - 1]);
				} else {
					while (parsed1[index] > pos && index < parsed1.size())
						index++;
					value = parsed2[index - 1] + (parsed2[index]
							- parsed2[index - 1]) * (pos - parsed1[index - 1]);
				}
				data.points[i] = value;
				//std::cout << "Inverted point " << pos << " at " << value << std::endl;
				pos += step;
			}
		};
		fillCurve(doSmooth ? result.base[DOWN] : result.raw[DOWN], beginning, middle);
		fillCurve(doSmooth ? result.base[UP] : result.raw[UP], reverse, end);
	}
	return "";
}
