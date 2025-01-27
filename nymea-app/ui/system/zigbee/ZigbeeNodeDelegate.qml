/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2022, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

import QtQuick 2.0
import Nymea 1.0
import QtQuick.Layouts 1.0
import "qrc:/ui/components"

ColumnLayout {
    id: root

    property ZigbeeManager zigbeeManager: null
    property ZigbeeNetwork zigbeeNetwork: null
    property ZigbeeNode node: null

    NymeaItemDelegate {
        id: thisNode
        Layout.fillWidth: true
        text: root.node.model + " - " + root.node.neighbors.length
    }

    Repeater {
        model: root.node.neighbors.length
        delegate: Text {
            Layout.fillWidth: true
            text: "fdsfdfasa, index" + index
        }
    }

    Repeater {
        model: root.node.neighbors.length
        delegate: Loader {
            id: loader
            Layout.fillWidth: true
            Layout.preferredHeight: item ? item.implicitHeight : 0
            source: Qt.resolvedUrl("ZigbeeNodeDelegate.qml")
//            ZigbeeNodeDelegate {
            Binding {
                target: loader.item
                property: "zigbeeManager"
                value: root.zigbeeManager
            }
            Binding {
                target: loader.item
                property: "zigbeeNetwork"
                value: root.zigbeeNetwork
            }

            Binding {
                target: loader.item
                property: "node"
                value: root.zigbeeNetwork.nodes.getNodeByNetworkAddress(root.node.neighbors[index].networkAddress)
            }
        }
    }
}
