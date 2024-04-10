#include "weatherdata.h"

Today::Today()
{
    date = "2024-01-01 星期一";
    city = "广州";
    notice = "感冒指数";
    wendu = "0";
    shidu = "0%";
    pm25 = 0;
    quality = "无数据";
    type = "多云";
    fx = "南风";
    fl = "2级";
    high = 30;
    low = 18;
}

Day::Day()
{
    date = "2024-01-01";
    week = "星期一";
    type = "多云";
    notice = "感冒指数";
    fx = "南风";
    fl = "2级";
    high = 30;
    low = 18;
    aqi = 0;
}
