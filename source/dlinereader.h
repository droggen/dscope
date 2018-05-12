#ifndef DLINEREADER_H
#define DLINEREADER_H

#include <string>

class DLineReader
{

public:
	DLineReader();
	~DLineReader();

	void add(const std::string &str);
	bool getLine(std::string &str);
	int size();

private:
	std::string buffer;
};

#endif // DLINEREADER_H
