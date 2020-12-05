#include "photosphere.h"

#include <QtQuick/qquickwindow.h>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLContext>
#include <QOpenGLTexture>
#include <QtMath>

PhotoSphere::PhotoSphere()
    : m_imageUrl("")
    , m_scale(0)
    , m_renderer(0)
{
    connect(this, &QQuickItem::windowChanged, this, &PhotoSphere::handleWindowChanged);
}

void PhotoSphere::setImage(QString imageUrl)
{
    if (imageUrl == m_imageUrl)
        return;
    m_imageUrl = imageUrl;
    emit imageUrlChanged();
    if (window())
        window()->update();
}

void PhotoSphere::setScale(qreal scale)
{
    if (scale == m_scale)
        return;
    m_scale = scale;
    emit scaleChanged();
    if (window())
        window()->update();
}

void PhotoSphere::setLongitude(qreal longitude)
{
    if (longitude == m_longitude)
        return;
    m_longitude = longitude;
    emit longitudeChanged();
    emit scaleChanged();
    if (window())
        window()->update();
}

void PhotoSphere::setLatitude(qreal latitude)
{
    if (latitude == m_latitude)
        return;
    m_latitude = latitude;
    emit latitudeChanged();
    emit scaleChanged();
    if (window())
        window()->update();
}

void PhotoSphere::handleWindowChanged(QQuickWindow *win)
{
    if (win) {
        connect(win, &QQuickWindow::beforeSynchronizing, this, &PhotoSphere::sync, Qt::DirectConnection);
        connect(win, &QQuickWindow::sceneGraphInvalidated, this, &PhotoSphere::cleanup, Qt::DirectConnection);
        // If we allow QML to do the clearing, they would clear what we paint
        // and nothing would show.
        win->setClearBeforeRendering(false);
    }
}

void PhotoSphere::cleanup()
{
    if (m_renderer) {
        delete m_renderer;
        m_renderer = 0;
    }
}

PhotoSphereRenderer::~PhotoSphereRenderer()
{
    delete m_program;
}

void PhotoSphere::sync()
{
    if (!m_renderer) {
        m_renderer = new PhotoSphereRenderer();
        connect(window(), &QQuickWindow::beforeRendering, m_renderer, &PhotoSphereRenderer::paint, Qt::DirectConnection);
    }
    m_renderer->setViewportSize(window()->size() * window()->devicePixelRatio());
    m_renderer->setImage(m_imageUrl);
    m_renderer->setScale(m_scale);
    m_renderer->setLongitude(m_longitude);
    m_renderer->setLatitude(m_latitude);
    m_renderer->setWindow(window());
}

QVector3D PhotoSphereRenderer::getArcBallVector(int x, int y)
{
    QVector3D pt = QVector3D(2.0 * x / m_window->width() - 1.0, 2.0 * y / m_window->height() - 1.0, 0);

    pt.setZ(1);
    pt.normalize();

    return pt;
}

void PhotoSphereRenderer::startDrag(int x, int y)
{
    m_dragging = true;
    m_startDragX = x;
    m_startDragY = y;
    m_oldTransformMatrix = m_transformMatrix;
}
void PhotoSphereRenderer::endDrag()
{
    m_dragging = false;
    m_startDragX = m_longitude;
    m_startDragY = m_latitude;
}

void PhotoSphereRenderer::rotateView(qreal degrees) {
    qreal angle = degrees / m_scale;

    QMatrix4x4 tmp;
    tmp.rotate(angle, QVector3D(0,0,1));
    m_transformMatrix = tmp * m_transformMatrix;
    m_oldTransformMatrix = m_transformMatrix;
}
void PhotoSphereRenderer::lookBelow()
{
    m_transformMatrix = QMatrix4x4();
}

