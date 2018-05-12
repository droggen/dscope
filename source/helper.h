#ifndef __HELPER_H
#define __HELPER_H

#include <QString>
#include <QStringList>

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>


//-------------------------------------------------------------------------------------------------
// Data structures
//-------------------------------------------------------------------------------------------------
typedef struct {
   std::string title;
   std::vector<unsigned> traces;
   std::vector<unsigned> colors;
   int xscale;
   std::vector<int> yscale;
   bool yauto;
} ScopeDefinition;

typedef struct {
   //unsigned numtraces;
   std::vector <ScopeDefinition> scopedefinition;
} ScopesDefinition;


//-------------------------------------------------------------------------------------------------
// SplitInVector
//-------------------------------------------------------------------------------------------------
// Returns false in case of failure.
// Only one split character is allowed
template<class T> bool SplitInVector(std::string str,std::string split,std::vector<T> &r,std::ios_base&(&b)(std::ios_base&)=std::dec)
{

   r.clear();
   QString qstr(str.c_str());
   QStringList sl = qstr.split(QString(split.c_str()),QString::SkipEmptyParts);
   for(int i=0;i<sl.size();i++)
   {
      std::istringstream iss(sl[i].toStdString());
      T j;
      if( !((iss >> b >> j).fail()))
         r.push_back(j);
      else
         return false;
   }
   return true;
}

int ParseLayoutString(std::string str,ScopesDefinition &scopesdefs);



#endif // __HELPER_H
