import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QtQuick.Dialogs
import Vesper

Item {
    id: root

    signal editRequested(string path)

    property int selectedRow: -1

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
                    text: "Gallery"
                    color: Theme.text
                    font.pixelSize: Theme.fontDisplay
                    font.weight: Font.DemiBold
                }

                Text {
                    text: GalleryModel.count + " image" + (GalleryModel.count === 1 ? "" : "s")
                    color: Theme.textFaint
                    font.pixelSize: Theme.fontSmall
                }
            }

            Item { Layout.fillWidth: true }

            GlassButton {
                text: "Open folder"
                onClicked: GalleryModel.revealDirectory()
            }

            GlassButton {
                text: "Refresh"
                onClicked: GalleryModel.refresh()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.gapMedium

            GlassPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true

                GridView {
                    id: grid
                    anchors.fill: parent
                    anchors.margins: Theme.gapMedium
                    model: GalleryModel
                    cellWidth: 186
                    cellHeight: 186
                    clip: true
                    ScrollBar.vertical: ScrollBar { width: 6 }

                    delegate: Item {
                        required property string imageUrl
                        required property int index

                        width: grid.cellWidth - 10
                        height: grid.cellHeight - 10

                        Rectangle {
                            anchors.fill: parent
                            radius: Theme.radiusMedium
                            color: Qt.rgba(0, 0, 0, 0.25)
                            border.width: root.selectedRow === index ? 2 : 1
                            border.color: root.selectedRow === index ? Theme.accent : Theme.glassEdgeSoft
                            clip: true

                            Image {
                                anchors.fill: parent
                                anchors.margins: 3
                                source: imageUrl
                                fillMode: Image.PreserveAspectCrop
                                asynchronous: true
                                sourceSize.width: 360
                            }

                            Behavior on border.color { ColorAnimation { duration: Theme.durationFast } }
                        }

                        scale: hover.hovered ? 1.03 : 1.0
                        Behavior on scale { NumberAnimation { duration: Theme.durationFast; easing.type: Easing.OutQuad } }

                        HoverHandler { id: hover }
                        TapHandler { onTapped: root.selectedRow = index }
                    }
                }

                Text {
                    anchors.centerIn: parent
                    visible: GalleryModel.count === 0
                    text: "Generated images land here"
                    color: Theme.textFaint
                    font.pixelSize: Theme.fontTitle
                }
            }

            GlassPanel {
                id: detailPanel

                Layout.preferredWidth: 316
                Layout.fillHeight: true
                visible: root.selectedRow >= 0
                elevated: true

                property var details: root.selectedRow >= 0 ? GalleryModel.settingsAt(root.selectedRow)
                                                            : ({})

                ScrollView {
                    anchors.fill: parent
                    anchors.margins: Theme.gapLarge
                    clip: true
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                    ColumnLayout {
                        width: parent.parent.width - Theme.gapLarge * 2
                        spacing: Theme.gapMedium

                        SectionLabel { text: "Prompt" }

                        Text {
                            Layout.fillWidth: true
                            text: detailPanel.details.prompt || "No metadata recorded"
                            color: Theme.text
                            font.pixelSize: Theme.fontSmall
                            wrapMode: Text.WordWrap
                            lineHeight: 1.35
                        }

                        SectionLabel { text: "Settings" }

                        GridLayout {
                            Layout.fillWidth: true
                            columns: 2
                            rowSpacing: 6
                            columnSpacing: Theme.gapMedium

                            Repeater {
                                model: {
                                    const d = detailPanel.details;
                                    return [
                                        { key: "Model", value: d.modelId || "" },
                                        { key: "Seed", value: d.seed !== undefined ? String(d.seed) : "" },
                                        { key: "Sampler", value: d.sampler || "" },
                                        { key: "Steps", value: d.steps !== undefined ? String(d.steps) : "" },
                                        { key: "CFG", value: d.cfgScale !== undefined ? String(d.cfgScale) : "" },
                                        { key: "Size", value: d.width ? d.width + " x " + d.height : "" }
                                    ];
                                }

                                delegate: Item {
                                    required property var modelData
                                    Layout.fillWidth: true
                                    implicitHeight: 34

                                    Column {
                                        spacing: 1

                                        Text {
                                            text: modelData.key
                                            color: Theme.textFaint
                                            font.pixelSize: Theme.fontMicro
                                        }

                                        Text {
                                            text: modelData.value.length > 0 ? modelData.value : "-"
                                            color: Theme.text
                                            font.pixelSize: Theme.fontSmall
                                        }
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: Theme.glassEdgeSoft
                        }

                        GlassButton {
                            Layout.fillWidth: true
                            text: "Open in Composer"
                            primary: true
                            onClicked: {
                                const path = GalleryModel.data(
                                    GalleryModel.index(root.selectedRow, 0),
                                    Qt.UserRole + 1);
                                root.editRequested(path);
                            }
                        }

                        GlassButton {
                            Layout.fillWidth: true
                            text: "Save a copy"
                            onClicked: exportPicker.open()
                        }

                        GlassButton {
                            Layout.fillWidth: true
                            text: "Delete"
                            destructive: true
                            onClicked: deleteDialog.open()
                        }
                    }
                }
            }
        }
    }

    FileDialog {
        id: exportPicker
        title: "Save a copy"
        fileMode: FileDialog.SaveFile
        nameFilters: ["PNG image (*.png)", "JPEG image (*.jpg)"]
        onAccepted: GalleryModel.exportTo(root.selectedRow, selectedFile)
    }

    ConfirmDialog {
        id: deleteDialog
        parent: Overlay.overlay
        heading: "Delete this image?"
        body: "The image and its metadata sidecar are removed from disk. This cannot be undone."
        confirmText: "Delete"
        destructive: true
        onAccepted: {
            GalleryModel.removeAt(root.selectedRow);
            root.selectedRow = -1;
        }
    }
}
