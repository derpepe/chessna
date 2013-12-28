#ifndef LIB_H
#define LIB_H

#include <string>
#include <sstream>
#include <vector>

class Lib
{
public:
	static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
	static std::vector<std::string> split(const std::string &s, char delim);
};

#endif