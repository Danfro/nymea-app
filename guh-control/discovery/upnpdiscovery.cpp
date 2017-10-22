/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <stuerz.simon@gmail.com>               *
 *                                                                         *
 *  This file is part of guh-ubuntu.                                       *
 *                                                                         *
 *  guh-ubuntu is free software: you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 3 of the License.                *
 *                                                                         *
 *  guh-ubuntu is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the           *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh-ubuntu. If not, see <http://www.gnu.org/licenses/>.     *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "upnpdiscovery.h"

#include <QDebug>
#include <QUrl>
#include <QXmlStreamReader>

UpnpDiscovery::UpnpDiscovery(QObject *parent) :
    QUdpSocket(parent),
    m_discovering(false),
    m_available(false)
{
    m_networkAccessManager = new QNetworkAccessManager(this);
    connect(m_networkAccessManager, &QNetworkAccessManager::finished, this, &UpnpDiscovery::networkReplyFinished);

    m_discoveryModel = new DiscoveryModel(this);

    m_timer.setSingleShot(true);
    m_timer.setInterval(5000);
    connect(&m_timer, &QTimer::timeout, this, &UpnpDiscovery::onTimeout);
    m_repeatTimer.setInterval(500);
    connect(&m_repeatTimer, &QTimer::timeout, this, &UpnpDiscovery::writeDiscoveryPacket);

    // bind udp socket and join multicast group
    m_port = 1900;
    m_host = QHostAddress("239.255.255.250");

    setSocketOption(QAbstractSocket::MulticastTtlOption,QVariant(1));
    setSocketOption(QAbstractSocket::MulticastLoopbackOption,QVariant(1));

    if(!bind(QHostAddress::AnyIPv4, m_port, QUdpSocket::ShareAddress)){
        qWarning() << "UPnP discovery could not bind to port" << m_port;
        setAvailable(false);
        return;
    }

    if(!joinMulticastGroup(m_host)){
        qWarning() << "UPnP discovery could not join multicast group" << m_host;
        setAvailable(false);
        return;
    }

    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
    connect(this, &UpnpDiscovery::readyRead, this, &UpnpDiscovery::readData);
    setAvailable(true);
}

bool UpnpDiscovery::discovering() const
{
    return m_discovering;
}

DiscoveryModel *UpnpDiscovery::discoveryModel()
{
    return m_discoveryModel;
}

bool UpnpDiscovery::available() const
{
    return m_available;
}

void UpnpDiscovery::discover()
{
    if (!m_available) {
        qWarning() << "Could not discover. UPnP not available.";
        return;
    }

    qDebug() << "start discovering...";
    m_timer.start();
    m_repeatTimer.start();
//    m_discoveryModel->clearModel();
    m_foundDevices.clear();

    setDiscovering(true);

    writeDiscoveryPacket();
}

void UpnpDiscovery::stopDiscovery()
{
    qDebug() << "stop discovering";
    m_timer.stop();
    m_repeatTimer.stop();
    m_discoveryModel->clearModel();
    setDiscovering(false);
}

void UpnpDiscovery::setDiscovering(const bool &discovering)
{
    m_discovering = discovering;
    emit discoveringChanged();
}

void UpnpDiscovery::setAvailable(const bool &available)
{
    m_available = available;
    emit availableChanged();
}

void UpnpDiscovery::writeDiscoveryPacket()
{
    QByteArray ssdpSearchMessage = QByteArray("M-SEARCH * HTTP/1.1\r\n"
                                              "HOST:239.255.255.250:1900\r\n"
                                              "MAN:\"ssdp:discover\"\r\n"
                                              "MX:2\r\n"
                                              "ST: ssdp:all\r\n\r\n");

    writeDatagram(ssdpSearchMessage, m_host, m_port);
}

void UpnpDiscovery::error(QAbstractSocket::SocketError error)
{
    qWarning() << "UPnP socket error:" << error << errorString();
}

