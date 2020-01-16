#include "dlinereader.h"
#include <algorithm>

DLineReader::DLineReader()
{
	buffer.clear();
}

DLineReader::~DLineReader()
{

}

void DLineReader::add(const std::string& str)
{
	buffer.append(str);
}
int DLineReader::size()
{
	return buffer.size();
}

bool DLineReader::getLine(std::string &line)
{
	int p1,p2,p;

	//printf("<%s>\n",buffer.c_str());

	while(1)
	{
		// Search for the first newline characters (10 or 13)
		p1 = buffer.find(10);
		p2 = buffer.find(13);



		// Return if no newline character found
		if(p1==std::string::npos && p2==std::string::npos)
			return false;

		// First newline delimiter
		if(p1==std::string::npos || p2==std::string::npos)
		{
			//printf("max\n");
			p = std::max(p1,p2);
		}
		else
		{
			//printf("min\n");
			p=std::min(p1,p2);
		}

		//printf("%d %d %d\n",p1,p2,p);

		// If position is zero for first delimiter remove character and search for the next entry
		if(p==0)
		{
			buffer=buffer.erase(p,1);
			continue;
		}

		// Position is nonzero: return the string

		line = buffer.substr(0,p);
		buffer.erase(0,p);
		return true;
	}


}
