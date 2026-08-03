import Vesper
import QtQuick
import QtQuick.Controls.Basic

Item {
    id: root

    property alias text: input.text
    property alias placeholder: input.placeholderText
    property bool multiline: false
    property int rows: 4
    property alias readOnly: input.readOnly

    signal edited()

    implicitHeight: multiline ? (rows * 20 + 22) : 38

    Rectangle {
        anchors.fill: parent
        radius: Theme.radiusSmall
        color: Qt.rgba(0, 0, 0, 0.22)
        border.width: 1
        border.color: input.activeFocus ? Theme.accent : Theme.glassEdgeSoft

        Behavior on border.color { ColorAnimation { duration: Theme.durationFast } }
    }

    Flickable {
        anchors.fill: parent
        anchors.margins: 10
        contentWidth: width
        contentHeight: input.implicitHeight
        clip: true
        interactive: root.multiline
        boundsBehavior: Flickable.StopAtBounds

        TextArea.flickable: TextArea {
            id: input
            width: root.width - 20
            wrapMode: root.multiline ? TextEdit.Wrap : TextEdit.NoWrap
            color: Theme.text
            placeholderTextColor: Theme.textFaint
            font.pixelSize: Theme.fontBody
            selectionColor: Theme.accentDeep
            selectedTextColor: Theme.text
            padding: 0
            background: null
            onTextChanged: root.edited()

            Keys.onReturnPressed: function(event) {
                if (!root.multiline) {
                    event.accepted = true;
                } else {
                    event.accepted = false;
                }
            }
        }

        ScrollBar.vertical: ScrollBar { visible: root.multiline; width: 6 }
    }
}
