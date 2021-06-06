#include "radardisplay.h"

#include <cmath>
#include "autonomousvehicleproject.h"
#include "roslink.h"
#include <QOffscreenSurface>
#include <QOpenGLShader>
#include <QPainter>
#include <QOpenGLFramebufferObject>
#include <tf2/utils.h>


RadarDisplay::RadarDisplay(ROSLink* parent): QObject(parent), GeoGraphicsItem(parent)
{
    
    QSurfaceFormat surfaceFormat;
    surfaceFormat.setMajorVersion(4);
    surfaceFormat.setMinorVersion(3);
    surfaceFormat.setAlphaBufferSize(8);
    surfaceFormat.setRedBufferSize(8);
    surfaceFormat.setGreenBufferSize(8);
    surfaceFormat.setBlueBufferSize(8);
    
    m_context = new QOpenGLContext();
    m_context->setFormat(surfaceFormat);
    m_context->create();
    if(!m_context->isValid()) return;

    m_surface = new QOffscreenSurface();
    m_surface->setFormat(surfaceFormat);
    m_surface->create();
    if(!m_surface->isValid()) return;

    m_context->makeCurrent(m_surface);
    QOpenGLFramebufferObjectFormat fboFormat;
    m_fbo = new QOpenGLFramebufferObject(2048,2048,fboFormat);

    initializeGL();

}

void RadarDisplay::setTF2Buffer(tf2_ros::Buffer* buffer)
{
    m_tf_buffer = buffer;
}

void RadarDisplay::setMapFrame(std::string mapFrame)
{
    m_mapFrame = mapFrame;
}

void RadarDisplay::setPixelSize(double s)
{
    m_pixel_size = s;
}

void RadarDisplay::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    
    QVector<GLfloat> vertData;
    vertData.append(-1.0); vertData.append(-1.0); vertData.append(0.0);
    vertData.append(1.0); vertData.append(-1.0); vertData.append(0.0);
    vertData.append(1.0); vertData.append(1.0); vertData.append(0.0);
    vertData.append(-1.0); vertData.append(1.0); vertData.append(0.0);
    
    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(vertData.constData(), vertData.count() * sizeof(GLfloat));
    
#define PROGRAM_VERTEX_ATTRIBUTE 0

    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vsrc =
        "attribute highp vec4 vertex;\n"
        "varying mediump vec4 texc;\n"
        "uniform mediump mat4 matrix;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = matrix * vertex;\n"
        "    texc = vertex;\n"
        "}\n";
    vshader->compileSourceCode(vsrc);

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *fsrc =
        "#define M_PI 3.1415926535897932384626433832795\n"
        "uniform sampler2D texture;\n"
        "varying mediump vec4 texc;\n"
        "uniform float minAngle;\n"
        "uniform float maxAngle;\n"
        "uniform float fade;\n"
        "uniform vec4 color;\n"
        "void main(void)\n"
        "{\n"
        "    if(texc.x == 0.0) discard;\n"
        "    float r = length(texc.xy);\n"
        "    if(r>1.0) discard;\n"
        "    float theta = atan(texc.x, texc.y);\n"
        "    if(minAngle > 0.0 && theta < 0.0) theta += 2.0*M_PI;\n"
        "    if(theta < minAngle) discard;\n"
        "    if(theta > maxAngle) discard;\n"
        "    vec4 radarData = texture2D(texture, vec2(r, (theta-minAngle)/(maxAngle-minAngle)));\n"
        "    if(radarData.r < 0.01) discard;\n"
        "    gl_FragColor = color*fade*radarData.r;\n"
        "    //gl_FragColor.a = radarData.r*fade;\n"
        "}\n";
    fshader->compileSourceCode(fsrc);

    m_program = new QOpenGLShaderProgram;
    m_program->addShader(vshader);
    m_program->addShader(fshader);
    m_program->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
    m_program->link();

    m_program->bind();
    m_program->setUniformValue("texture", 0);
}

QRectF RadarDisplay::boundingRect() const
{
    if(!m_sectors.empty())
    {
        double r = m_sectors.back().range;
        r /= m_pixel_size;
        return QRectF(-r,-r,r*2,r*2);
    }
    return QRectF();
}

