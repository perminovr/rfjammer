#ifndef CTLSERVER_H
#define CTLSERVER_H

#include <QTcpServer>
#include <QLocalServer>
#include <QTcpSocket>
#include <QLocalSocket>
#include <QLinkedList>
#include <QMap>
#include "prgdatabase.h"
#include "framehandler.h"
#include "qwframehandler.h"

/*!
 * @class CtlServer
 * @brief Сервер управления программой
 * @details Предоставляет доступ к параметрам программы со стороны пользователя и wtgui.
 * Пользователь взаимодействует с программой через сервер по tcp каналу
 * wtgui взаимодействует с программой через сервер по tcp и локальному каналам
 * Сервер локального канала поднимается в констркторе класса
 * Сервер tcp канала поднимается через @ref CtlServer::start
*/
class CtlServer : public QObject
{
	Q_OBJECT
public:
	CtlServer(const PrgDatabase *db, QObject *parent = nullptr);
	virtual ~CtlServer();

public slots:
	void start(const QHostAddress &addr, quint16 port);
	void stop();
	void restart() { start(tcp.addr, tcp.port); }

	void onDevicesStatReceived(bool valid, const DevStruct::DeviceStat &d);
	void onDevicesListReceived(const QVector_u16 &ids);

signals:
	void radioPrmsChangeRequested(const ServStruct::RadioParams &d);
	void netPrmsChangeRequested(const QwFrameHandler::DeviceNetParams &d);
	void servAddrChangeRequested(const QwFrameHandler::ServerAddr &d);
	void vpnCfgChangeRequested(const QString &d);
	void vpnStateChangeRequested(bool d);
	//
	void devStatRequested(quint16 id);
	void ctlChangeRequested(bool valid, const DevStruct::DeviceCtl &d);
	void subscribtionsListChanged(const QVector_u16 &ids);
	void devicesListRequested();

public:
	/*!
	* @class TcpClient
	* @brief Данные tcp клиента, управляющего программой
	*/
	struct TcpClient {
		QMap <quint16, bool> subList; //!< список подписок
		QMap <int, bool> tempSubList; //!< единичные подписки на события (запросы) ( id >= 0 ; cmd < 0 )
		QTcpSocket *socket;
		TcpClient() { socket = nullptr; }
		~TcpClient() = default;
		TcpClient(const TcpClient &c) { *this = c; }
		void operator=(const TcpClient &c) { subList = c.subList; tempSubList = c.tempSubList; socket = c.socket; }
		bool operator==(const TcpClient &c) { return subList == c.subList && tempSubList == c.tempSubList && socket == c.socket; }
		bool operator==(const QTcpSocket *s) { return socket == s; }
	};

private:
	const PrgDatabase *db; // for read-only purposes
	struct {
		QTcpServer *socket;
		FrameHandler *handler;
		QHostAddress addr;
		quint16 port;
		QLinkedList <TcpClient> clients;
	} tcp;
	struct {
		QLocalServer *socket;
		QwFrameHandler *handler;
		QLinkedList <QLocalSocket *> clients;
	} pipe;

	void handleSubscriptionsList();
};

#endif /* CTLSERVER_H */
