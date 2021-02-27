#ifndef QTWORKER_H
#define QTWORKER_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QProcess>
#include <QHostAddress>

class PrgDatabase;
class CtlServer;
class IVpnClient;
class RfJammersCtl;
class SystemNetworking;

class StatusDrv;

/*!
 * @class QtWorker
 * @brief Основной рабочий класс. Связывает все компоненты программы
*/
class QtWorker : public QObject
{
	Q_OBJECT
public:
	void start();

	QtWorker(QObject *parent = nullptr);
	virtual ~QtWorker();

signals:

private:
	SystemNetworking *networking;
	PrgDatabase *db;
	CtlServer *ctl;
	IVpnClient *vpn;
	RfJammersCtl *rfj;

	StatusDrv *status;

	struct {
		QProcess *proc;
		QTimer *timer;
	} gui;

	void restartCtlServer(const QHostAddress &addr, quint16 port, int slot);
};

#endif /* QTWORKER_H */
