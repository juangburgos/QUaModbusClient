#ifndef QUAMODBUSQMLCONTEXT_H
#define QUAMODBUSQMLCONTEXT_H

#include <QUaServer>

#include <QUaModbusClientList>
#include <QUaModbusClient>
#include <QUaModbusTcpClient>
#include <QUaModbusRtuSerialClient>

#include <QUaModbusDataBlockList>
#include <QUaModbusDataBlock>

#include <QUaModbusValueList>
#include <QUaModbusValue>

#ifdef QUA_ACCESS_CONTROL
class QUaUser;
#endif // QUA_ACCESS_CONTROL

#include <QQmlEngine>
#include <QQmlContext>

class QUaModbusQmlContext;

/******************************************************************************************************
*/

class QUaModbusValueQmlContext : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString valueId READ valueId CONSTANT)
#ifdef QUA_ACCESS_CONTROL                                                                                    
     Q_PROPERTY(bool canWrite READ canWrite NOTIFY canWriteChanged)
#endif // QUA_ACCESS_CONTROL
public:
    explicit QUaModbusValueQmlContext(QObject* parent = nullptr);

    // QML API

    // QUaModbusValue
    QString valueId() const;

#ifdef QUA_ACCESS_CONTROL
    bool canWrite() const;
#endif // QUA_ACCESS_CONTROL

    // C++ API

    void bindValue(QUaModbusValue* value);

    void clear();

signals:

#ifdef QUA_ACCESS_CONTROL
    void canWriteChanged();
#endif // QUA_ACCESS_CONTROL


public slots:

private:
    QUaModbusValue* m_value;
#ifdef QUA_ACCESS_CONTROL
    bool m_canWrite;
    QUaUser* m_loggedUser;
#endif // QUA_ACCESS_CONTROL

};

/******************************************************************************************************
*/

class QUaModbusDataBlockQmlContext : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString              blockId      READ blockId      CONSTANT)
    Q_PROPERTY(QModbusDataBlockType type         READ type         WRITE setType         NOTIFY typeChanged)
    Q_PROPERTY(int                  address      READ address      WRITE setAddress      NOTIFY addressChanged)
    Q_PROPERTY(quint32              size         READ size         WRITE setSize         NOTIFY sizeChanged)
    Q_PROPERTY(quint32              samplingTime READ samplingTime WRITE setSamplingTime NOTIFY samplingTimeChanged)
    //Q_PROPERTY(QVector<quint16>     data         READ data         NOTIFY dataChanged)
    Q_PROPERTY(QModbusError         lastError    READ lastError    NOTIFY lastErrorChanged)
#ifdef QUA_ACCESS_CONTROL                                                                                    
    Q_PROPERTY(bool                 canWrite     READ canWrite     NOTIFY canWriteChanged)
#endif // QUA_ACCESS_CONTROL
    // QUaModbusValueList
    Q_PROPERTY(QVariantMap values      READ values      NOTIFY valuesChanged     )
    Q_PROPERTY(QVariant    valuesModel READ valuesModel NOTIFY valuesModelChanged)

public:
    explicit QUaModbusDataBlockQmlContext(QObject* parent = nullptr);

    // QML API

    // QUaModbusDataBlock
    QString blockId() const;

    QModbusDataBlockType type() const;
    void                 setType(const int& type);

    int  address() const;
    void setAddress(const int& address);

    quint32 size() const;
    void    setSize(const quint32& size);

    quint32 samplingTime() const;
    void    setSamplingTime(const quint32& samplingTime);

    //QVector<quint16> data() const;

    QModbusError lastError() const;

#ifdef QUA_ACCESS_CONTROL
    bool canWrite() const;
#endif // QUA_ACCESS_CONTROL

    // QUaModbusValueList
    QVariantMap values();
    QVariant    valuesModel();

    // C++ API

    void bindBlock(QUaModbusDataBlock* block);

    void clear();

#ifdef QUA_ACCESS_CONTROL
    QUaUser* loggedUser() const;
#endif // QUA_ACCESS_CONTROL

signals:
    void typeChanged();
    void addressChanged();
    void sizeChanged();
    void samplingTimeChanged();
    //void dataChanged();
    void lastErrorChanged();
#ifdef QUA_ACCESS_CONTROL
    void canWriteChanged();
#endif // QUA_ACCESS_CONTROL
    // QUaModbusValueList
    void valuesChanged();
    void valuesModelChanged();

public slots:

private:
    QUaModbusDataBlock* m_block;
#ifdef QUA_ACCESS_CONTROL
    bool m_canWrite;
    QUaUser* m_loggedUser;
