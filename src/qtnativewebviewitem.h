#ifndef QTNATIVEWEBVIEWITEM_H
#define QTNATIVEWEBVIEWITEM_H

#include "qtwindowcontrolleritem.h"
#include "qtwebview.h"

#include <QtGui/QIcon>

class QtNativeWebViewItem : public QtWindowControllerItem
{
    Q_OBJECT
public:
    // QQuickWebEngineView API:
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QUrl icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingStateChanged)
//    Q_PROPERTY(int loadProgress READ loadProgress NOTIFY loadProgressChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY loadingStateChanged)
    Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY loadingStateChanged)
//    Q_PROPERTY(bool inspectable READ inspectable WRITE setInspectable)
//    Q_ENUMS(LoadStatus);
//    Q_ENUMS(ErrorDomain);

public:
    explicit QtNativeWebViewItem(QQuickItem *parent = 0);
    ~QtNativeWebViewItem();

    QUrl url() const;
    void setUrl(const QUrl&url);
    QUrl icon() const;
    bool isLoading() const;
    int loadProgress() const;
    QString title() const;
    bool canGoBack() const;
    bool canGoForward() const;
    bool inspectable() const;
    void setInspectable(bool);

    enum LoadStatus {
        LoadStartedStatus,
        LoadStoppedStatus,
        LoadSucceededStatus,
        LoadFailedStatus
    };
/*
    enum ErrorDomain {
         NoErrorDomain,
         InternalErrorDomain,
         ConnectionErrorDomain,
         CertificateErrorDomain,
         HttpErrorDomain,
         FtpErrorDomain,
         DnsErrorDomain
    };
*/
public Q_SLOTS:
    void goBack();
    void goForward();
    void reload();
    void stop();

Q_SIGNALS:
    void titleChanged();
    void urlChanged();
    void iconChanged();
    void loadingStateChanged(LoadStatus loadStatus);
    void loadProgressChanged();

private:
    QtWebView *webView;
};

#endif
