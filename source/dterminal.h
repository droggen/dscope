#ifndef DTERMINAL_H
#define DTERMINAL_H

#include <QList>
#include <QString>
#include "dlinereader.h"

class DTerminal
{
public:
    DTerminal(unsigned maxlines);
    ~DTerminal();

    void addBytes(const QByteArray &ba);
    QString getText();

    void clear();

private:
    unsigned maxlines;
    DLineReader linereader;
    QList<QString> buffer;

    void trimOld();

};

#endif // DTERMINAL_H
