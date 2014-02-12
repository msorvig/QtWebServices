#include "qtnativewebviewitem.h"

QtNativeWebViewItem::QtNativeWebViewItem(QQuickItem *parent)
    :QtWindowControllerItem(parent)
{
    webView = new QtWebView(this);
    QtWindowControllerItem::setWindow(QWindow::fromWinId(WId(webView->nativeView())));
}

QtNativeWebViewItem::~QtNativeWebViewItem()
{

}

QUrl QtNativeWebViewItem::url() const
{
    return webView->url();
}

void QtNativeWebViewItem::setUrl(const QUrl&url)
{
    webView->setUrl(url);
}

QUrl QtNativeWebViewItem::icon() const
{
    //return webView->icon()->toQuickImageProvider;
    return QUrl();
}

bool QtNativeWebViewItem::isLoading() const
{
    return webView->isLoading();
}

QString QtNativeWebViewItem::title() const
{
    return webView->title();
}

bool QtNativeWebViewItem::canGoBack() const
{
    return webView->canGoBack();
}

bool QtNativeWebViewItem::canGoForward() const
{
    return webView->canGoForward();
}

void QtNativeWebViewItem::goBack()
{
    webView->goBack();
}

void QtNativeWebViewItem::goForward()
{
    webView->goForward();
}

void QtNativeWebViewItem::reload()
{
    webView->reload();
}

void QtNativeWebViewItem::stop()
{
   webView->stop();
}

