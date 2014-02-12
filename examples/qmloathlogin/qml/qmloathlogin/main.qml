import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

import QtNativeWebView 1.0

Rectangle {
    width: 360
    height: 360

    ColumnLayout {
        id: buttons
        Button { text : "Toggle Visibility"; onClicked : { webView.visible = !webView.visible; } }
        Button { text : "Move"; onClicked : { webView.y = webView.y + 20; } }
        Button { text : "Embiggen"; onClicked : { webView.height += 20; } }
        Item { Layout.fillHeight: true }
        anchors.left: parent.left
        height: parent.height
        width: 120
    }

    NativeWebView {
        id: webView
        url : "http://news.ycombinator.com"
        anchors.left: buttons.right
        height: parent.height - 200
        width: parent.width - 100
    }
}
