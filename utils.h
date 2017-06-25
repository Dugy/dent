#ifndef UTILS
#define UTILS
#include <QString>
#include <string>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <sstream>

inline std::string to_string(const float num) {
	std::stringstream strm;
	strm << num;
	return strm.str();
}

inline int qStringToInt(QString str) {
    return atoi(str.toUtf8().constData());
}

inline float qStringToFloat(QString str) {
	return str.toFloat();
}

inline QString intToQString(int num) {
	return QString(to_string(num).c_str());
}


inline QString floatToQString(float num) {
    if (num > 10 || num < -10 || num == 0 ||
            ((int)num < num * 1.001 && (int)num > num * 0.999))
		return QString(to_string((int)num).c_str());

    // Nope, it isn't a simple case
	QString result = to_string((int)num).c_str();
    if (num < 0) num *= -1;
    result.push_back('.');
    if (num >= 1) {
        result.push_back((char)((num - (int)num) * 10) + '0');
    } else {
        num -= (int)num;
        while (true) {
            num *= 10;
            if (num >= 1) {
                result.push_back((int)num + '0');
                num -= (int)num;
                result.push_back((int)(num * 10) + '0');
                return result;
            }
            result.push_back('0');
        }
    }
    return result;
}

inline std::string ISO8819toUTF8(const std::string source) {
	std::string result;
	unsigned char sourceText[source.size() + 1];
	memcpy(sourceText, source.c_str(), source.size() + 1);
	unsigned char* in = sourceText;
	while (*in) {
		if (*in < 128) result.push_back(*in++);
		else {
			result.push_back(0xc2 + (*in > 0xbf));
			result.push_back((*in++ &0x3f)+0x80);
		}
	}
	return result;
}

inline float stor(const char* str) {
	// The problem with std::stof and atof is that it's locale dependent and unpredictably
	// accepts decimal dot or decimal comma and rejects the other, this one accepts both,
	// the number parser in formula accepts both too
	float result = 0;
	float sign = *str == '-' ? str++, -1 : 1;
	while (*str >= '0' && *str <= '9') {
		result *= 10;
		result += *str - '0';
		str++;
	}
	if (*str == ',' || *str == '.') {
		str++;
		float multiplier = 0.1;
		while (*str >= '0' && *str <= '9') {
			result += (*str - '0') * multiplier;
			multiplier /= 10;
			str++;
		}
	}
	result *= sign;
	if (*str == 'e' || *str == 'E') {
		str++;
		float powerer = *str == '-'? str++, 0.1 : 10;
		float power = 0;
		while (*str >= '0' && *str <= '9') {
			power *= 10;
			power += *str - '0';
			str++;
		}
		result *= pow(powerer, power);
	}
	return result;
}

inline float stor(const std::string& str) {
	return stor(str.c_str());
}

inline float stor(const QString& str) {
	return stor(str.toUtf8().constData());
}

#endif // UTILS

