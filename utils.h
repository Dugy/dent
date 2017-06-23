#ifndef UTILS
#define UTILS
#include <QString>
#include <string>
#include <cstdlib>
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

#endif // UTILS

