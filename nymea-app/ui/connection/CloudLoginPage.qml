import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Nymea 1.0
import "../components"

Page {
    id: root
    header: GuhHeader {
        text: qsTr("Cloud login")
        onBackPressed: pageStack.pop()
    }

    ColumnLayout {
        anchors { left: parent.left; top: parent.top; right: parent.right }
        visible: Engine.awsClient.isLoggedIn
        Label {
            Layout.fillWidth: true
            Layout.topMargin: app.margins
            Layout.leftMargin: app.margins
            Layout.rightMargin: app.margins
            wrapMode: Text.WordWrap
            text: qsTr("Logged in as %1").arg(Engine.awsClient.username)
        }

        Button {
            Layout.fillWidth: true
            Layout.margins: app.margins
            text: qsTr("Log out")
            onClicked: Engine.awsClient.logout();
        }

        ThinDivider {}

        Label {
            Layout.fillWidth: true
            Layout.topMargin: app.margins
            Layout.leftMargin: app.margins
            Layout.rightMargin: app.margins
            wrapMode: Text.WordWrap
            text: Engine.awsClient.awsDevices.count === 0 ?
                      qsTr("There are no boxes connected to your cloud yet.") :
                      qsTr("There (are|is) %1 boxe(s) connected to your cloud", "", Engine.awsClient.awsDevices.count)
        }
        Repeater {
            model: Engine.awsClient.awsDevices
            delegate: MeaListItemDelegate {
                Layout.fillWidth: true
                text: model.name
                subText: model.uuid
                progressive: false
                prominentSubText: false
                iconName: "../images/cloud.svg"
                secondaryIconName: model.online ? "../images/cloud.svg" : "../images/cloud-offline.svg"
            }
        }
    }

    ColumnLayout {
        anchors { left: parent.left; right: parent.right; top: parent.top }
        visible: !Engine.awsClient.isLoggedIn
        Label {
            Layout.fillWidth: true
            Layout.leftMargin: app.margins; Layout.rightMargin: app.margins; Layout.topMargin: app.margins
            text: "Username (e-mail)"
        }
        TextField {
            id: usernameTextField
            Layout.fillWidth: true
            Layout.leftMargin: app.margins; Layout.rightMargin: app.margins
            placeholderText: "john@dummy.com"
            inputMethodHints: Qt.ImhEmailCharactersOnly
        }
        Label {
            Layout.fillWidth: true
            Layout.leftMargin: app.margins; Layout.rightMargin: app.margins; Layout.topMargin: app.margins
            text: qsTr("Password")
        }
        TextField {
            id: passwordTextField
            Layout.leftMargin: app.margins; Layout.rightMargin: app.margins
            Layout.fillWidth: true
            echoMode: TextInput.Password
        }

        Button {
            Layout.fillWidth: true
            Layout.leftMargin: app.margins; Layout.rightMargin: app.margins; Layout.topMargin: app.margins
            text: qsTr("OK")
            enabled: usernameTextField.displayText.length > 0 && passwordTextField.displayText.length > 0
            onClicked:  {
                Engine.awsClient.login(usernameTextField.text, passwordTextField.text);
            }
        }
    }
}
