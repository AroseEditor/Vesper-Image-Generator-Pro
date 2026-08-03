import QtQuick
import QtQuick.Controls.Basic
import Vesper.theme

Button {
    id: control

    property bool primary: false
    property bool destructive: false
    property color accentColor: destructive ? Theme.danger : Theme.accent

    implicitHeight: 38
    implicitWidth: Math.max(96, contentItem.implicitWidth + 34)
    padding: 0
    font.pixelSize: Theme.fontBody
    hoverEnabled: true

    background: Rectangle {
        radius: Theme.radiusSmall
        color: {
            if (!control.enabled)
                return Qt.rgba(1, 1, 1, 0.03);
            if (control.primary)
                return control.down ? Qt.darker(control.accentColor, 1.25)
                                    : (control.hovered ? Qt.lighter(control.accentColor, 1.08)
                                                       : control.accentColor);
            return control.down ? Qt.rgba(1, 1, 1, 0.16)
                                : (control.hovered ? Qt.rgba(1, 1, 1, 0.11) : Qt.rgba(1, 1, 1, 0.06));
        }
        border.width: 1
        border.color: control.primary ? Qt.rgba(1, 1, 1, 0.24)
                                      : (control.hovered ? Theme.glassEdge : Theme.glassEdgeSoft)

        Behavior on color { ColorAnimation { duration: Theme.durationFast } }

        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: parent.radius - 1
            visible: control.enabled
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.12) }
                GradientStop { position: 1.0; color: Qt.rgba(1, 1, 1, 0.0) }
            }
        }
    }

    contentItem: Text {
        text: control.text
        font: control.font
        color: {
            if (!control.enabled)
                return Theme.textFaint;
            return control.primary ? "#1a0f24" : Theme.text;
        }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    scale: down && enabled ? 0.975 : 1.0
    Behavior on scale { NumberAnimation { duration: Theme.durationFast; easing.type: Easing.OutQuad } }
}
