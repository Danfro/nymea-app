#ifndef SERVERCONFIGURATION_H
#define SERVERCONFIGURATION_H

#include <QObject>
#include <QHostAddress>
#include <QUuid>

class ServerConfiguration : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString address READ address WRITE setAddress NOTIFY addressChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(bool authenticationEnabled READ authenticationEnabled WRITE setAuthenticationEnabled NOTIFY authenticationEnabledChanged)
    Q_PROPERTY(bool sslEnabled READ sslEnabled WRITE setSslEnabled NOTIFY sslEnabledChanged)

public:
    explicit ServerConfiguration(const QString &id, const QHostAddress &address = QHostAddress(), int port = 0, bool authEnabled = false, bool sslEnabled = false, QObject *parent = nullptr);

    QString id() const;

    QString address() const;
    void setAddress(const QString &address);

    int port() const;
    void setPort(int port);

    bool authenticationEnabled() const;
    void setAuthenticationEnabled(bool authenticationEnabled);

    bool sslEnabled() const;
    void setSslEnabled(bool sslEnabled);

    Q_INVOKABLE ServerConfiguration* clone() const;

signals:
    void addressChanged();
    void portChanged();
    void authenticationEnabledChanged();
    void sslEnabledChanged();

private:
    QString m_id;
    QHostAddress m_hostAddress;
    int m_port;
    bool m_authEnabled;
    bool m_sslEnabled;
};

#endif // SERVERCONFIGURATION_H