void PhotoSphereRenderer::paint()
{
    if (!m_program) {
        initializeOpenGLFunctions();

        m_program = new QOpenGLShaderProgram();
        m_program->addCacheableShaderFromSourceCode(QOpenGLShader::Vertex,
                                                    "uniform highp mat4   projection;\n"
                                                    "attribute highp vec3 position;\n"
                                                    "attribute highp vec2 texcoord;\n"
                                                    "varying highp vec2 v_texcoord;\n"
                                                    "void main() {\n"
                                                    "    v_texcoord = texcoord;\n"
                                                    "    gl_Position = projection * vec4(position, 1.);\n"
                                                    "}\n");
        m_program->addCacheableShaderFromSourceCode(QOpenGLShader::Fragment,
                                                    "uniform sampler2D texture;\n"
                                                    "uniform highp float scale, aspect;\n"
                                                    "uniform highp mat3 transform;\n"

                                                    "varying highp vec2 v_texcoord;\n"

                                                    "#define PI 3.141592653589793\n"

                                                    "void main() {\n"
                                                    "    vec2 rads = vec2(PI * 2., PI);\n"

                                                    "    vec2 pnt = (v_texcoord - .5) / vec2(scale, scale / aspect);\n"

                                                    "    // Project to Sphere;\n"
                                                    "    float x2y2 = pnt.x * pnt.x + pnt.y * pnt.y;\n"
                                                    "    vec3 sphere_pnt = vec3(2. * pnt, x2y2 - 1.) / (x2y2 + 1.);\n"
                                                    "    sphere_pnt *= transform;\n"

                                                    "    // Convert to Spherical Coordinates\n"
                                                    "    float r = length(sphere_pnt);\n"
                                                    "    float lon = atan(sphere_pnt.y, sphere_pnt.x);\n"
                                                    "    float lat = acos(sphere_pnt.z / r);\n"
                                                    // "    gl_FragColor = texture2D(texture, v_texcoord.st);\n"
                                                    "    gl_FragColor = texture2D(texture, vec2(lon, lat) / rads);\n"
                                                    "}\n");

        m_program->bindAttributeLocation("position", 0);
        m_program->bindAttributeLocation("texcoord", 1);
        m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
        m_program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));
        m_program->link();

        m_image = new QImage(m_imageUrl);
        m_texture = new QOpenGLTexture(m_image->scaled(QSize(4000, 2000), Qt::KeepAspectRatio));
        m_texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        m_texture->setMagnificationFilter(QOpenGLTexture::Linear);
        m_texture->setWrapMode(QOpenGLTexture::Repeat);
    }
    m_program->bind();

    float values[] = {
        +1., +1.,
        -1., +1.,
        -1., -1.,
        +1., -1.
    };
    static GLfloat const texCoords[] = {
            0.0f, 0.0f,
            2.0f, 0.0f,
            2.0f, 2.0f,
            0.0f, 2.0f
    };

    if (m_program->isLinked()) {
        m_program->setUniformValue("texture", 0);

        QMatrix4x4 projection = QMatrix4x4();
        projection.ortho(0, 1, 1, 0, -1, 1);
        m_program->setUniformValue("projection", projection);

        // ARCBALL
        if (m_dragging) {
            QVector3D start_arc_ball = getArcBallVector(m_startDragX, m_startDragY);
            QVector3D arc_ball = getArcBallVector(m_longitude, m_latitude);

            qreal angle = qAcos(qMin(1.0f, QVector3D::dotProduct(start_arc_ball, arc_ball))) / m_scale;

            QVector3D rotAxis = QVector3D::crossProduct(start_arc_ball, arc_ball);
            rotAxis.setZ(0);

            QMatrix4x4 tmp;
            tmp.rotate(qRadiansToDegrees(angle), rotAxis);
            m_transformMatrix = tmp * m_oldTransformMatrix;
            // startDrag(m_longitude, m_latitude);
        }

        // transform needs to be an exact QMatrix3x3. A QMatrix4x4 won't work
        m_program->setUniformValue("transform", m_transformMatrix.toGenericMatrix<3, 3>());

        // force cast to float is needed for the GLSL shaders
        m_program->setUniformValue("aspect", (float) m_viewportSize.height() / m_viewportSize.width());
        m_program->setUniformValue("scale", (float) m_scale);

        m_program->setAttributeArray(0, GL_FLOAT, values, 2);
        m_program->setAttributeArray(1, GL_FLOAT, texCoords, 2);
    }

    glViewport(0, 0, m_viewportSize.width(), m_viewportSize.height());


    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    m_texture->bind();

    m_program->enableAttributeArray(0);
    m_program->enableAttributeArray(1);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    m_program->disableAttributeArray(0);
    m_program->disableAttributeArray(1);
    m_program->release();
    m_texture->release(0);


    // Not strictly needed for this example, but generally useful for when
    // mixing with raw OpenGL.
    m_window->resetOpenGLState();
}
