#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QPainter>

#include <citycodeparser.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 设置无边框窗口
    setWindowFlag(Qt::FramelessWindowHint);
    setFixedSize(800, 450);
    // 设置右键菜单
    mExitMenu = new QMenu(this);
    mExitAction = new QAction();
    mExitAction->setText("退出");
    mExitAction->setIcon(QIcon(":/res/close.png"));
    mExitMenu->addAction(mExitAction);
    connect(mExitAction, &QAction::triggered, this, [=]() {
        qApp->exit();
    });
    setupContainer();

    // 网络请求
    mNetworkManager = new QNetworkAccessManager(this);
    connect(mNetworkManager, &QNetworkAccessManager::finished, this, &MainWindow::onReplied);

    getWeatherInfo("北京");

    ui->lblHighCurve->installEventFilter(this);
    ui->lblLowCurve->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    mExitMenu->exec(QCursor::pos());
    event->accept();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    mOffset = event->globalPosition().toPoint() - this->pos();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPosition().toPoint() - mOffset);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->lblHighCurve && event->type() == QEvent::Paint)
    {
        paintHighCurve();
    }
    if (watched == ui->lblLowCurve && event->type() == QEvent::Paint)
    {
        paintLowCurve();
    }
    return QWidget::eventFilter(watched, event);
}

void MainWindow::getWeatherInfo(QString cityName)
{
    QString cityCode = CityCodeParser::getCityCode(cityName);
    if (cityCode.isEmpty())
    {
        QMessageBox::warning(this, "警告", "无法查询到相关的天气信息，请检查城市名是否正确");
        return;
    }
    QUrl url("http://t.weather.itboy.net/api/weather/city/" + cityCode);
    mNetworkManager->get(QNetworkRequest(url));
}

void MainWindow::parseJson(QByteArray &byteArray)
{
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(byteArray, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        return;
    }

    QJsonObject jsonObject = document.object();
    mToday.date = jsonObject.value("date").toString();
    mToday.city = jsonObject.value("cityInfo").toObject().value("city").toString();

    QJsonObject dataObject = jsonObject.value("data").toObject();
    mToday.shidu = dataObject.value("shidu").toString();
    mToday.pm25 = dataObject.value("pm25").toInt();
    mToday.quality = dataObject.value("quality").toString();
    mToday.wendu = dataObject.value("wendu").toString();

    Day yesterday;
    QJsonObject yesterdayObj = dataObject.value("yesterday").toObject();
    yesterday.date = yesterdayObj.value("ymd").toString();
    QString highStr = yesterdayObj.value("high").toString().split(" ").at(1);
    highStr = highStr.left(highStr.length() - 1);
    yesterday.high = highStr.toInt();
    QString lowStr = yesterdayObj.value("low").toString().split(" ").at(1);
    lowStr = lowStr.left(lowStr.length() - 1);
    yesterday.low = lowStr.toInt();
    yesterday.week = yesterdayObj.value("week").toString();
    yesterday.aqi = yesterdayObj.value("aqi").toDouble();
    yesterday.fx = yesterdayObj.value("fx").toString();
    yesterday.fl = yesterdayObj.value("fl").toString();
    yesterday.type = yesterdayObj.value("type").toString();
    yesterday.notice = yesterdayObj.value("notice").toString();
    mDays[0] = yesterday;

    QJsonArray forecastArr = dataObject.value("forecast").toArray();
    for (int i = 0; i < mDays.size() - 1; ++i)
    {
        QJsonObject obj = forecastArr[i].toObject();
        Day day;
        day.date = obj.value("ymd").toString();
        QString highStr = obj.value("high").toString().split(" ").at(1);
        highStr = highStr.left(highStr.length() - 1);
        day.high = highStr.toInt();
        QString lowStr = obj.value("low").toString().split(" ").at(1);
        lowStr = lowStr.left(lowStr.length() - 1);
        day.low = lowStr.toInt();
        day.week = obj.value("week").toString();
        day.aqi = obj.value("aqi").toDouble();
        day.fx = obj.value("fx").toString();
        day.fl = obj.value("fl").toString();
        day.type = obj.value("type").toString();
        day.notice = yesterdayObj.value("notice").toString();
        mDays[i + 1] = day;
    }

    mToday.type = mDays[1].type;
    mToday.fx = mDays[1].fx;
    mToday.fl = mDays[1].fl;
    mToday.high = mDays[1].high;
    mToday.low = mDays[1].low;
    mToday.notice = mDays[1].notice;

    updateUI();
}

