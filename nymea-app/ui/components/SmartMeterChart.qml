/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
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

import QtQuick 2.8
import QtQuick.Controls 2.1
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts 1.1
import QtCharts 2.2
import Nymea 1.0

ChartView {
    id: root
    backgroundColor: Style.tileBackgroundColor
    backgroundRoundness: Style.cornerRadius
    theme: ChartView.ChartThemeLight
    legend.labelColor: Style.foregroundColor
    legend.font.pixelSize: app.smallFont
    legend.alignment: Qt.AlignRight
    titleColor: Style.foregroundColor

    property Thing rootMeter: null
    property ThingsProxy meters: null
    property int multiplier: 1
    property string stateName: "totalEnergyConsumed"

    readonly property State rootMeterTotalEnergyState: rootMeter ? rootMeter.stateByName(stateName) : null

    Connections {
        target: meters
        onCountChanged: root.refresh()
    }

    Component.onCompleted: {
        root.refresh()
    }

    QtObject {
        id: d
        property var sliceMap: {}
    }

    function refresh() {
        pieSeries.clear();
        d.sliceMap = {}

        var unknownEnergy = rootMeterTotalEnergyState ? rootMeterTotalEnergyState.value : 0

        for (var i = 0; i < meters.count; i++) {
            var thing = meters.get(i);
            var value = 0;
            var energyState = thing.stateByName(root.stateName)
            if (energyState) {
                value += energyState.value
            }
            var slice = pieSeries.append(thing.name, Math.max(0, value))
            var color = Style.accentColor
            for (var j = 0; j <= i; j+=2) {
                if (i % 2 == 0) {
                    color = Qt.lighter(color, 1.2);
                } else {
                    color = Qt.darker(color, 1.2)
                }
            }
            slice.color = color
            d.sliceMap[slice] = thing
            unknownEnergy -= value
        }

        if (unknownEnergy > 0) {
            var slice = pieSeries.append(qsTr("Unknown"), unknownEnergy)
            slice.color = Style.accentColor
            d.sliceMap[slice] = rootMeter
        }
    }

    PieSeries {
        id: pieSeries
        holeSize: 0.6
        size: 0.8

        onClicked: {
            print("clicked slice", slice, d.sliceMap[slice], d.sliceMap[slice].name)
            pageStack.push("../devicepages/SmartMeterDevicePage.qml", {thing: d.sliceMap[slice]})
        }
    }

    ColumnLayout {
        x: root.plotArea.x + (root.plotArea.width * 0.5) - (width / 2)
        y: root.plotArea.y + (root.plotArea.height * 0.5) - (height / 2)
        width: root.width

        Label {
            font.pixelSize: app.largeFont
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: root.rootMeterTotalEnergyState
                  ? root.rootMeterTotalEnergyState.value.toFixed(2)
                  : Math.round(pieSeries.sum * 1000) / 1000
        }

        Label {
            text: "KWh"
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }
    }
}

