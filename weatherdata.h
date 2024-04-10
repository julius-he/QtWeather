#ifndef WEATHERDATA_H
#define WEATHERDATA_H

#include <QString>

class Today
{
public:
    QString date;
    QString city;
    QString notice;

    QString wendu;
    QString shidu;
    int pm25;
    QString quality;
    QString type;
    QString fx;
    QString fl;
    int high;
    int low;

    Today();
};

class Day
{
public:
    QString date;
    QString week;
    QString type;
    QString fx;
    QString fl;
    QString notice;
    int high;
    int low;
    int aqi;

    Day();
};


#endif // WEATHERDATA_H
