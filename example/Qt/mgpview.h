#ifndef MGPVIEW_H
#define MGPVIEW_H

#include "mgp.h"

#include <QWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QSize>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>


/**
 * The main game view to render scenes into viewport(s).
 */
class MgpView : public QOpenGLWidget, protected QOpenGLFunctions, public mgp::Application
{
    Q_OBJECT
public:

    /**
     * Constructor.
     *
     * @param parent The parent widget.
     */
    explicit MgpView(QWidget* parent = nullptr);

    /**
     * Destructor.
     */
    ~MgpView();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

    void initialize() override;
    void finalize() override;

protected:

    /**
     * @see QWidget::mousePressEvent
     */
    void mousePressEvent(QMouseEvent* evt) override;

    /**
     * @see QWidget::mouseReleaseEvent
     */
    void mouseReleaseEvent(QMouseEvent* evt) override;

    /**
     * @see QWidget::mouseMoveEvent
     */
    void mouseMoveEvent(QMouseEvent* evt) override;

    /**
     * @see QWidget::mouseWheelEvent
     */
    void wheelEvent(QWheelEvent* evt) override;

    /**
     * @see QWidget::keyPressEvent
     */
    void keyPressEvent(QKeyEvent* evt) override;

    /**
     * @see QWidget::keyReleaseEvent
     */
    void keyReleaseEvent(QKeyEvent* evt) override;

    /**
     * @see QWidget::closeEvent
     */
    void closeEvent(QCloseEvent* evt) override;

private:

    bool _mouseDown = false;
};



#endif // MGPVIEW_H
