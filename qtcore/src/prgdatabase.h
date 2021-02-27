#ifndef PRGDATABASE_H
#define PRGDATABASE_H

#include <QObject>
#include <QSettings>
#include "common.h"
#include "qwframehandler.h"

#undef Q_PROPERTY_IMPLEMENTATION
#define Q_PROPERTY_IMPLEMENTATION(type, name, getter, setter, notifier) \
	public slots: void setter(const type &t) { if (!(this->m_##name == t)) { this->m_##name = t; emit this->notifier(); emit this->smthChanged(); } } \
	public: type getter() const { return this->m_##name; } \
	Q_SIGNAL void notifier(); \
	protected: type m_##name;

#define PRGDB_DEVICE_SETTER(name, notifier)\
	int id = t.getM(id);\
	auto it = this->m_##name.find(id);\
	if (it != this->m_##name.end()) {\
		if (*it == t) { return; } else { *it = t; }\
	} else { it = this->m_##name.insert(id, t); }\
	emit this->notifier(id);\
	emit this->smthChanged();

#undef Q_PROPERTY_IMPLEMENTATION2
#define Q_PROPERTY_IMPLEMENTATION2(ktype, type, name, getter, setter, notifier) \
	public slots: void setter(const type &t) { PRGDB_DEVICE_SETTER(name, notifier) } \
	public: type getter(const ktype &t) const { auto it = this->m_##name.find(t); return (it != this->m_##name.end())? *it : type(); } \
	Q_SIGNAL void notifier(const ktype &t); \
	protected: QMap<ktype, type> m_##name;

/*!
 * @class PrgDatabase
 * @brief Выполняет функции хранения и передачи параметров программы между ее модулями
*/
class PrgDatabase : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString tunlName READ tunlName WRITE setTunlName NOTIFY tunlNameChanged)
	Q_PROPERTY(QString ethName READ ethName WRITE setEthName NOTIFY ethNameChanged)
	Q_PROPERTY(bool vpnEnabled READ vpnEnabled WRITE setVpnEnabled NOTIFY vpnEnabledChanged)
	Q_PROPERTY(QString vpnFileName READ vpnFileName WRITE setVpnFileName NOTIFY vpnFileNameChanged)
	Q_PROPERTY(QHostAddress vpnAddr READ vpnAddr WRITE setVpnAddr NOTIFY vpnAddrChanged)
	Q_PROPERTY(QwFrameHandler::DeviceNetParams netParam READ netParam WRITE setNetParam NOTIFY netParamChanged)
	Q_PROPERTY(QwFrameHandler::ServerAddr serverAddr READ serverAddr WRITE setServerAddr NOTIFY serverAddrChanged)
	Q_PROPERTY(ServStruct::RadioParams radioPrm READ radioPrm WRITE setRadioPrm NOTIFY radioPrmChanged)
	// Q_PROPERTY(DevStruct::DeviceCtl devicesCtl READ devicesCtl WRITE setDevicesCtl NOTIFY devicesCtlChanged)

	Q_PROPERTY_IMPLEMENTATION(QString , tunlName , tunlName , setTunlName , tunlNameChanged)
	Q_PROPERTY_IMPLEMENTATION(QString , ethName , ethName , setEthName , ethNameChanged)
	Q_PROPERTY_IMPLEMENTATION(bool , vpnEnabled , vpnEnabled , setVpnEnabled , vpnEnabledChanged)
	Q_PROPERTY_IMPLEMENTATION(QString , vpnFileName , vpnFileName , setVpnFileName , vpnFileNameChanged)
	Q_PROPERTY_IMPLEMENTATION(QHostAddress , vpnAddr , vpnAddr , setVpnAddr , vpnAddrChanged)
	Q_PROPERTY_IMPLEMENTATION(QwFrameHandler::DeviceNetParams , netParam , netParam , setNetParam , netParamChanged)
	Q_PROPERTY_IMPLEMENTATION(QwFrameHandler::ServerAddr , serverAddr , serverAddr , setServerAddr , serverAddrChanged)
	Q_PROPERTY_IMPLEMENTATION(ServStruct::RadioParams , radioPrm , radioPrm , setRadioPrm , radioPrmChanged)
	Q_PROPERTY_IMPLEMENTATION2(int , DevStruct::DeviceCtl , devicesCtl , devicesCtl , setDevicesCtl , devicesCtlChanged)

public:
	PrgDatabase(QObject *parent = nullptr);
	virtual ~PrgDatabase();

	void save();
	void readIfaces();
	void read();

	const QMap<int, DevStruct::DeviceCtl> &devicesCtlAll() const { return m_devicesCtl; }

signals:
	void smthChanged();

private:
	QSettings *settings;
};

#endif /* PRGDATABASE_H */
