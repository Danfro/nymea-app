#include "pushnotifications.h"

#include <QDebug>

#if defined(Q_OS_ANDROID)
#include <QtAndroid>
#include <QtAndroidExtras>
#include <QAndroidJniObject>
static PushNotifications *m_client_pointer;
#endif

PushNotifications::PushNotifications(QObject *parent) : QObject(parent)
{
    connectClient();

#ifdef UBPORTS
    m_pushClient = new PushClient(this);
    m_pushClient->setAppId("io.guh.nymeaapp_nymea-app");
    connect(m_pushClient, &PushClient::tokenChanged, this, [this](const QString &token) {
        m_token = token;
        emit tokenChanged();
    });
#endif
}

QObject *PushNotifications::pushNotificationsProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return instance();
}

PushNotifications *PushNotifications::instance()
{
    static PushNotifications* pushNotifications = new PushNotifications();
    return pushNotifications;
}

void PushNotifications::connectClient()
{
#ifdef Q_OS_ANDROID
    m_firebaseApp = ::firebase::App::Create(::firebase::AppOptions(), QAndroidJniEnvironment(),
                                                QtAndroid::androidActivity().object());

    m_client_pointer = this;

    m_firebase_initializer.Initialize(m_firebaseApp, nullptr, [](::firebase::App * fapp, void *) {
        return ::firebase::messaging::Initialize( *fapp, (::firebase::messaging::Listener *)m_client_pointer);
    });

    while (m_firebase_initializer.InitializeLastResult().status() !=
            firebase::kFutureStatusComplete) {

        qDebug() << "Firebase: InitializeLastResult wait...";
    }
#endif
}

void PushNotifications::disconnectClient()
{
#ifdef Q_OS_ANDROID
    ::firebase::messaging::Terminate();
#endif
}

QString PushNotifications::token() const
{
    return m_token;
}

void PushNotifications::setAPNSRegistrationToken(const QString &apnsRegistrationToken)
{
    qDebug() << "Received APNS push notification token:" << apnsRegistrationToken;
    m_token = apnsRegistrationToken;
    emit tokenChanged();
}

#ifdef Q_OS_ANDROID
void PushNotifications::OnMessage(const firebase::messaging::Message &message)
{
    qDebug() << "Firebase message received:" << QString::fromStdString(message.from);
}

void PushNotifications::OnTokenReceived(const char *token)
{
    qDebug() << "Firebase token received:" << token;
    m_token = QString(token);
    emit tokenChanged();
}
#endif
