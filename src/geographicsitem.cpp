#include "geographicsitem.h"
#include "backgroundraster.h"
#include "autonomousvehicleproject.h"
#include "missionitem.h"
#include <QGraphicsSimpleTextItem>
#include <QFont>
#include <QBrush>
#include <QPen>
#include <QDebug>

GeoGraphicsItem::GeoGraphicsItem(QGraphicsItem *parentItem): QGraphicsItem(parentItem), m_showLabelFlag(false)
{
    m_label = new QGraphicsSimpleTextItem(this);
    m_label->setFlag(GraphicsItemFlag::ItemIgnoresTransformations);
    auto font = m_label->font();
    font.setPointSize(20);
    font.setBold(true);
    m_label->setFont(font);
    m_label->setBrush(QBrush(QColor("black")));
    QPen p(QColor("white"));
    p.setWidth(0);
    m_label->setPen(p);
    //m_label->setFlag(QGraphicsItem::ItemIsMovable); this caused other elements to move while trying to move the label!
}

QPointF GeoGraphicsItem::geoToPixel(const QGeoCoordinate &point, AutonomousVehicleProject *p) const
{
    if(p)
        return geoToPixel(point, p->getBackgroundRaster());
    return QPointF();

}

QPointF GeoGraphicsItem::geoToPixel(const QGeoCoordinate &point, BackgroundRaster *bg) const
{
    if(bg)
    {
        QPointF ret = bg->geoToPixel(point);
        QGraphicsItem *pi = parentItem();
        if(pi)
        {
            return ret - pi->scenePos();
        }
        return ret;
    }
}

void GeoGraphicsItem::prepareGeometryChange()
{
    QGraphicsItem::prepareGeometryChange();
}

void GeoGraphicsItem::setLabel(const QString &label)
{
    m_labelText = label;
    if(m_showLabelFlag)
        m_label->setText(m_labelText);
}

void GeoGraphicsItem::setLabelPosition(QPointF pos)
{
    m_label->setPos(pos);
}

bool GeoGraphicsItem::showLabelFlag() const
{
    return m_showLabelFlag;
}

void GeoGraphicsItem::setShowLabelFlag(bool show)
{
    m_showLabelFlag = show;
    if(show)
        m_label->setText(m_labelText);
    else
        m_label->setText("");
}

BackgroundRaster* GeoGraphicsItem::findParentBackgroundRaster() const
{
    BackgroundRaster* ret = nullptr;
    QGraphicsItem* parent = parentItem();
    while(parent)
    {
        ret = dynamic_cast<BackgroundRaster*>(parent);
        if(ret)
            return ret;
        parent = parent->parentItem();
    }
    return ret;
}

