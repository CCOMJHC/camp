#ifndef ROSNODEDETAILS_H
#define ROSNODEDETAILS_H

#include <QWidget>

namespace Ui
{
class ROSDetails;
}

namespace ros
{
    class Time;
}

class ROSLink;

class ROSDetails : public QWidget
{
    Q_OBJECT
    
public:
    explicit ROSDetails(QWidget *parent =0);
    ~ROSDetails();
    
    void setROSLink(ROSLink *rosLink);
    
public slots:
    void rangeAndBearingUpdate(double range, ros::Time const &range_timestamp, double bearing, ros::Time const &bearing_timestamp);
    void sogUpdate(qreal sog, qreal sog_avg);
    void sendNextItem();
    void restartMission();
    void clearTasks();

private slots:
    void on_stopPingingPushButton_clicked(bool checked);
    void on_startPingingPushButton_clicked(bool checked);
    void on_pingAndLogPushButton_clicked(bool checked);
    void on_incrementLinePushButton_clicked(bool checked);

    void updateMissionStatus(QString const &status);
    void on_gotoLinePushButton_clicked(bool checked);
    void on_startLinePushButton_clicked(bool checked);

    void on_nextMissionItemPushButton_clicked(bool checked);
    void on_restartMissionPushButton_clicked(bool checked);
    void on_clearTasksPushButton_clicked(bool checked);
    
    void on_missionStatusTextBrowser_customContextMenuRequested(const QPoint &pos);
    
private:
    Ui::ROSDetails* ui;
    ROSLink *m_rosLink;
};

#endif // ROSNODEDETAILS_H
