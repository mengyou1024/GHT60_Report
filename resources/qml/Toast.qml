import QtQuick 2.15

Rectangle {
    id: toast

    property real time: defaultTime
    readonly property real defaultTime: 1500
    readonly property real fadeTime: 400
    property string msgType: "Succed"

    property real margin: 10
    property bool selfDestroying: false

    implicitWidth: 98
    implicitHeight: 98
    radius: 4
    opacity: 0
    color: "#7F0066CC"

    anchors.horizontalCenter: parent.horizontalCenter

    function show(text, duration) {
        theText.text = text
        if (typeof duration !== "undefined") {
            if (duration >= 2 * fadeTime)
                time = duration
            else
                time = 2 * fadeTime
        } else
            time = defaultTime
        anim.start()
    }

    Image {
        id: imgHeader
        anchors.top: parent.top
        anchors.topMargin: 13
        anchors.horizontalCenter: parent.horizontalCenter
        sourceSize.width: 50
        sourceSize.height: 50
        source: "qrc:/img/" + msgType + ".png"
    }

    Text {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        width: parent.width
        id: theText
        text: ""
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 14
        color: "#000000"
    }

    SequentialAnimation on opacity {
        id: anim

        running: false

        NumberAnimation {
            to: 0.9
            duration: fadeTime
        }
        PauseAnimation {
            duration: time - 2 * fadeTime
        }
        NumberAnimation {
            to: 0
            duration: fadeTime
        }

        onRunningChanged: {
            if (!running && selfDestroying) {
                toast.destroy()
            }
        }
    }
}
