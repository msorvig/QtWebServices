#include <Cocoa/Cocoa.h>
#include <WebKit/WebView.h>

#include "qtwebview.h"
#include <QtCore/QString>

@interface QtFrameLoadDelegate : NSObject {
    QtWebView *qtWebView;
}
    - (QtFrameLoadDelegate *)initWithQtWebView:(QtWebView *)view;
@end

@implementation QtFrameLoadDelegate

- (QtFrameLoadDelegate *)initWithQtWebView:(QtWebView *)view;
{
    qtWebView = view;
    return self;
}

- (void)webView:(WebView *)sender didStartProvisionalLoadForFrame:(WebFrame *)frame
{
    emit qtWebView->loadStarted();
}

- (void)webView:(WebView *)sender didCommitLoadForFrame:(WebFrame *)frame
{

}

- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame
{
    emit qtWebView->loadFinished(true);
}

@end

QtWebView::QtWebView(QObject *parent)
:QObject(0)
{
    NSRect frame = NSMakeRect(0.0, 0.0, 400, 400);
    WebView *myWebView = [[WebView alloc] initWithFrame:frame frameName:@"Test Frame" groupName:nil];

    [myWebView setFrameLoadDelegate : [[QtFrameLoadDelegate alloc] initWithQtWebView : this]];

    webView = myWebView;
}

QtWebView::~QtWebView()
{
    [webView release];
}

void QtWebView::load(const QUrl &url)
{
    [[webView mainFrame] loadRequest:[NSURLRequest requestWithURL:url.toNSURL()]];
}

QString QtWebView::title() const
{
    return QString::fromNSString([webView mainFrameTitle]);
}

void *QtWebView::nativeView()
{
    return webView;
}
