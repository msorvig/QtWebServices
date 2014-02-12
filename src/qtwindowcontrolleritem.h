#ifndef QTWINDOWCONTROLLERITEM_H
#define QTWINDOWCONTROLLERITEM_H

#include <QtQuick/QQuickItem>

class QtWindowControllerItem : public QQuickItem
{
    Q_OBJECT
public:
    explicit QtWindowControllerItem(QQuickItem *parent = 0);
    void setNativeWindow(WId windowId);
    void componentComplete();
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);

public slots:
    void onWindowChanged(QQuickWindow* window);
    void onVisibleChanged();

private:
    QWindow *m_controlledWindow;
};

#endif // QTWINDOWCONTROLLERITEM_H