void RadarDisplay::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if(m_show_radar && !m_sectors.empty())
    {
        double r = m_sectors.back().range;
        //qDebug() << "range: " << r;
        r /= m_pixel_size;
        QPen p;
        p.setColor(Qt::green);
        p.setWidth(2);
        painter->setPen(p);
        QRectF radarRect(-r,-r,r*2,r*2);
        //painter->drawEllipse(radarRect);

        
        m_context->makeCurrent(m_surface);
        m_fbo->bind();
        glClear(GL_COLOR_BUFFER_BIT);

        QMatrix4x4 matrix;
        matrix.ortho(-1, 1, -1, 1, 4.0f, 15.0f);
        matrix.translate(0.0f, 0.0f, -10.0f);
        
        glViewport(0,0,2048,2048);
        
        m_program->setUniformValue("matrix", matrix);
        m_program->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
        m_program->setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));
    
        ros::Time now = ros::Time::now();
        float persistance = 5.0;
        while(!m_sectors.empty() && m_sectors.front().timestamp + ros::Duration(persistance) < now)
        {
            if(m_sectors.front().sectorImage)
                delete m_sectors.front().sectorImage;
            if(m_sectors.front().sectorTexture)
                delete m_sectors.front().sectorTexture;
            m_sectors.pop_front();
        }
        for(Sector &s: m_sectors)
            if(s.sectorImage)
            {
                float fade = 1.0-((now-s.timestamp).toSec()/persistance);
                if(!s.sectorTexture)
                {
                    s.sectorTexture = new QOpenGLTexture(*s.sectorImage);
                }
                if(s.heading < 0)
                {
                    //std::cerr << "tf buffer? " << m_tf_buffer << std::endl;
                    //std::cerr << "map frame: " << m_mapFrame << std::endl;
                    //if(m_tf_buffer && !m_mapFrame.empty())
                        //std::cerr << "can transform? " << m_tf_buffer->canTransform(m_mapFrame, s.frame_id.toStdString(), s.timestamp) << std::endl;
                    //std::cerr << "timestamp: " << s.timestamp << std::endl;
                    //std::cerr << "frame_id: " << s.frame_id.toStdString() << std::endl;
                    if(m_tf_buffer && !m_mapFrame.empty() && m_tf_buffer->canTransform(m_mapFrame, s.frame_id.toStdString(), s.timestamp))
                    {
                        geometry_msgs::TransformStamped t = m_tf_buffer->lookupTransform(m_mapFrame, s.frame_id.toStdString(), s.timestamp);
                        double heading =  90.0-180.0*tf2::getYaw(t.transform.rotation);
                        s.heading = heading*M_PI/180.0;
                        std::cerr << "radar sector heading: " << s.heading << std::endl;
                    }
                }
                if(s.heading >= 0.0)
                {
                    m_program->setUniformValue("minAngle", GLfloat(s.heading+s.angle1-s.half_scanline_angle*1.1));
                    m_program->setUniformValue("maxAngle", GLfloat(s.heading+s.angle2+s.half_scanline_angle*1.1));
                }
                else
                {
                    m_program->setUniformValue("minAngle", GLfloat(s.angle1-s.half_scanline_angle*1.1));
                    m_program->setUniformValue("maxAngle", GLfloat(s.angle2+s.half_scanline_angle*1.1));
                }
                m_program->setUniformValue("fade", GLfloat(fade));
                m_program->setUniformValue("color", m_color);
                
                s.sectorTexture->bind();
                glDisable(GL_BLEND);
                glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            }
        
        painter->drawImage(radarRect, m_fbo->toImage());
        
    }
    
}


void RadarDisplay::addSector(double angle1, double angle2, double range, QImage *sector, ros::Time stamp, QString frame_id)
{
    prepareGeometryChange();
    //std::cerr << angle1 << " - " << angle2 << " degs, " << range << " meters" << std::endl;
    Sector s;
    if(angle1 > angle2)
        s.angle1 = (angle1-360.0)*M_PI/180.0;
    else
        s.angle1 = angle1*M_PI/180.0;
    s.angle2 = angle2*M_PI/180.0;
    s.half_scanline_angle = (s.angle2 - s.angle1)/(2.0*sector->height());
    s.range = range;
    s.sectorImage = sector;
    s.timestamp = stamp;
    s.frame_id = frame_id;
    m_sectors.push_back(s);
    update();
}

void RadarDisplay::showRadar(bool show)
{
    m_show_radar = show;
    update();
}

const QColor& RadarDisplay::getColor() const
{
    return m_color;
}

void RadarDisplay::setColor(QColor color)
{
    m_color = color;
}
