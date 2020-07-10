#ifndef QUAMODBUSQMLCONTEXT_H
#define QUAMODBUSQMLCONTEXT_H

#include <QUaServer>

// TODO : modbus includes
#include <QUaModbusClientList>
#include <QUaModbusClient>

#ifdef QUA_ACCESS_CONTROL
class QUaUser;
#endif // QUA_ACCESS_CONTROL

#include <QQmlEngine>
#include <QQmlContext>


class QUaModbusQmlContext;

/******************************************************************************************************
*/

class QUaModbusClientQmlContext : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString           clientId       READ clientId       CONSTANT)
    Q_PROPERTY(QModbusClientType type           READ type           CONSTANT)
    Q_PROPERTY(quint8            serverAddress  READ serverAddress  WRITE setServerAddress  NOTIFY serverAddressChanged )
    Q_PROPERTY(bool              keepConnecting READ keepConnecting WRITE setKeepConnecting NOTIFY keepConnectingChanged)
    Q_PROPERTY(QModbusState      state          READ state          NOTIFY stateChanged)
    Q_PROPERTY(QModbusError      lastError      READ lastError      NOTIFY lastErrorChanged)
#ifdef QUA_ACCESS_CONTROL                                                                                    
    Q_PROPERTY(bool              canWrite       READ canWrite       NOTIFY canWriteChanged)
#endif // QUA_ACCESS_CONTROL
public:
    explicit QUaModbusClientQmlContext(QObject* parent = nullptr);

    // QML API

    // QUaModbusClient
    QString           clientId() const;
    QModbusClientType type() const;


    quint8 serverAddress() const;
    void   setServerAddress(const quint8& serverAddress);

    bool   keepConnecting() const;
    void   setKeepConnecting(const bool& keepConnecting);

    QModbusState state() const;
    QModbusError lastError() const;

#ifdef QUA_ACCESS_CONTROL
    bool canWrite() const;
#endif // QUA_ACCESS_CONTROL

    // TODO : tcp, serial

    // TODO : blocks, blocksModel

    // C++ API

    void bindClient(QUaModbusClient * client);

    void clear();

signals:
    void serverAddressChanged();
    void keepConnectingChanged();
    void stateChanged();
    void lastErrorChanged();
#ifdef QUA_ACCESS_CONTROL
    void canWriteChanged();
#endif // QUA_ACCESS_CONTROL

private:
    QUaModbusClient* m_client;
#ifdef QUA_ACCESS_CONTROL
    bool m_canWrite;
    QUaUser* m_loggedUser;
#endif // QUA_ACCESS_CONTROL

    QList<QMetaObject::Connection> m_connections;
};

/******************************************************************************************************
*/

class QUaModbusQmlContext : public QObject
{
    Q_OBJECT
    // NOTE : signals must start with lower case or "Cannot assign to non-existent property" error
    Q_PROPERTY(QVariantMap clients      READ clients      NOTIFY clientsChanged     )
    Q_PROPERTY(QVariant    clientsModel READ clientsModel NOTIFY clientsModelChanged)

public:
    explicit QUaModbusQmlContext(QObject *parent = nullptr);
    ~QUaModbusQmlContext();

    // QML API

    QVariantMap clients();
    QVariant    clientsModel();

    // C++ API

    void bindClients(QUaModbusClientList* clients);

    void clear();

#ifdef QUA_ACCESS_CONTROL
    QUaUser* loggedUser() const;
#endif // QUA_ACCESS_CONTROL

signals:
    void clientsChanged();
    void clientsModelChanged();

#ifdef QUA_ACCESS_CONTROL
    // NOTE : internal signal
    void loggedUserChanged(QPrivateSignal);
public slots:
    void on_loggedUserChanged(QUaUser* user);
#else
public slots:
#endif // QUA_ACCESS_CONTROL

private:
    QVariantMap                    m_clients;
    QList<QMetaObject::Connection> m_connections;

    void bindClient(QUaModbusClient* client);
    void addClient(QUaModbusClient* client);
    void removeClient(QUaModbusClient* client);

#ifdef QUA_ACCESS_CONTROL
    QUaUser* m_loggedUser;
#endif // QUA_ACCESS_CONTROL
};

#endif // QUAMODBUSQMLCONTEXT_H
