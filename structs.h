#ifndef STRUCTS
#define STRUCTS

#include <string>
#include <iostream>
#include <vector>

#define GRABBED_VARIABLES 7

enum curveDirection {
    UP,
    DOWN
};

struct curve {
    float start;
    float end;
    float* points;
    unsigned int length;
    curve() : points(nullptr), length(0) {}
    ~curve() {
        if (points) delete points;
    }
    void debugPrint() {
        if (!points) {
            std::cout << "No points" << std::endl;
            return;
        }
        float addition = (end - start) / length;
        float pos = start;
        std::cout << length << " points " << std::endl;
        for (unsigned int i = 0; i < length; i++) {
            std::cout << pos << " " << points[i] << std::endl;
            pos += addition;
        }
    }
};

struct graph {
    curve raw[2];
    curve base[2];
    curve derivative[2];
	curve integral[2];
	curve fitted[2];
    std::string name;
	std::string axisX;
	std::string axisY;
    bool visible;
    std::vector<float> popins;
	std::vector<float> fittedVars[2];
	union {
		struct {
			float Lmax; // max force applied
			float hmax; // max penetration depth
			float hr; // sink-in as if the decreasing part was linear
			float hp; // sink-in
			float Amax; // max penetrated area
			float Wirr; // inelastic work
			float We; // elastic work
		};
        float grabbedParametres[GRABBED_VARIABLES];
	};
    std::vector<float> getVariables();
};

#endif // STRUCTS

