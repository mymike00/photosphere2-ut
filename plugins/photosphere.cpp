#include "photosphere.h"

#include <QtQuick/qquickwindow.h>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLContext>
#include <QOpenGLTexture>

PhotoSphere::PhotoSphere()
    : m_t(0)
    , m_image("")
    , m_renderer(0)
{
    connect(this, &QQuickItem::windowChanged, this, &PhotoSphere::handleWindowChanged);
}

void PhotoSphere::setImage(QString image)
{
    if (image == m_image)
        return;
    m_image = image;
    emit imageChanged();
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
    delete m_vao;
    m_vbo->destroy();
    delete m_vbo;
}

void PhotoSphere::sync()
{
    if (!m_renderer) {
        m_renderer = new PhotoSphereRenderer();
        connect(window(), &QQuickWindow::beforeRendering, m_renderer, &PhotoSphereRenderer::paint, Qt::DirectConnection);
    }
    m_renderer->setViewportSize(window()->size() * window()->devicePixelRatio());
    m_renderer->setT(m_t);
    m_renderer->setImage(m_image);
    m_renderer->setWindow(window());
}

void PhotoSphereRenderer::paint()
{

    if (!m_vao) {
        m_vao = new QOpenGLVertexArrayObject;
        m_vao->create();
    }
    if (!m_program) {
        initializeOpenGLFunctions();

        m_program = new QOpenGLShaderProgram();
        m_program->addCacheableShaderFromSourceCode(QOpenGLShader::Vertex,
                                                    "#version 130\n"
                                                    "uniform mediump mat4   projection;\n"
                                                    "attribute mediump vec3 position;\n"
                                                    "attribute mediump vec2 texcoord;\n"
                                                    "varying mediump vec2 v_texcoord;\n"
                                                    "void main() {\n"
                                                    "    v_texcoord = texcoord;\n"
                                                    "    gl_Position = projection * vec4(position, 1.);\n"
                                                    "}\n");
        m_program->addCacheableShaderFromSourceCode(QOpenGLShader::Fragment,
                                                    "#version 130\n"
                                                    "uniform sampler2D texture;\n"
                                                    "uniform mediump float scale, aspect;\n"
                                                    "uniform mediump mat3 transform;\n"

                                                    "varying mediump vec2 v_texcoord;\n"

                                                    "#define PI 3.141592653589793\n"

                                                    "void main() {\n"
                                                    "    vec2 mediump rads = vec2(PI * 2., PI);\n"

                                                    "    vec2 mediump pnt = (v_texcoord - .5) * vec2(scale, scale * aspect);\n"

                                                    "    // Project to Sphere;\n"
                                                    "    float mediump x2y2 = pnt.x * pnt.x + pnt.y * pnt.y;\n"
                                                    "    vec3 mediump sphere_pnt = vec3(2. * pnt, x2y2 - 1.) / (x2y2 + 1.);\n"
                                                    "    sphere_pnt *= transform;\n"

                                                    "    // Convert to Spherical Coordinates\n"
                                                    "    float mediump r = length(sphere_pnt);\n"
                                                    "    float mediump lon = atan(sphere_pnt.y, sphere_pnt.x);\n"
                                                    "    float mediump lat = acos(sphere_pnt.z / r);\n"
                                                    // "    gl_FragColor = texture2D(texture, v_texcoord.st);\n"
                                                    "    gl_FragColor = texture2D(texture, vec2(lon, lat) / rads);\n"
                                                    "}\n");

        m_program->bindAttributeLocation("position", 0);
        m_program->bindAttributeLocation("texcoord", 1);
        m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
        m_program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));
        m_program->link();

    }
    m_program->bind();
    m_vao->bind();

    m_program->enableAttributeArray(0);
    m_program->setUniformValue("texture", 0);

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
    QMatrix4x4 projection = QMatrix4x4();
    projection.ortho(0, 1, 1, 0, -1, 1);
    m_program->setUniformValue("projection", projection);

    QQuaternion quat = QQuaternion();
    QMatrix4x4 temp1 = QMatrix4x4(quat.toRotationMatrix());
    QMatrix4x4 transform = temp1;
    m_program->setUniformValue("transform", transform);


    m_program->setUniformValue("aspect", m_viewportSize.height() / m_viewportSize.width());
    m_program->setUniformValue("scale", 20);

    m_program->setAttributeArray(0, GL_FLOAT, values, 2);
    m_program->setAttributeArray(1, GL_FLOAT, texCoords, 2);

    glViewport(0, 0, m_viewportSize.width(), m_viewportSize.height());

    glEnable( GL_DEPTH_TEST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    QImage *img = new QImage(m_image);
    QOpenGLTexture *texture = new QOpenGLTexture(img->mirrored());
    texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->bind();

    m_program->enableAttributeArray(0);
    m_program->enableAttributeArray(1);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    // glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_program->disableAttributeArray(0);
    m_program->disableAttributeArray(1);
    m_vao->release();
    m_program->release();
    texture->release(0);


    // Not strictly needed for this example, but generally useful for when
    // mixing with raw OpenGL.
    m_window->resetOpenGLState();
}
