import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.folderlistmodel 2.15
import Qt.labs.settings 1.0
import GHT60 1.0

ApplicationWindow {
    width: 640
    height: 480
    title: "报表导出"
    visible: true
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    modality: Qt.ApplicationModal

    color: "transparent"

    Rectangle {
        anchors.fill: parent
        anchors.margins: 16
        border.width: 1
        border.color: "black"
        radius: 5
        Image {
            source: "qrc:/img/close.png"
            anchors.verticalCenter: parent.top
            anchors.horizontalCenter: parent.right
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    close()
                }
            }
        }
    }

    Settings {
        id: setting
        fileName: "export_setting.ini"
        category: "DIR"
    }

    property var folderList: []
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 30
        GridLayout {
            id: _option
            Layout.alignment: Qt.AlignHCenter
            rows: 2
            columns: 2

            CheckBox {
                visible: false
                Layout.columnSpan: 2
                id: use_dac
                checked: true
                text: "优先使用DAC作为判定标准(如果无DAC则使用波门)"
            }

            RadioButton {
                id: override_gate
                checked: false
                text: "覆盖波门高度"
            }
            SpinBox {
                id: gate_value
                enabled: override_gate.checked
                from: 0
                to: 100
                value: 50
                editable: true
            }

            RadioButton {
                id: override_dac
                checked: false
                text: "DAC增益"
            }

            SpinBox {
                id: dac_value
                enabled: override_dac.checked
                from: -50
                to: 50
                value: 0
                editable: true
            }
        }

        Rectangle {
            width: 240
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter
            border.width: 1
            border.color: "lightblue"
            radius: 5
            clip: true
            ListView {
                id: listView
                clip: true
                anchors.fill: parent
                opacity: 1
                anchors.margins: 1
                model: FolderListModel {
                    showFiles: false
                    showDirs: true
                    folder: QSUtils.toAbsoluteUrl(setting.value("data", ""))
                }
                delegate: Rectangle {
                    height: 35
                    width: 240
                    RowLayout {
                        anchors.fill: parent
                        spacing: 0
                        CheckBox {
                            Layout.alignment: Qt.AlignCenter
                            Layout.preferredHeight: 35
                            Layout.preferredWidth: 35
                            onCheckedChanged: {
                                if (checked) {
                                    folderList.push(filePath)
                                } else {
                                    folderList = folderList.filter(item => {
                                                                       return item !== filePath
                                                                   })
                                }
                            }
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: fileName
                            }
                            MouseArea {
                                id: _ma
                                anchors.fill: parent
                                onClicked: {
                                    listView.currentIndex = index
                                }
                            }
                        }
                    }
                }
            }
        }

        Intr {
            id: intr
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Button {
                Layout.alignment: Qt.AlignHCenter
                width: 120
                height: 32
                text: "导出勾选"
                onClicked: {
                    if (intr.excelRender(folderList, use_dac.checked, override_gate.checked, gate_value, override_dac, dac_value)) {
                        toast.showSuccessful("导出成功")
                    }
                }
            }
            Button {
                Layout.alignment: Qt.AlignHCenter
                width: 120
                height: 32
                text: "导出当前"
                onClicked: {
                    let ret = false
                    if (typeof (SELECTED_FILE) != undefined) {
                        ret = intr.excelRender([SELECTED_FILE], use_dac.checked, override_gate.checked, gate_value, override_dac, dac_value)
                    } else {
                        ret = intr.excelRender(folderList, use_dac.checked, override_gate.checked, gate_value, override_dac, dac_value)
                    }
                    if (ret) {
                        toast.showSuccessful("导出成功")
                    }
                }
            }
        }
    }
    ToastManager {
        id: toast
    }
}