void MainWindow::updateUI()
{
    //更新日期和城市
    ui->lblDate->setText(QDateTime::fromString(mToday.date,"yyyyMMdd").toString("yyyy/MM/dd")
                         + " " + mDays[1].week);
    ui->lblCity->setText(mToday.city);

    //更新今天
    ui->lblTypeIcon->setPixmap(mTypeMap[mToday.type]);
    ui->lblTemp->setText(QString(mToday.wendu) + "°");
    ui->lblType->setText(mToday.type);
    ui->lblLowHigh->setText(QString::number(mToday.low) + "~"
                            + QString::number(mToday.high) + "°C");
    ui->lblGanMao->setText(mToday.notice);
    ui->lblWindFx->setText(mToday.fx);
    ui->lblWindFl->setText(mToday.fl);
    ui->lblPM25->setText(QString::number(mToday.pm25));
    ui->lblShiDu->setText(mToday.shidu);
    ui->lblQuality->setText(mToday.quality);

    //更新六天的数据
    for (int i = 0; i < mDays.size(); i++)
    {
        //更新日期和时间
        mWeekList[i]->setText("星期" + mDays[i].week.right(1));
        ui->lblWeek0->setText("昨天");
        ui->lblWeek1->setText("今天");
        ui->lblWeek2->setText("明天");
        QStringList ymdList = mDays[i].date.split("-");
        mDateList[i]->setText(ymdList[1] + "/" + ymdList[2]);

        //更新天气类型
        mTypeList[i]->setText(mDays[i].type);
        mTypeIconList[i]->setPixmap(mTypeMap[mDays[i].type]);

        //更新空气质量
        if (mDays[i].aqi >0 && mDays[i].aqi <= 50)
        {
            mAqiList[i]->setText("优");
            mAqiList[i]->setStyleSheet("background-color: rgb(139,195,74);");
        }
        else if (mDays[i].aqi > 50 && mDays[i].aqi <= 100)
        {
            mAqiList[i]->setText("良");
            mAqiList[i]->setStyleSheet("background-color: rgb(255,170,0);");
        }
        else if (mDays[i].aqi > 100 && mDays[i].aqi <= 150)
        {
            mAqiList[i]->setText("轻度");
            mAqiList[i]->setStyleSheet("background-color: rgb(255,87,97);");
        }
        else if (mDays[i].aqi > 150 && mDays[i].aqi <= 200)
        {
            mAqiList[i]->setText("中度");
            mAqiList[i]->setStyleSheet("background-color: rgb(255,17,27);");
        }
        else if (mDays[i].aqi > 150 && mDays[i].aqi <= 200)
        {
            mAqiList[i]->setText("重度");
            mAqiList[i]->setStyleSheet("background-color: rgb(170,0,0);");
        }
        else
        {
            mAqiList[i]->setText("严重");
            mAqiList[i]->setStyleSheet("background-color: rgb(110,0,0);");
        }
        //更新风力、风向
        mFxList[i]->setText(mDays[i].fx);
        mFlList[i]->setText(mDays[i].fl);
    }
    ui->lblHighCurve->update();
    ui->lblLowCurve->update();
}

