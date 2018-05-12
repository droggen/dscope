#include <dterminal.h>

DTerminal::DTerminal(unsigned maxlines)
{
    DTerminal::maxlines=maxlines;
}

DTerminal::~DTerminal()
{

}

void DTerminal::addBytes(const QByteArray &ba)
{
    // Add the data to the line reader
    linereader.add(std::string(ba.constData()));
    // Read out the lines from the line reader and put them in the list of strings structure
    while(1)
    {
        std::string str;
        bool rv = linereader.getLine(str);
        if(rv==false)
            break;
        QString qstr(str.c_str());
        buffer.append(qstr);
    }
    trimOld();
}

QString DTerminal::getText()
{
    // Assemble the strings together
    QString str="";


    for(auto i=buffer.begin();i!=buffer.end();i++)
    {
        str+=*i+"\n";
    }
    return str;
}

void DTerminal::trimOld()
{
    // Remove the lines beyond maxlines

    if(buffer.size()>maxlines)
    {
        for(int j=0;j<buffer.size()-maxlines;j++)
        {
            buffer.removeFirst();
        }
    }

}



