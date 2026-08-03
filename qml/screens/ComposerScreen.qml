import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QtQuick.Dialogs
import Vesper

Item {
    id: root

    function loadBackground(path) {
        document.backgroundPath = path;
        if (document.count === 0)
            document.applyTemplate("letter", "", "");
    }

    CompositionDocument {
        id: document
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: Theme.gapMedium
        spacing: Theme.gapMedium

        GlassPanel {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Item {
                id: canvasArea
                anchors.fill: parent
                anchors.margins: Theme.gapMedium

                readonly property real aspect: document.backgroundHeight > 0
                                               ? document.backgroundWidth / document.backgroundHeight
                                               : 1.5
                readonly property real canvasWidth: Math.min(width, height * aspect)
                readonly property real canvasHeight: canvasWidth / aspect

                Rectangle {
                    id: canvas
                    width: canvasArea.canvasWidth
                    height: canvasArea.canvasHeight
                    anchors.centerIn: parent
                    color: "#ffffff"
                    radius: 4
                    clip: true

                    Image {
                        anchors.fill: parent
                        source: document.backgroundUrl
                        fillMode: Image.PreserveAspectCrop
                        asynchronous: true
                    }

                    Repeater {
                        model: document

                        delegate: Item {
                            id: layerItem

                            required property int index
                            required property var model

                            readonly property bool selected: document.selectedIndex === index

                            x: model.x * canvas.width
                            y: model.y * canvas.height
                            width: model.width * canvas.width
                            height: layerText.implicitHeight
                            rotation: model.rotation
                            opacity: model.opacity

                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: -5
                                color: "transparent"
                                border.width: layerItem.selected ? 1 : 0
                                border.color: Theme.accent
                                radius: 3
                                visible: layerItem.selected
                            }

                            Text {
                                id: layerText
                                width: parent.width
                                text: layerItem.model.text
                                color: layerItem.model.color
                                font.family: layerItem.model.fontFamily
                                font.pixelSize: Math.max(1, layerItem.model.fontSize * canvas.height)
                                font.bold: layerItem.model.bold
                                font.italic: layerItem.model.italic
                                lineHeight: layerItem.model.lineHeight
                                lineHeightMode: Text.ProportionalHeight
                                wrapMode: Text.WordWrap
                                horizontalAlignment: layerItem.model.alignment === "center"
                                                     ? Text.AlignHCenter
                                                     : (layerItem.model.alignment === "right"
                                                        ? Text.AlignRight : Text.AlignLeft)
                            }

                            TapHandler {
                                onTapped: document.selectedIndex = layerItem.index
                            }

                            DragHandler {
                                id: dragger
                                target: null
                                onActiveChanged: if (active) document.selectedIndex = layerItem.index
                                onCentroidChanged: {
                                    if (!active)
                                        return;
                                    const nx = (layerItem.model.x * canvas.width
                                                + centroid.position.x - centroid.pressPosition.x)
                                               / canvas.width;
                                    const ny = (layerItem.model.y * canvas.height
                                                + centroid.position.y - centroid.pressPosition.y)
                                               / canvas.height;
                                    document.moveLayer(layerItem.index,
                                                       Math.max(0, Math.min(0.98, nx)),
                                                       Math.max(0, Math.min(0.98, ny)));
                                }
                            }

                            Rectangle {
                                width: 10
                                height: 10
                                radius: 5
                                color: Theme.accent
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                                anchors.margins: -5
                                visible: layerItem.selected

                                DragHandler {
                                    target: null
                                    onCentroidChanged: {
                                        if (!active)
                                            return;
                                        const nw = (layerItem.model.width * canvas.width
                                                    + centroid.position.x - centroid.pressPosition.x)
                                                   / canvas.width;
                                        document.resizeLayer(layerItem.index,
                                                             Math.max(0.05, Math.min(1.0, nw)));
                                    }
                                }
                            }
                        }
                    }
                }

                Column {
                    anchors.centerIn: parent
                    spacing: Theme.gapSmall
                    visible: document.backgroundPath.length === 0
                    width: parent.width * 0.5

                    Text {
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        text: "No background loaded"
                        color: Theme.textMuted
                        font.pixelSize: Theme.fontTitle
                    }

                    Text {
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        text: "Pick an image from the Gallery, or open a saved composition."
                        color: Theme.textFaint
                        font.pixelSize: Theme.fontSmall
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

        GlassPanel {
            Layout.preferredWidth: 330
            Layout.fillHeight: true
            elevated: true

            ScrollView {
                anchors.fill: parent
                anchors.margins: Theme.gapLarge
                clip: true
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                ColumnLayout {
                    width: parent.parent.width - Theme.gapLarge * 2
                    spacing: Theme.gapMedium

                    SectionLabel { text: "Template" }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Repeater {
                            model: TemplateCatalog

                            GlassButton {
                                required property string templateId
                                required property string name

                                Layout.fillWidth: true
                                implicitWidth: 0
                                text: name
                                primary: document.templateId === templateId
                                onClicked: document.applyTemplate(templateId, "", "")
                            }
                        }
                    }

                    Rectangle { Layout.fillWidth: true; height: 1; color: Theme.glassEdgeSoft }

                    RowLayout {
                        Layout.fillWidth: true

                        SectionLabel { text: "Layers"; Layout.fillWidth: true }

                        GlassButton {
                            text: "Add"
                            implicitWidth: 64
                            implicitHeight: 28
                            onClicked: document.addLayer("New text")
                        }
                    }

                    Repeater {
                        model: document

                        delegate: Rectangle {
                            id: layerRow

                            required property int index
                            required property var model

                            Layout.fillWidth: true
                            implicitHeight: 32
                            radius: Theme.radiusSmall
                            color: document.selectedIndex === index ? Qt.rgba(1, 1, 1, 0.1)
                                                                    : "transparent"

                            Text {
                                anchors.left: parent.left
                                anchors.leftMargin: 10
                                anchors.right: removeButton.left
                                anchors.verticalCenter: parent.verticalCenter
                                text: String(layerRow.model.text).split("\n")[0]
                                color: Theme.text
                                font.pixelSize: Theme.fontSmall
                                elide: Text.ElideRight
                            }

                            Text {
                                id: removeButton
                                anchors.right: parent.right
                                anchors.rightMargin: 10
                                anchors.verticalCenter: parent.verticalCenter
                                text: "x"
                                color: Theme.textFaint
                                font.pixelSize: Theme.fontSmall

                                TapHandler { onTapped: document.removeLayer(layerRow.index) }
                            }

                            TapHandler { onTapped: document.selectedIndex = layerRow.index }
                        }
                    }

                    Rectangle { Layout.fillWidth: true; height: 1; color: Theme.glassEdgeSoft }

                    SectionLabel {
                        text: "Selected layer"
                        visible: document.selectedIndex >= 0
                    }

                    GlassField {
                        id: layerTextField
                        Layout.fillWidth: true
                        visible: document.selectedIndex >= 0
                        multiline: true
                        rows: 5
                        text: document.selectedIndex >= 0
                              ? (document.layerAt(document.selectedIndex).text || "") : ""
                        onEdited: {
                            if (document.selectedIndex >= 0)
                                document.setLayerProperty(document.selectedIndex, "text", text);
                        }

                        Connections {
                            target: document
                            function onSelectedIndexChanged() {
                                if (document.selectedIndex >= 0)
                                    layerTextField.text =
                                        document.layerAt(document.selectedIndex).text || "";
                            }
                        }
                    }

                    GlassCombo {
                        id: fontCombo
                        Layout.fillWidth: true
                        visible: document.selectedIndex >= 0
                        model: ["Georgia", "Segoe Script", "Arial", "Times New Roman",
                                "Palatino Linotype", "Courier New", "Verdana"]
                        onActivated: document.setLayerProperty(document.selectedIndex,
                                                               "fontFamily", currentText)
                    }

                    GlassSlider {
                        Layout.fillWidth: true
                        visible: document.selectedIndex >= 0
                        label: "Font size"
                        from: 0.01
                        to: 0.2
                        stepSize: 0.002
                        decimals: 3
                        value: document.selectedIndex >= 0
                               ? (document.layerAt(document.selectedIndex).fontSize || 0.04) : 0.04
                        onMoved: function(v) {
                            document.setLayerProperty(document.selectedIndex, "fontSize", v);
                        }
                    }

                    GlassSlider {
                        Layout.fillWidth: true
                        visible: document.selectedIndex >= 0
                        label: "Line height"
                        from: 0.8
                        to: 2.5
                        stepSize: 0.05
                        decimals: 2
                        value: document.selectedIndex >= 0
                               ? (document.layerAt(document.selectedIndex).lineHeight || 1.35) : 1.35
                        onMoved: function(v) {
                            document.setLayerProperty(document.selectedIndex, "lineHeight", v);
                        }
                    }

                    GlassCombo {
                        Layout.fillWidth: true
                        visible: document.selectedIndex >= 0
                        model: ["left", "center", "right"]
                        onActivated: document.setLayerProperty(document.selectedIndex,
                                                               "alignment", currentText)
                    }

                    Rectangle { Layout.fillWidth: true; height: 1; color: Theme.glassEdgeSoft }

                    SectionLabel { text: "Export" }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        GlassField {
                            id: exportWidth
                            Layout.fillWidth: true
                            text: String(Math.max(1024, document.backgroundWidth))
                            placeholder: "Width"
                        }

                        GlassField {
                            id: exportHeight
                            Layout.fillWidth: true
                            text: String(Math.max(704, document.backgroundHeight))
                            placeholder: "Height"
                        }
                    }

                    GlassButton {
                        Layout.fillWidth: true
                        text: "Export image"
                        primary: true
                        enabled: document.backgroundPath.length > 0
                        onClicked: exportPicker.open()
                    }

                    GlassButton {
                        Layout.fillWidth: true
                        text: "Save composition"
                        enabled: document.backgroundPath.length > 0
                        onClicked: projectSaver.open()
                    }

                    GlassButton {
                        Layout.fillWidth: true
                        text: "Open composition"
                        onClicked: projectLoader.open()
                    }

                    Item { Layout.preferredHeight: Theme.gapSmall }
                }
            }
        }
    }

    FileDialog {
        id: exportPicker
        title: "Export flattened image"
        fileMode: FileDialog.SaveFile
        nameFilters: ["PNG image (*.png)", "JPEG image (*.jpg)"]
        onAccepted: document.exportImage(selectedFile,
                                         parseInt(exportWidth.text, 10) || 0,
                                         parseInt(exportHeight.text, 10) || 0,
                                         95)
    }

    FileDialog {
        id: projectSaver
        title: "Save composition"
        fileMode: FileDialog.SaveFile
        nameFilters: ["Composition (*.composition.json)"]
        onAccepted: document.saveProject(selectedFile)
    }

    FileDialog {
        id: projectLoader
        title: "Open composition"
        nameFilters: ["Composition (*.composition.json)", "All files (*)"]
        onAccepted: document.loadProject(selectedFile)
    }
}
