#ifndef CAMP_HELM_MANAGER_H
#define CAMP_HELM_MANAGER_H

#include <QWidget>
#include "ros/ros.h"
#include "project11_msgs/Heartbeat.h"

namespace Ui
{
class HelmManager;
}

class HelmManager: public QWidget
{
  Q_OBJECT

public:
  explicit HelmManager(QWidget *parent =0);
  ~HelmManager(); 

signals:
  void pilotingModeUpdated(QString piloting_mode);
  void heartbeatTimesUpdated(double last_heartbeat_timestamp, double last_heartbeat_receive_time);
  void robotStatusUpdated(QString status);
  void requestPilotingMode(QString piloting_mode);

public slots:
  void updateRobotNamespace(QString robot_namespace);
  void sendPilotingModeRequest(QString piloting_mode);

private slots:
  void on_standbyPushButton_clicked(bool checked);
  void on_autonomousPushButton_clicked(bool checked);

  void on_timeLatencyConfigPushButton_clicked(bool checked);
  
  void updatePilotingMode(QString const &piloting_mode);
  void updateHeartbeatTimes(double last_heartbeat_timestamp, double last_heartbeat_receive_time);
  void watchdogUpdate();

private:
  void heartbeatCallback(const project11_msgs::Heartbeat::ConstPtr& message);

  Ui::HelmManager* ui;

  ros::Subscriber m_heartbeat_subscriber;
  ros::Publisher m_send_command_publisher;

  ros::Time m_last_heartbeat_timestamp;
  ros::Time m_last_heartbeat_receive_time;
    
  QTimer * m_watchdog_timer;

  ros::Duration max_green_duration_ = ros::Duration(2.0);
  ros::Duration max_yellow_duration_ = ros::Duration(5.0);

  QString robot_namespace_;
};

#endif