void MainWindow::paintHighCurve()
{
    QPainter painter(ui->lblHighCurve);
    // 设置抗锯齿
    painter.setRenderHint(QPainter::Antialiasing, true);

    QVector<int> pointX;
    for (const auto& label : mWeekList)
    {
        pointX.push_back(label->pos().x() + label->width() / 2);
    }

    int tempSum = 0;
    int tempAvg = 0;
    for (const auto& dayWeather : mDays)
    {
        tempSum += dayWeather.high;
    }
    tempAvg = tempSum / mDays.size();

    QVector<int> pointY;
    int yCenter = ui->lblHighCurve->height() / 2;
    for (const auto& dayWeather : mDays)
    {
        pointY.push_back(yCenter - (dayWeather.high - tempAvg) * kIncrement);
    }

    QPen pen = painter.pen();
    pen.setWidth(1);
    pen.setColor(QColor(255, 170, 0));
    painter.setPen(pen);
    painter.setBrush(QColor(255, 170, 0));

    for (int i = 0; i < mDays.size(); ++i)
    {
        painter.drawEllipse(QPoint(pointX[i], pointY[i]), kPointRadius, kPointRadius);
        painter.drawText(QPoint(pointX[i] - kTextOffsetX, pointY[i] - kTextOffsetY), QString::number(mDays[i].high) + "°C");
    }
    for (int i = 0; i < pointX.size() - 1; ++i)
    {
        if (i == 0)
        {
            pen.setStyle(Qt::DotLine);
            painter.setPen(pen);
        }
        else
        {
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
        }
        painter.drawLine(QPoint(pointX[i], pointY[i]), QPoint(pointX[i + 1], pointY[i + 1]));
    }
}

void MainWindow::paintLowCurve()
{
    QPainter painter(ui->lblLowCurve);
    // 设置抗锯齿
    painter.setRenderHint(QPainter::Antialiasing, true);

    QVector<int> pointX;
    for (const auto& label : mWeekList)
    {
        pointX.push_back(label->pos().x() + label->width() / 2);
    }

    int tempSum = 0;
    int tempAvg = 0;
    for (const auto& dayWeather : mDays)
    {
        tempSum += dayWeather.low;
    }
    tempAvg = tempSum / mDays.size();

    QVector<int> pointY;
    int yCenter = ui->lblHighCurve->height() / 2;
    for (const auto& dayWeather : mDays)
    {
        pointY.push_back(yCenter - (dayWeather.low - tempAvg) * kIncrement);
    }

    QPen pen = painter.pen();
    pen.setWidth(1);
    pen.setColor(QColor(0, 170, 255));
    painter.setPen(pen);
    painter.setBrush(QColor(0, 170, 255));

    for (int i = 0; i < mDays.size(); ++i)
    {
        painter.drawEllipse(QPoint(pointX[i], pointY[i]), kPointRadius, kPointRadius);
        painter.drawText(QPoint(pointX[i] - kTextOffsetX, pointY[i] - kTextOffsetY), QString::number(mDays[i].low) + "°C");
    }
    for (int i = 0; i < pointX.size() - 1; ++i)
    {
        if (i == 0)
        {
            pen.setStyle(Qt::DotLine);
            painter.setPen(pen);
        }
        else
        {
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
        }
        painter.drawLine(QPoint(pointX[i], pointY[i]), QPoint(pointX[i + 1], pointY[i + 1]));
    }
}

void MainWindow::onReplied(QNetworkReply *reply)
{
    qDebug() << "reply success";

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "operation: " << reply->operation();
    qDebug() << "status code: " << statusCode;
    qDebug() << "url: " << reply->url();
    qDebug() << "raw header: " << reply->rawHeaderList();

    if(reply->error() != QNetworkReply::NoError || statusCode != 200)
    {
        qDebug() << reply->errorString().toLatin1().data();
        QMessageBox::warning(this, "警告", "请求数据失败", QMessageBox::Ok);
    }
    else
    {
        QByteArray byteArray = reply->readAll();
        qDebug() << "read all: " << byteArray.data();
        parseJson(byteArray);
    }

    reply->deleteLater();
}

