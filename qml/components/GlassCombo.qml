import QtQuick
import QtQuick.Controls.Basic
import Vesper.theme

ComboBox {
    id: control

    implicitHeight: 38
    font.pixelSize: Theme.fontBody

    background: Rectangle {
        radius: Theme.radiusSmall
        color: control.pressed ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.22)
        border.width: 1
        border.color: control.activeFocus ? Theme.accent : Theme.glassEdgeSoft
        Behavior on border.color { ColorAnimation { duration: Theme.durationFast } }
    }

    contentItem: Text {
        leftPadding: 12
        rightPadding: 30
        text: control.displayText
        font: control.font
        color: Theme.text
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Canvas {
        x: control.width - width - 12
        y: control.height / 2 - 2
        width: 10
        height: 6
        contextType: "2d"

        onPaint: {
            const ctx = getContext("2d");
            ctx.reset();
            ctx.moveTo(0, 0);
            ctx.lineTo(width, 0);
            ctx.lineTo(width / 2, height);
            ctx.closePath();
            ctx.fillStyle = Theme.textMuted;
            ctx.fill();
        }
    }

    popup: Popup {
        y: control.height + 4
        width: control.width
        implicitHeight: Math.min(contentItem.implicitHeight + 12, 260)
        padding: 6

        background: Rectangle {
            radius: Theme.radiusSmall
            color: "#1a1626"
            border.width: 1
            border.color: Theme.glassEdge
        }

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            ScrollBar.vertical: ScrollBar { width: 6 }
        }
    }

    delegate: ItemDelegate {
        required property var model
        required property int index

        width: control.width - 12
        height: 32
        highlighted: control.highlightedIndex === index

        background: Rectangle {
            radius: Theme.radiusSmall - 3
            color: highlighted ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
        }

        contentItem: Text {
            text: model[control.textRole] === undefined ? model.modelData : model[control.textRole]
            color: Theme.text
            font.pixelSize: Theme.fontBody
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }
}
