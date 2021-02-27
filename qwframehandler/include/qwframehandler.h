#ifndef QWFRAMEHANDLER_H
#define QWFRAMEHANDLER_H

#include <QObject>
#include <QVector>
#include <QByteArray>
#include <QHostAddress>
#include <QNetworkAddressEntry>

#include "common.h"

typedef QVector<quint16> QVector_u16;

/*!
 * @class QwFrameHandler
 * @brief Обработчик кадров взаимодейтсвия qtcore и wtgui
 * @details По получению кадра вызывается @ref QwFrameHandler::parse , который
 * 		эмитит сигналы по обнаружению соотвествующих кадров. Для формирования
 * 		кадров используются статические методы
*/
class QwFrameHandler : public QObject
{
	Q_OBJECT
public:
	struct DeviceNetParams {
		int ipMode;
		QHostAddress gw;
		QNetworkAddressEntry localAddr;
		//
		DeviceNetParams() { ipMode = 0; }
		DeviceNetParams(const DeviceNetParams &prm) { *this = prm; }
		~DeviceNetParams() = default;
		void operator=(const DeviceNetParams &prm) {
			localAddr.setIp(prm.localAddr.ip());
			localAddr.setNetmask(prm.localAddr.netmask());
			gw = prm.gw;
			ipMode = prm.ipMode;
		}
		bool operator==(const DeviceNetParams &prm) {
			return gw == prm.gw && localAddr == prm.localAddr && ipMode == prm.ipMode;
		}
	};
	struct ServerAddr {
		enum Slot { Vpn, Device };
		Slot slot;
		int port;
		QHostAddress addr;
		//
		ServerAddr() { slot = Slot::Vpn; port = 0; }
		ServerAddr(const ServerAddr &s) { *this = s; }
		~ServerAddr() = default;
		void operator=(const ServerAddr &s) { slot = s.slot; port = s.port; addr = s.addr; }
		bool operator==(const ServerAddr &s) { return slot == s.slot && port == s.port && addr == s.addr; }
	};
	struct AllData {
		ServStruct::RadioParams rp;
		DeviceNetParams net;
		ServerAddr servAddr;
		QHostAddress vpnAddr;
		bool vpnEnabled;
	};

	QwFrameHandler(QObject *parent = nullptr) : QObject(parent) {
		qRegisterMetaType<QwFrameHandler::DeviceNetParams>("QwFrameHandler::DeviceNetParams");
		qRegisterMetaType<QwFrameHandler::ServerAddr>("QwFrameHandler::ServerAddr");
		qRegisterMetaType<QwFrameHandler::AllData>("QwFrameHandler::AllData");
		qRegisterMetaType<ServStruct::RadioParams>("ServStruct::RadioParams");
	}
	virtual ~QwFrameHandler() = default;

	static QByteArray getAllDataRequest();
	static QByteArray getAllDataResponse(const AllData &d);

	static QByteArray getRadioPrmsResponse(const ServStruct::RadioParams &d);
	static QByteArray getNetPrmsResponse(const DeviceNetParams &d);
	static QByteArray getServAddrResponse(const ServerAddr &d);
	static QByteArray getVpnAddrResponse(const QHostAddress &d);
	static QByteArray getVpnCfgResponse(const QString &d);
	static QByteArray getVpnEnableResponse(bool d);

	void parse(const QByteArray &data, void *sender = nullptr);

signals:
	void allDataRequestFound(void *sender);
	void allDataResponseFound(const AllData &d, void *sender);

	void radioPrmsResponseFound(const ServStruct::RadioParams &d, void *sender);
	void netPrmsResponseFound(const DeviceNetParams &d, void *sender);
	void servAddrResponseFound(const ServerAddr &d, void *sender);
	void vpnAddrResponseFound(const QHostAddress &d, void *sender);
	void vpnCfgResponseFound(const QString &d, void *sender);
	void vpnEnableResponseFound(bool d, void *sender);

protected:
};

#endif /* QWFRAMEHANDLER_H */
