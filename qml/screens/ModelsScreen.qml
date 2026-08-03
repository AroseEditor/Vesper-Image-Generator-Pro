import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Vesper
import Vesper.theme
import Vesper.components

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.gapMedium
        spacing: Theme.gapMedium

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Theme.gapSmall

            ColumnLayout {
                spacing: 2

                Text {
                    text: "Models"
                    color: Theme.text
                    font.pixelSize: Theme.fontDisplay
                    font.weight: Font.DemiBold
                }

                Text {
                    text: ModelCatalog.installedCount + " of " + modelList.count + " installed. Stored in "
                          + ModelCatalog.modelsDirectory
                    color: Theme.textFaint
                    font.pixelSize: Theme.fontSmall
                    elide: Text.ElideMiddle
                }
            }

            Item { Layout.fillWidth: true }
        }

        Text {
            Layout.fillWidth: true
            Layout.leftMargin: Theme.gapSmall
            visible: ModelCatalog.error.length > 0
            text: ModelCatalog.error
            color: Theme.danger
            font.pixelSize: Theme.fontSmall
            wrapMode: Text.WordWrap
        }

        ListView {
            id: modelList
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: ModelCatalog
            spacing: Theme.gapSmall
            clip: true
            ScrollBar.vertical: ScrollBar { width: 6 }

            delegate: GlassPanel {
                id: card

                required property string modelId
                required property string name
                required property string family
                required property string sizeLabel
                required property string licenseName
                required property string licenseUrl
                required property string notes
                required property int state
                required property string stateLabel
                required property real progress
                required property string statusDetail
                required property int fileCount

                readonly property bool installed: state === 3
                readonly property bool busy: state === 1 || state === 2
                readonly property bool licenseOk: AppSettings.licenseAccepted(card.modelId)

                width: modelList.width - 8
                height: body.implicitHeight + Theme.gapLarge * 2
                elevated: card.busy

                ColumnLayout {
                    id: body
                    anchors.fill: parent
                    anchors.margins: Theme.gapLarge
                    spacing: Theme.gapSmall

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Theme.gapSmall

                        Text {
                            text: card.name
                            color: Theme.text
                            font.pixelSize: Theme.fontTitle
                            font.weight: Font.DemiBold
                        }

                        Rectangle {
                            implicitWidth: familyText.implicitWidth + 16
                            implicitHeight: 22
                            radius: 11
                            color: Qt.rgba(1, 1, 1, 0.08)

                            Text {
                                id: familyText
                                anchors.centerIn: parent
                                text: card.family
                                color: Theme.textMuted
                                font.pixelSize: Theme.fontMicro
                            }
                        }

                        Rectangle {
                            implicitWidth: sizeText.implicitWidth + 16
                            implicitHeight: 22
                            radius: 11
                            color: Qt.rgba(197, 138, 240, 0.18)

                            Text {
                                id: sizeText
                                anchors.centerIn: parent
                                text: card.sizeLabel
                                color: Theme.accent
                                font.pixelSize: Theme.fontMicro
                                font.weight: Font.DemiBold
                            }
                        }

                        Text {
                            visible: card.fileCount > 1
                            text: card.fileCount + " files"
                            color: Theme.textFaint
                            font.pixelSize: Theme.fontMicro
                        }

                        Item { Layout.fillWidth: true }

                        Rectangle {
                            visible: card.installed
                            implicitWidth: installedText.implicitWidth + 18
                            implicitHeight: 24
                            radius: 12
                            color: Qt.rgba(99, 211, 166, 0.16)
                            border.width: 1
                            border.color: Qt.rgba(99, 211, 166, 0.4)

                            Text {
                                id: installedText
                                anchors.centerIn: parent
                                text: "Installed"
                                color: Theme.positive
                                font.pixelSize: Theme.fontMicro
                                font.weight: Font.DemiBold
                            }
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: card.notes
                        color: Theme.textMuted
                        font.pixelSize: Theme.fontSmall
                        wrapMode: Text.WordWrap
                        lineHeight: 1.3
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        visible: !card.installed

                        CheckBox {
                            id: licenseCheck
                            checked: card.licenseOk
                            onToggled: AppSettings.setLicenseAccepted(card.modelId, checked)

                            indicator: Rectangle {
                                implicitWidth: 17
                                implicitHeight: 17
                                radius: 5
                                y: parent.height / 2 - height / 2
                                color: licenseCheck.checked ? Theme.accent : Qt.rgba(0, 0, 0, 0.25)
                                border.width: 1
                                border.color: licenseCheck.checked ? Theme.accent : Theme.glassEdge

                                Text {
                                    anchors.centerIn: parent
                                    text: "x"
                                    color: "#1a0f24"
                                    font.pixelSize: 11
                                    font.weight: Font.Bold
                                    visible: licenseCheck.checked
                                }
                            }

                            contentItem: Text {
                                leftPadding: licenseCheck.indicator.width + 8
                                text: "I accept the " + card.licenseName + " terms"
                                color: Theme.textMuted
                                font.pixelSize: Theme.fontSmall
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        Text {
                            text: "Read license"
                            color: Theme.accent
                            font.pixelSize: Theme.fontSmall
                            font.underline: licenseLink.hovered

                            HoverHandler { id: licenseLink }
                            TapHandler { onTapped: Qt.openUrlExternally(card.licenseUrl) }
                        }

                        Item { Layout.fillWidth: true }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5
                        visible: card.busy || card.statusDetail.length > 0

                        Rectangle {
                            Layout.fillWidth: true
                            height: 5
                            radius: 3
                            color: Qt.rgba(1, 1, 1, 0.08)
                            visible: card.busy

                            Rectangle {
                                width: parent.width * card.progress
                                height: parent.height
                                radius: parent.radius
                                gradient: Gradient {
                                    orientation: Gradient.Horizontal
                                    GradientStop { position: 0.0; color: Theme.accentDeep }
                                    GradientStop { position: 1.0; color: Theme.accent }
                                }
                                Behavior on width { NumberAnimation { duration: Theme.durationFast } }
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            text: card.statusDetail
                            color: card.state === 4 ? Theme.danger : Theme.textFaint
                            font.pixelSize: Theme.fontMicro
                            wrapMode: Text.WordWrap
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Theme.gapSmall

                        Item { Layout.fillWidth: true }

                        GlassButton {
                            text: "Cancel"
                            visible: card.busy
                            onClicked: ModelCatalog.cancelDownload()
                        }

                        GlassButton {
                            text: card.installed ? "Installed"
                                                 : (card.state === 4 ? "Retry" : "Download")
                            primary: !card.installed
                            implicitWidth: 128
                            visible: !card.busy
                            enabled: !card.installed && licenseCheck.checked
                                     && ModelCatalog.activeDownloadId.length === 0
                            onClicked: ModelCatalog.download(card.modelId)
                        }
                    }
                }
            }
        }
    }
}
