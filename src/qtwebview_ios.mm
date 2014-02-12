#include <CoreFoundation/CoreFoundation.h>
#include <UIKit/UIKit.h>


#include "qtwebview.h"
#include <QtCore/QString>

@interface QtWebViewDelegate : NSObject {
    QtWebView *QtWebView;
}
- (QtWebViewDelegate *)initWithQtWebView:(QtWebView *)view;
- (void)webViewDidStartLoad:(UIWebView *)webView;
- (void)webViewDidFinishLoad:(UIWebView *)webView;
- (void)webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error;
@end

@implementation QtWebViewDelegate
- (QtWebViewDelegate *)initWithQtWebView:(QtWebView *)view
{
    QtWebView = view;
    return self;
}

- (void)webViewDidStartLoad:(UIWebView *)webView
{
    Q_UNUSED(webView);
    emit QtWebView->loadStarted();
}

- (void)webViewDidFinishLoad:(UIWebView *)webView
{
    Q_UNUSED(webView);
    emit QtWebView->loadFinished(true);
}

- (void)webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error
{
    Q_UNUSED(webView);
}

@end

QtWebView::QtWebView(QObject *parent)
    :QObject(parent)
{
    CGRect frame = CGRectMake(0.0, 0.0, 400, 400);
    UIWebView *w = [[UIWebView alloc] initWithFrame:frame];
    w.delegate = [[QtWebViewDelegate alloc] initWithQtWebView:this];
    webView = w;
}

QtWebView::~QtWebView()
{
    [webView release];
}

QUrl QtWebView::url() const
{
    qWarning() << "Not implemented" << __FUNCTION__;
    return QUrl();
}

void QtWebView::setUrl(const QUrl&url)
{
    [(UIWebView*)webView loadRequest:[NSURLRequest requestWithURL:url.toNSURL()]];
}

QImage QtWebView::icon() const
{
    qWarning() << "Not implemented" << __FUNCTION__;
    return QImage();
}

bool QtWebView::isLoading() const
{
    qWarning() << "Not implemented" << __FUNCTION__;
    return false;
}

QString QtWebView::title() const
{
    NSString *title = [webView stringByEvaluatingJavaScriptFromString:@"document.title"];
    return QString::fromNSString(title);
}

bool QtWebView::canGoBack() const
{
    qWarning() << "Not implemented" << __FUNCTION__;
    return false;
}

bool QtWebView::canGoForward() const
{
    qWarning() << "Not implemented" << __FUNCTION__;
    return false;
}

void QtWebView::goBack()
{
    qWarning() << "Not implemented" << __FUNCTION__;
}

void QtWebView::goForward()
{
    qWarning() << "Not implemented" << __FUNCTION__;
}

void QtWebView::reload()
{
    qWarning() << "Not implemented" << __FUNCTION__;
}

void QtWebView::stop()
{
    qWarning() << "Not implemented" << __FUNCTION__;
}

void QtWebView::load(const QUrl &url)
{
    setUrl(url);
}

void *QtWebView::nativeView()
{
    return webView;
}