#endif // QUA_ACCESS_CONTROL
    // QUaModbusValueList
    QList<QMetaObject::Connection> m_connections;
    QVariantMap m_values;
    void bindValues(QUaModbusValueList* values);
    void bindValue(QUaModbusValue* value);
    void addValue(QUaModbusValue* value);
    void removeValue(QUaModbusValue* value);
};

/******************************************************************************************************
*/

class QUaModbusClientQmlContext : public QObject
{
    Q_OBJECT
    // QUaModbusClient
    Q_PROPERTY(QString           clientId       READ clientId       CONSTANT)
    Q_PROPERTY(QModbusClientType type           READ type           CONSTANT)
    Q_PROPERTY(quint8            serverAddress  READ serverAddress  WRITE setServerAddress  NOTIFY serverAddressChanged )
    Q_PROPERTY(bool              keepConnecting READ keepConnecting WRITE setKeepConnecting NOTIFY keepConnectingChanged)
    Q_PROPERTY(QModbusState      state          READ state          NOTIFY stateChanged)
    Q_PROPERTY(QModbusError      lastError      READ lastError      NOTIFY lastErrorChanged)
#ifdef QUA_ACCESS_CONTROL                                                                                    
    Q_PROPERTY(bool              canWrite       READ canWrite       NOTIFY canWriteChanged)
#endif // QUA_ACCESS_CONTROL
    // QUaModbusTcpClient
    Q_PROPERTY(QString networkAddress  READ networkAddress WRITE setNetworkAddress NOTIFY networkAddressChanged)
    Q_PROPERTY(quint16 networkPort     READ networkPort    WRITE setNetworkPort    NOTIFY networkPortChanged   )
    // QUaModbusRtuSerialClient
    Q_PROPERTY(QString   comPort  READ comPort  WRITE setComPort  NOTIFY comPortChanged )
    Q_PROPERTY(QParity   parity   READ parity   WRITE setParity   NOTIFY parityChanged  )
    Q_PROPERTY(QBaudRate baudRate READ baudRate WRITE setBaudRate NOTIFY baudRateChanged)
    Q_PROPERTY(QDataBits dataBits READ dataBits WRITE setDataBits NOTIFY dataBitsChanged)
    Q_PROPERTY(QStopBits stopBits READ stopBits WRITE setStopBits NOTIFY stopBitsChanged)
    // QUaModbusDataBlockList
    Q_PROPERTY(QVariantMap blocks      READ blocks      NOTIFY blocksChanged     )
    Q_PROPERTY(QVariant    blocksModel READ blocksModel NOTIFY blocksModelChanged)
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

    // QUaModbusTcpClient
    QString networkAddress() const;
    void    setNetworkAddress(const QString& networkAddress);

    quint16 networkPort   () const;
    void    setNetworkPort(const quint16& networkPort);

    // QUaModbusRtuSerialClient

    QString   comPort() const;
    void      setComPort(const QString& strComPort);

    QParity   parity() const;
    void      setParity(const int& parity);

    QBaudRate baudRate() const;
    void      setBaudRate(const int& baudRate);

    QDataBits dataBits() const;
    void      setDataBits(const int& dataBits);

    QStopBits stopBits() const;
    void      setStopBits(const int& stopBits);

    // QUaModbusDataBlockList
    QVariantMap blocks();
    QVariant    blocksModel();

    // C++ API

    void bindClient(QUaModbusClient * client);

    void clear();

#ifdef QUA_ACCESS_CONTROL
    QUaUser* loggedUser() const;
#endif // QUA_ACCESS_CONTROL

signals:
    // QUaModbusClient
    void serverAddressChanged();
    void keepConnectingChanged();
    void stateChanged();
    void lastErrorChanged();
#ifdef QUA_ACCESS_CONTROL
    void canWriteChanged();
#endif // QUA_ACCESS_CONTROL
    // QUaModbusTcpClient
    void networkAddressChanged();
    void networkPortChanged   ();
    // QUaModbusRtuSerialClient
    void comPortChanged ();
    void parityChanged  ();
    void baudRateChanged();
    void dataBitsChanged();
    void stopBitsChanged();
    // QUaModbusDataBlockList
    void blocksChanged();
    void blocksModelChanged();

public slots:
    void connect();
    void disconnect();

private:
    QUaModbusClient* m_client;
#ifdef QUA_ACCESS_CONTROL
    bool m_canWrite;
    QUaUser* m_loggedUser;
#endif // QUA_ACCESS_CONTROL
    // QUaModbusDataBlockList
    QList<QMetaObject::Connection> m_connections;
    QVariantMap m_blocks;
    void bindBlocks(QUaModbusDataBlockList* blocks);
    void bindBlock(QUaModbusDataBlock* block);
    void addBlock(QUaModbusDataBlock* block);
    void removeBlock(QUaModbusDataBlock* block);
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
