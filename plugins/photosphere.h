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
    , m_startDragX(0)
    , m_startDragY(0)
    , m_dragging(false)
    { }
    ~PhotoSphereRenderer();

    void setImage(QString imageUrl) { m_imageUrl = imageUrl; }
    void setScale(qreal scale) { m_scale = scale; }
    void setLongitude(qreal longitude) {m_longitude = longitude;}
    void setLatitude(qreal latitude) {m_latitude = latitude; }
    void setViewportSize(const QSize &size) { m_viewportSize = size; }
    void setWindow(QQuickWindow *window) { m_window = window; }
    QVector3D getArcBallVector(int x, int y);
    void startDrag(int x, int y);
    void endDrag();
    void rotateView(qreal degrees);
    void lookBelow();

public slots:
    void paint();

private:
    QSize m_viewportSize;
    QString m_imageUrl;
    QImage *m_image;
    QOpenGLTexture *m_texture;
    qreal m_scale;
    qreal m_longitude;
    qreal m_latitude;
    QOpenGLShaderProgram *m_program;
    QQuickWindow *m_window;
    int m_startDragX;
    int m_startDragY;
    bool m_dragging;
    QMatrix4x4 m_transformMatrix;
    QMatrix4x4 m_oldTransformMatrix;
};

class PhotoSphere : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString image READ imageUrl WRITE setImage NOTIFY imageUrlChanged)
    Q_PROPERTY(qreal scale READ scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(qreal longitude READ longitude WRITE setLongitude NOTIFY longitudeChanged)
    Q_PROPERTY(qreal latitude READ latitude WRITE setLatitude NOTIFY latitudeChanged)

public:
    PhotoSphere();
    QString imageUrl() const { return m_imageUrl; }
    void setImage(QString imageUrl);
    qreal scale() const { return m_scale; }
    void setScale(qreal scale);

    qreal longitude() const { return m_longitude; }
    void setLongitude(qreal longitude);
    qreal latitude() const { return m_latitude; }
    void setLatitude(qreal latitude);

    Q_INVOKABLE void startDrag(int x, int y) { m_renderer->startDrag(x, y); }
    Q_INVOKABLE void endDrag() { m_renderer->endDrag(); }
    Q_INVOKABLE void rotateView(qreal degrees) { m_renderer->rotateView(degrees); }
    Q_INVOKABLE void lookBelow() { m_renderer->lookBelow(); }

signals:
    void imageUrlChanged();
    void scaleChanged();
    void longitudeChanged();
    void latitudeChanged();

public slots:
    void sync();
    void cleanup();

private slots:
    void handleWindowChanged(QQuickWindow *win);

private:
    QString m_imageUrl;
    qreal m_scale;
    qreal m_longitude;
    qreal m_latitude;
    PhotoSphereRenderer *m_renderer;
};

#endif // SQUIRCLE_H
