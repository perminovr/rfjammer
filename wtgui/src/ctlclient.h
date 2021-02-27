#ifndef CTLCLIENT_H
#define CTLCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QLocalSocket>
#include <QTimer>
#include "framehandler.h"
#include "qwframehandler.h"

/*!
 * @class CtlClient
 * @brief Клиент управления основной программой
 * @details Предоставляет доступ к параметрам программы ( @ref CtlServer )
 * Выполняет автоподключение к tcp серверу по получению уведомления от локального сервера
*/
class CtlClient : public QObject
{
	Q_OBJECT
public:
	CtlClient(QObject *parent = nullptr);
	virtual ~CtlClient();

    inline void addDevice(const quint16 &id) { emit p_addDevice(id); }
	inline void setDeviceCtl(const DevStruct::DeviceCtl &d) { emit p_setDeviceCtl(d); }
	inline void setRadioPrm(const ServStruct::RadioParams &d) { emit p_setRadioPrm(d); }
	inline void setNetParam(const QwFrameHandler::DeviceNetParams &d) { emit p_setNetParam(d); }
	inline void setServerAddr(const QwFrameHandler::ServerAddr &d) { emit p_setServerAddr(d); }
	inline void setVpnFileName(const QString &d) { emit p_setVpnFileName(d); }
	inline void setVpnEnabled(bool d) { emit p_setVpnEnabled(d); }

signals:
	void allDataChanged(const QwFrameHandler::AllData &d); // starts tcp with this signal
	void vpnAddrChanged(const QHostAddress &d);
	void deviceStatChanged(bool valid, const DevStruct::DeviceStat &d);

    void p_addDevice(const quint16 &id);
	void p_setDeviceCtl(const DevStruct::DeviceCtl &d);
	void p_setRadioPrm(const ServStruct::RadioParams &d);
	void p_setNetParam(const QwFrameHandler::DeviceNetParams &d);
	void p_setServerAddr(const QwFrameHandler::ServerAddr &d);
	void p_setVpnFileName(const QString &d);
	void p_setVpnEnabled(bool d);

private:
	struct {
		QTcpSocket *socket;
		FrameHandler *handler;
		//
		QHostAddress addr;
		quint16 port;
	} tcp;
	struct {
		QLocalSocket *socket;
		QwFrameHandler *handler;
	} pipe;

    QVector_u16 subedIds;

	QTimer *restartTimer;
	void tcpRestart(const QwFrameHandler::ServerAddr &d);
};

#endif /* CTLCLIENT_H */
