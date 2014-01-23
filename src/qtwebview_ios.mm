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
//    qDebug() << "webViewDidStartLoad";
    emit QtWebView->loadStarted();
}

- (void)webViewDidFinishLoad:(UIWebView *)webView
{
    Q_UNUSED(webView);
//    qDebug() << "webViewDidStartLoad";
    emit QtWebView->loadFinished(true);
}

- (void)webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error
{
    Q_UNUSED(webView);
//    qDebug() << "didFailLoadWithError";
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

void QtWebView::load(const QUrl &url)
{
    [(UIWebView*)webView loadRequest:[NSURLRequest requestWithURL:url.toNSURL()]];
}

QString QtWebView::title() const
{
    NSString *title = [webView stringByEvaluatingJavaScriptFromString:@"document.title"];
    return QString::fromNSString(title);
}

void *QtWebView::nativeView()
{
    return webView;
}
