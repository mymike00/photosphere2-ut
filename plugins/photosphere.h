#ifndef SQUIRCLE_H
#define SQUIRCLE_H

#include <QtQuick/QQuickItem>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>

class PhotoSphereRenderer : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    PhotoSphereRenderer()
    : m_scale(0)
    , m_program(0)
    , m_image(0)
    { }
    ~PhotoSphereRenderer();

    void setImage(QString imageUrl) { m_imageUrl = imageUrl; }
    void setScale(qreal scale) { m_scale = scale; }
    void setViewportSize(const QSize &size) { m_viewportSize = size; }
    void setWindow(QQuickWindow *window) { m_window = window; }

public slots:
    void paint();

private:
    QSize m_viewportSize;
    QString m_imageUrl;
    QImage *m_image;
    QOpenGLTexture *m_texture;
    qreal m_scale;
    QOpenGLShaderProgram *m_program;
    QQuickWindow *m_window;
    QMatrix4x4 m_transformMatrix;
};

class PhotoSphere : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString image READ imageUrl WRITE setImage NOTIFY imageUrlChanged)
    Q_PROPERTY(qreal scale READ scale WRITE setScale NOTIFY scaleChanged)

public:
    PhotoSphere();
    QString imageUrl() const { return m_imageUrl; }
    void setImage(QString imageUrl);
    qreal scale() const { return m_scale; }
    void setScale(qreal scale);

signals:
    void imageUrlChanged();
    void scaleChanged();

public slots:
    void sync();
    void cleanup();

private slots:
    void handleWindowChanged(QQuickWindow *win);

private:
    QString m_imageUrl;
    qreal m_scale;
    PhotoSphereRenderer *m_renderer;
};

#endif // SQUIRCLE_H
