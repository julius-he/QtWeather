#ifndef CITYCODEPARSER_H
#define CITYCODEPARSER_H

#include <QMap>
#include <QString>

class CityCodeParser
{
public:
    static QString getCityCode(QString cityName);

private:
    static QMap<QString, QString> mCityMap;

    static void initCityMap();
};

#endif // CITYCODEPARSER_H
