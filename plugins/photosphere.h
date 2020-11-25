#ifndef SQUIRCLE_H
#define SQUIRCLE_H

#include <QtQuick/QQuickItem>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

class PhotoSphereRenderer : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    PhotoSphereRenderer() : m_t(0), m_program(0) , m_vao(0) { }
    ~PhotoSphereRenderer();

    void setImage(QString image) { m_image = image; }
    void setViewportSize(const QSize &size) { m_viewportSize = size; }
    void setWindow(QQuickWindow *window) { m_window = window; }

public slots:
    void paint();

private:
    QSize m_viewportSize;
    qreal m_t;
    QString m_image;
    QOpenGLShaderProgram *m_program;
    QOpenGLVertexArrayObject *m_vao;
    QOpenGLBuffer *m_vbo;
    QQuickWindow *m_window;
};

class PhotoSphere : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString image READ image WRITE setImage NOTIFY imageChanged)

public:
    PhotoSphere();
    
    QString image() const { return m_image; }
    void setImage(QString image);

signals:
    void tChanged();
    void imageChanged();

public slots:
    void sync();
    void cleanup();

private slots:
    void handleWindowChanged(QQuickWindow *win);

private:
    qreal m_t;
    QString m_image;
    PhotoSphereRenderer *m_renderer;
};

#endif // SQUIRCLE_H
