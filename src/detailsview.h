#ifndef DETAILSVIEW_H
#define DETAILSVIEW_H

#include <QWidget>

class QPushButton;
class AutonomousVehicleProject;
class BackgroundDetails;
class WaypointDetails;
class TrackLineDetails;
class SurveyPatternDetails;
class PlatformDetails;
class ROSDetails;
class BehaviorDetails;

class DetailsView : public QWidget
{
    Q_OBJECT
public:
    explicit DetailsView(QWidget *parent = 0);

    void setProject(AutonomousVehicleProject *project);

    QSize sizeHint() const override;

signals:

public slots:
    void onCurrentItemChanged(const QModelIndex  &current, const QModelIndex &previous);
    void onRenamedPushButton_clicked();
    void onExecutePushButton_clicked();

private:
    AutonomousVehicleProject *m_project;

    QPushButton* m_executePushButton;
    QPushButton* m_renamePushButton;

    QWidget * currentWidget;

    BackgroundDetails * backgroundDetails;
    WaypointDetails * waypointDetails;
    TrackLineDetails * trackLineDetails;
    SurveyPatternDetails * surveyPatternDetails;
    PlatformDetails * platformDetails;
    ROSDetails * rosDetails;
    BehaviorDetails * behaviorDetails;

    void setCurrentWidget(QWidget *widget);
};

#endif // DETAILSVIEW_H
