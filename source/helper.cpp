#include "helper.h"
#include <QRegExp>

//-------------------------------------------------------------------------------------------------
// ParseLayoutString
// Layout string: n_scopes<<title>;<traces>[;xscale=<xs>][;yscale=<auto|min;max>][color=<colors>]>
//-------------------------------------------------------------------------------------------------
// Example: <Scope 2;4;yscale=-100 100>
int ParseLayoutString(std::string str,ScopesDefinition &scopesdefs)
{
   scopesdefs.scopedefinition.clear();

   // Regexp for layout string:
   // Characters ;<> are excluded in the match of the expression for syntax checking.
   // Group 0: whole expression
   // Group 1: title
   // Group 2: traces
   // Group 4: xscale
   // Group 6: yscale
   // Group 8: color
   // Validation of the whole string: look for multiple expressions with a '(expression)*' regex string.
   // Original string:
   //string expression = string("\\s*<\\s*([^;^>^<]*?)\\s*;\\s*([^;^>^<]+?)\\s*(;\\s*xscale\\s*=\\s*([^;^>^<]*?))?(;\\s*yscale\\s*=\\s*([^;^>^<]*?))?(;\\s*color\\s*=\\s*([^;^>^<]*?))?>\\s*");

   // Beware QRegExp does not support non greedy match *? and +? (anyway isn't needed)
   std::string expression = std::string("\\s*<\\s*([^;^>^<]*)\\s*;\\s*([^;^>^<]+)\\s*(;\\s*xscale\\s*=\\s*([^;^>^<]*))?(;\\s*yscale\\s*=\\s*([^;^>^<]*))?(;\\s*color\\s*=\\s*([^;^>^<]*))?>\\s*");
   QRegExp e(QString(expression.c_str()));
   //printf("e.isValid: %d\n",(int)e.isValid());
   //printf("e.isValid: %s\n",e.errorString().toStdString().c_str());
   std::string expression2=std::string("(")+expression+std::string(")*");
   QRegExp validation(QString(expression2.c_str()));
   //printf("validation.isValid: %d\n",(int)validation.isValid());
   //printf("validation.isValid: %s\n",e.errorString().toStdString().c_str());


   // Validation
   int iin=validation.exactMatch(QString(str.c_str()));
   if(iin==-1)
   {
      printf("Validation match failed\n");
      return -5;
   }


   // Expression parsing
   e.indexIn(QString(str.c_str()));
   unsigned midx=0;
   int pos = 0;
   while((pos = e.indexIn(QString(str.c_str()), pos)) != -1)
   {
      pos += e.matchedLength();

      /*printf("Capture group %d\n",midx);
      printf("NumCaptures: %d\n",e.numCaptures());
      for(int i=0;i<e.numCaptures()+1;i++)
         printf("Capture %d: %s\n",i,e.cap(i).toStdString().c_str());*/


      ScopeDefinition sd;
      if(e.captureCount()!=8)
      {
         std::cout << "Invalid match size\n";
         return -9;
      }

      sd.title = e.cap(1).toStdString();
      std::string traces = e.cap(2).toStdString();
      std::string xscale = e.cap(4).toStdString();
      std::string yscale = e.cap(6).toStdString();
      std::string color = e.cap(8).toStdString();

      std::vector<unsigned> d;
      std::vector<int> dxscale,dyscale;
      std::vector<std::string> dstring;

      if(!SplitInVector(traces,std::string(" "),sd.traces))
         return -20;
      std::cout << "Traces: ";
      for(unsigned j=0;j<sd.traces.size();j++)
         std::cout << sd.traces[j] << "; ";
      std::cout << "\n";

      // Parse the X scale.
      if(!SplitInVector(xscale,std::string(" "),dxscale))
         return -21;
      if(dxscale.size()>1)
         return -22;
      if(dxscale.size()==0)
         sd.xscale=1;
      else
         sd.xscale=dxscale[0];
      std::cout << "xscale: " << sd.xscale << "\n";


      // Parse the Y scale.
      if(!SplitInVector(yscale,std::string(" "),dstring))
         return -23;
      if(dstring.size()>2)
         return -24;
      switch(dstring.size())
      {
         case 0:
            sd.yauto=true;
            break;
         case 1:
            if(dstring[0].compare("auto")==0)
               sd.yauto=true;
            else
               return -25;
            break;
         case 2:
            sd.yauto=false;
            if(!SplitInVector(yscale,std::string(" "),sd.yscale))
               return -26;
            if(sd.yscale.size()!=2)
               return -27;
            if(sd.yscale[0]>sd.yscale[1])
            {
               int t=sd.yscale[0];
               sd.yscale[0]=sd.yscale[1];
               sd.yscale[1]=t;
            }
            if(sd.yscale[0]==sd.yscale[1])
               sd.yscale[0]--;
            break;
      }
      std::cout << "yscale: ";
      if(sd.yauto==true)
         std::cout << "auto\n";
      else
         std::cout << "[" << sd.yscale[0] << ";" << sd.yscale[1] << "]\n";

      // Parse the colors
      if(!SplitInVector(color,std::string(" "),sd.colors,std::hex))
         return -28;
      std::cout << "color: ";
      for(unsigned j=0;j<sd.colors.size();j++)
         std::cout << sd.colors[j] << "; ";
      std::cout << "\n";

      if(sd.colors.size()!=0 && (sd.colors.size() != sd.traces.size()))
         return -29;

      std::cout << "\n";
      scopesdefs.scopedefinition.push_back(sd);

      midx++;
   }



   return 0;
}

