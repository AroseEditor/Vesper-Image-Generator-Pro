import Vesper
import QtQuick
import QtQuick.Controls.Basic

Dialog {
    id: root

    property string heading: ""
    property string body: ""
    property string confirmText: "Confirm"
    property bool destructive: false

    modal: true
    anchors.centerIn: parent
    width: 420
    padding: Theme.gapLarge
    closePolicy: Popup.CloseOnEscape

    background: Rectangle {
        radius: Theme.radiusLarge
        color: "#1b1729"
        border.width: 1
        border.color: Theme.glassEdge
    }

    Overlay.modal: Rectangle { color: Qt.rgba(0, 0, 0, 0.55) }

    contentItem: Column {
        spacing: Theme.gapMedium

        Text {
            width: parent.width
            text: root.heading
            color: Theme.text
            font.pixelSize: Theme.fontTitle
            font.weight: Font.DemiBold
            wrapMode: Text.WordWrap
        }

        Text {
            width: parent.width
            text: root.body
            color: Theme.textMuted
            font.pixelSize: Theme.fontBody
            wrapMode: Text.WordWrap
            lineHeight: 1.35
        }

        Row {
            anchors.right: parent.right
            spacing: Theme.gapSmall

            GlassButton {
                text: "Cancel"
                onClicked: root.reject()
            }

            GlassButton {
                text: root.confirmText
                primary: true
                destructive: root.destructive
                onClicked: root.accept()
            }
        }
    }
}