void UpnpDiscovery::readData()
{
    QByteArray data;
    quint16 port;
    QHostAddress hostAddress;

    // read the answere from the multicast
    while (hasPendingDatagrams()) {
        data.resize(pendingDatagramSize());
        readDatagram(data.data(), data.size(), &hostAddress, &port);
    }

    // if the data contains the HTTP OK header...
    if (data.contains("HTTP/1.1 200 OK")) {
        QUrl location;
        bool isGuh = false;

        const QStringList lines = QString(data).split("\r\n");
        foreach (const QString& line, lines) {
            int separatorIndex = line.indexOf(':');
            QString key = line.left(separatorIndex).toUpper();
            QString value = line.mid(separatorIndex+1).trimmed();


            if (key.contains("Server") || key.contains("SERVER")) {
                if (value.contains("guh")) {
                    qDebug() << " --> " << key << value;
                    isGuh = true;
                }
            }

            // get location
            if (key.contains("LOCATION") || key.contains("Location")) {
                location = QUrl(value);
            }
        }

        if (!m_foundDevices.contains(location) && isGuh) {
            m_foundDevices.append(location);
            DiscoveryDevice discoveryDevice;
            discoveryDevice.setHostAddress(hostAddress);
            discoveryDevice.setPort(port);
            discoveryDevice.setLocation(location.toString());

            qDebug() << "Getting server data from:" << location;
            QNetworkReply *reply = m_networkAccessManager->get(QNetworkRequest(location));
            connect(reply, &QNetworkReply::sslErrors, [this, reply](const QList<QSslError> &errors){
                reply->ignoreSslErrors(errors);
            });
            m_runningReplies.insert(reply, discoveryDevice);
        }
    }
}

void UpnpDiscovery::networkReplyFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    switch (status) {
    case(200):{
        QByteArray data = reply->readAll();
        DiscoveryDevice discoveryDevice = m_runningReplies.take(reply);

        // parse XML data
        QXmlStreamReader xml(data);
        while (!xml.atEnd() && !xml.hasError()) {
            xml.readNext();

            if (xml.isStartDocument())
                continue;

            if (xml.isStartElement()) {
                if (xml.name().toString() == "websocketURL") {
                    discoveryDevice.setWebSocketUrl(xml.readElementText());
                }
            }

            if (xml.isStartElement()) {
                if (xml.name().toString() == "guhRpcURL") {
                    discoveryDevice.setGuhRpcUrl(xml.readElementText());
                }
            }

            if (xml.isStartElement()) {
                if (xml.name().toString() == "device") {
                    while (!xml.atEnd()) {
                        if (xml.name() == "friendlyName" && xml.isStartElement()) {
                            discoveryDevice.setFriendlyName(xml.readElementText());
                        }
                        if (xml.name() == "manufacturer" && xml.isStartElement()) {
                            discoveryDevice.setManufacturer(xml.readElementText());
                        }
                        if (xml.name() == "manufacturerURL" && xml.isStartElement()) {
                            discoveryDevice.setManufacturerURL(QUrl(xml.readElementText()));
                        }
                        if (xml.name() == "modelDescription" && xml.isStartElement()) {
                            discoveryDevice.setModelDescription(xml.readElementText());
                        }
                        if (xml.name() == "modelName" && xml.isStartElement()) {
                            discoveryDevice.setModelName(xml.readElementText());
                        }
                        if (xml.name() == "modelNumber" && xml.isStartElement()) {
                            discoveryDevice.setModelNumber(xml.readElementText());
                        }
                        if (xml.name() == "modelURL" && xml.isStartElement()) {
                            discoveryDevice.setModelURL(QUrl(xml.readElementText()));
                        }
                        if (xml.name() == "UDN" && xml.isStartElement()) {
                            discoveryDevice.setUuid(xml.readElementText());
                        }
                        xml.readNext();
                    }
                    xml.readNext();
                }
            }
        }


        if (discoveryDevice.friendlyName().contains("guh")) {
            qDebug() << discoveryDevice;
            if (!m_discoveryModel->contains(discoveryDevice.uuid())) {
                m_discoveryModel->addDevice(discoveryDevice);
            }
        }

        break;
    }
    default:
        qWarning() << "HTTP request error " << status;
        m_runningReplies.remove(reply);
    }

    reply->deleteLater();
}

void UpnpDiscovery::onTimeout()
{
    qDebug() << "discovery timeout";
    m_repeatTimer.stop();
    setDiscovering(false);
}
