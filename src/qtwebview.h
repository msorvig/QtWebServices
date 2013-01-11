#ifndef QTWEBVIEW_H
#define QTWEBVIEW_H

#include <QtCore/QtCore>

class QtWebView : public QObject
{
Q_OBJECT
public:
    QtWebView(QObject *parent = 0);
    void load(const QUrl &url);
    QString title() const;

    void *nativeView();
Q_SIGNALS:
    void loadStarted();
    void loadFinished(bool ok);
private:
    void *webView;
};

#endif
