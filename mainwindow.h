#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "weatherdata.h"

#include <QMainWindow>
#include <QMouseEvent>

#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

    void getWeatherInfo(QString cityName);
    void parseJson(QByteArray &byteArray);
    void updateUI();
    void paintHighCurve();
    void paintLowCurve();

private:
    Ui::MainWindow *ui;
    QMenu *mExitMenu; // 右键菜单
    QAction *mExitAction;
    QPoint mOffset;
    QNetworkAccessManager *mNetworkManager;
    Today mToday; // 当天天气
    QVector<Day> mDays{6}; // 六天内天气
    QVector<QLabel*> mWeekList;
    QVector<QLabel*> mDateList;
    QVector<QLabel*> mTypeList;
    QVector<QLabel*> mTypeIconList;
    QVector<QLabel*> mAqiList;
    QVector<QLabel*> mFxList;
    QVector<QLabel*> mFlList;
    QMap<QString, QString> mTypeMap; // 天气类型映射为图标

    const int kIncrement = 3; // 温度曲线增量
    const int kPointRadius = 3; // 曲线锚点的半径
    const int kTextOffsetX = 12; // 曲线描述文本位置x轴上的偏移量
    const int kTextOffsetY = 12; // 曲线描述文本位置x轴上的偏移量

private slots:
    void onReplied(QNetworkReply *reply);
    void setupContainer();
    void on_btnSearch_clicked();
};
#endif // MAINWINDOW_H
