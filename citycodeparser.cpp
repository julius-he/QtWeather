#include "citycodeparser.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

QMap<QString,QString> CityCodeParser::mCityMap = {};

QString CityCodeParser::getCityCode(QString cityName)
{
    if (mCityMap.isEmpty())
    {
        initCityMap();
    }
    auto it = mCityMap.find(cityName);
    if (it == mCityMap.end())
    {
        if (!cityName.contains("市"))
        {
            cityName += "市";
        }
        else
        {
            cityName = cityName.split("市").first();
        }
        it = mCityMap.find(cityName);
    }
    if (it != mCityMap.end())
    {
        return it.value();
    }
    return "";
}

void CityCodeParser::initCityMap()
{
    QFile file(":/res/citycode.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray json = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        qDebug() << "解析json失败，错误：" << parseError.errorString();
        return;
    }

    if (!doc.isArray())
    {
        return;
    }

    QJsonArray cities = doc.array();
    for (const auto& city : cities) {
        QString cityName = city.toObject().value("city_name").toString();
        QString cityCode = city.toObject().value("city_code").toString();
        if (!cityCode.isEmpty())
        {
            mCityMap.insert(cityName, cityCode);
        }
    }
}
