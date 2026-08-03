import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Vesper

Item {
    id: root

    property string pendingRemovalId: ""
    property string pendingRemovalName: ""
    property string pendingRemovalSize: ""

    ScrollView {
        anchors.fill: parent
        anchors.margins: Theme.gapMedium
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: root.width - Theme.gapMedium * 2
            spacing: Theme.gapMedium

            ColumnLayout {
                Layout.leftMargin: Theme.gapSmall
                spacing: 2

                Text {
                    text: "Settings"
                    color: Theme.text
                    font.pixelSize: Theme.fontDisplay
                    font.weight: Font.DemiBold
                }

                Text {
                    text: "Version " + AppSettings.version + ". Everything runs locally."
                    color: Theme.textFaint
                    font.pixelSize: Theme.fontSmall
                }
            }

            GlassPanel {
                Layout.fillWidth: true
                implicitHeight: engineColumn.implicitHeight + Theme.gapLarge * 2

                ColumnLayout {
                    id: engineColumn
                    anchors.fill: parent
                    anchors.margins: Theme.gapLarge
                    spacing: Theme.gapMedium

                    SectionLabel { text: "Engine" }

                    RowLayout {
                        Layout.fillWidth: true

                        Text {
                            Layout.fillWidth: true
                            text: InferenceBridge.engineAvailable
                                  ? InferenceBridge.enginePath
                                  : "sd-cli was not found next to the application"
                            color: InferenceBridge.engineAvailable ? Theme.textMuted : Theme.danger
                            font.pixelSize: Theme.fontSmall
                            elide: Text.ElideMiddle
                        }
                    }

                    GlassSlider {
                        Layout.fillWidth: true
                        label: "Threads"
                        from: 1
                        to: 32
                        stepSize: 1
                        value: AppSettings.threadCount
                        onMoved: function(v) { AppSettings.threadCount = Math.round(v) }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Theme.gapMedium

                        Text {
                            Layout.fillWidth: true
                            text: "Backend"
                            color: Theme.textMuted
                            font.pixelSize: Theme.fontSmall
                        }

                        GlassCombo {
                            Layout.preferredWidth: 180
                            model: ["auto", "cpu", "vulkan", "cuda", "metal"]
                            currentIndex: Math.max(0, model.indexOf(AppSettings.backend))
                            onActivated: AppSettings.backend = currentText
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Theme.gapMedium

                        Text {
                            Layout.fillWidth: true
                            text: "Offload weights to CPU between steps"
                            color: Theme.textMuted
                            font.pixelSize: Theme.fontSmall
                        }

                        Switch {
                            checked: AppSettings.offloadToCpu
                            onToggled: AppSettings.offloadToCpu = checked
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Theme.gapMedium

                        Text {
                            Layout.fillWidth: true
                            text: "Flash attention"
                            color: Theme.textMuted
                            font.pixelSize: Theme.fontSmall
                        }

                        Switch {
                            checked: AppSettings.flashAttention
                            onToggled: AppSettings.flashAttention = checked
                        }
                    }
                }
            }

            GlassPanel {
                Layout.fillWidth: true
                implicitHeight: storageColumn.implicitHeight + Theme.gapLarge * 2

                ColumnLayout {
                    id: storageColumn
                    anchors.fill: parent
                    anchors.margins: Theme.gapLarge
                    spacing: Theme.gapSmall

                    SectionLabel { text: "Installed models" }

                    Text {
                        Layout.fillWidth: true
                        text: ModelCatalog.modelsDirectory
                        color: Theme.textFaint
                        font.pixelSize: Theme.fontMicro
                        elide: Text.ElideMiddle
                    }

                    Text {
                        Layout.fillWidth: true
                        Layout.topMargin: Theme.gapSmall
                        visible: ModelCatalog.installedCount === 0
                        text: "Nothing installed yet."
                        color: Theme.textFaint
                        font.pixelSize: Theme.fontSmall
                    }

                    Repeater {
                        model: ModelCatalog

                        delegate: RowLayout {
                            id: installedRow

                            required property var model

                            readonly property string rowModelId: model.modelId
                            readonly property string rowName: model.name

                            Layout.fillWidth: true
                            visible: model.state === 3
                            spacing: Theme.gapMedium

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 1

                                Text {
                                    text: installedRow.rowName
                                    color: Theme.text
                                    font.pixelSize: Theme.fontBody
                                }

                                Text {
                                    text: ModelCatalog.installedSizeLabel(installedRow.rowModelId)
                                          + " on disk"
                                    color: Theme.textFaint
                                    font.pixelSize: Theme.fontMicro
                                }
                            }

                            GlassButton {
                                text: "Delete"
                                destructive: true
                                implicitWidth: 92
                                implicitHeight: 32
                                onClicked: {
                                    root.pendingRemovalId = installedRow.rowModelId;
                                    root.pendingRemovalName = installedRow.rowName;
                                    root.pendingRemovalSize =
                                        ModelCatalog.installedSizeLabel(installedRow.rowModelId);
                                    removeDialog.open();
                                }
                            }
                        }
                    }
                }
            }

            GlassPanel {
                Layout.fillWidth: true
                implicitHeight: privacyColumn.implicitHeight + Theme.gapLarge * 2

                ColumnLayout {
                    id: privacyColumn
                    anchors.fill: parent
                    anchors.margins: Theme.gapLarge
                    spacing: Theme.gapSmall

                    SectionLabel { text: "Privacy" }

                    Text {
                        Layout.fillWidth: true
                        text: "Vesper only reaches the network while downloading a model. There is no "
                              + "telemetry, no crash reporting, no update check, and no analytics. "
                              + "Generation runs entirely on this machine."
                        color: Theme.textMuted
                        font.pixelSize: Theme.fontSmall
                        wrapMode: Text.WordWrap
                        lineHeight: 1.4
                    }
                }
            }

            Item { Layout.preferredHeight: Theme.gapMedium }
        }
    }

    ConfirmDialog {
        id: removeDialog
        parent: Overlay.overlay
        heading: "Delete " + root.pendingRemovalName + "?"
        body: "This frees " + root.pendingRemovalSize
              + " of disk space. You can download the model again later."
        confirmText: "Delete"
        destructive: true
        onAccepted: ModelCatalog.remove(root.pendingRemovalId)
    }
}