void MainWindow::setupContainer()
{
    //将控件装进vector里方便批量处理
    mWeekList << ui->lblWeek0 << ui->lblWeek1 << ui->lblWeek2 << ui->lblWeek3 << ui->lblWeek4 << ui->lblWeek5;
    mDateList << ui->lblDate0 << ui->lblDate1 << ui->lblDate2 << ui->lblDate3 << ui->lblDate4 << ui->lblDate5;
    mTypeList << ui->lblType0 << ui->lblType1 << ui->lblType2 << ui->lblType3 << ui->lblType4 << ui->lblType5;
    mTypeIconList << ui->lblTypeIcon0 << ui->lblTypeIcon1 << ui->lblTypeIcon2 << ui->lblTypeIcon3 << ui->lblTypeIcon4 << ui->lblTypeIcon5;
    mAqiList << ui->lblQuality0 << ui->lblQuality1 << ui->lblQuality2 << ui->lblQuality3 << ui->lblQuality4 << ui->lblQuality5;
    mFxList << ui->lblFx0 << ui->lblFx1 << ui->lblFx2 << ui->lblFx3 << ui->lblFx4 << ui->lblFx5;
    mFlList << ui->lblFl0 << ui->lblFl1 << ui->lblFl2 << ui->lblFl3 << ui->lblFl4 << ui->lblFl5;

    //天气对应图标
    mTypeMap.insert("暴雪",":/res/type/BaoXue.png");
    mTypeMap.insert("暴雨",":/res/type/BaoYu.png");
    mTypeMap.insert("暴雨到暴雪",":/res/type/BaoYuDaoDaBaoYu.png");
    mTypeMap.insert("大暴雨",":/res/type/DaBaoYu.png");
    mTypeMap.insert("大暴雨到大暴雪",":/res/type/DaBaoYuDaoTeDaBaoYu.png");
    mTypeMap.insert("大到暴雪",":/res/type/DaDaoBaoXue.png");
    mTypeMap.insert("大到暴雨",":/res/type/DaDaoBaoYu.png");
    mTypeMap.insert("大雪",":/res/type/DaXue.png");
    mTypeMap.insert("大雨",":/res/type/DaYu.png");
    mTypeMap.insert("冻雨",":/res/type/DongYu.png");
    mTypeMap.insert("多云",":/res/type/DuoYun.png");
    mTypeMap.insert("浮尘",":/res/type/FuChen.png");
    mTypeMap.insert("雷阵雨",":/res/type/LeiZhenYu.png");
    mTypeMap.insert("雷阵雨伴有冰雹",":/res/type/LeiZhenYuBanYouBingBao.png");
    mTypeMap.insert("霾",":/res/type/Mai.png");
    mTypeMap.insert("强沙尘暴",":/res/type/QiangShaChenBao.png");
    mTypeMap.insert("晴",":/res/type/Qing.png");
    mTypeMap.insert("沙尘暴",":/res/type/ShaChenBao.png");
    mTypeMap.insert("特大暴雨",":/res/type/TeDaBaoYu.png");
    mTypeMap.insert("雾",":/res/type/Wu.png");
    mTypeMap.insert("小到中雨",":/res/type/XiaoDaoZhongYu.png");
    mTypeMap.insert("小到中雪",":/res/type/XiaoDaoZhongXue.png");
    mTypeMap.insert("小雪",":/res/type/XiaoXue.png");
    mTypeMap.insert("小雨",":/res/type/XiaoYu.png");
    mTypeMap.insert("雪",":/res/type/Xue.png");
    mTypeMap.insert("扬沙",":/res/type/YangSha.png");
    mTypeMap.insert("阴",":/res/type/Yin.png");
    mTypeMap.insert("雨",":/res/type/Yu.png");
    mTypeMap.insert("雨夹雪",":/res/type/YuJiaXue.png");
    mTypeMap.insert("阵雨",":/res/type/ZhenYu.png");
    mTypeMap.insert("阵雪",":/res/type/ZhenXue.png");
    mTypeMap.insert("中雨",":/res/type/ZhongYu.png");
    mTypeMap.insert("中雪",":/res/type/ZhongXue.png");
}

void MainWindow::on_btnSearch_clicked()
{
    QString cityName = ui->leCity->text();
    if (cityName.isEmpty())
    {
        QMessageBox::warning(this, "警告", "请输入城市名");
        return;
    }
    getWeatherInfo(cityName);
}

