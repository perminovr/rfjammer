#include "ctlclient.h"
#include <QDebug>
#ifdef __linux__
#include <sys/socket.h>
#endif


#define TEST_UNUSED_FUNC 0


void CtlClient::tcpRestart(const QwFrameHandler::ServerAddr &d)
{
	if (tcp.addr != d.addr || tcp.port != d.port) {
		prgDebug() << "------" << "tcpRestart" << d.addr << d.port;
		tcp.addr = d.addr;
		tcp.port = d.port;
		tcp.socket->close();
		restartTimer->start(1000);
	}
}


CtlClient::CtlClient(QObject *parent) : QObject(parent)
{
	this->restartTimer = new QTimer(this);
	{
		restartTimer->setSingleShot(true);
		connect(restartTimer, &QTimer::timeout, this, [this](){
            prgDebug() << tcp.addr << tcp.port;
			this->tcp.socket->connectToHost(tcp.addr, tcp.port);
		});
	}

	this->pipe.handler = new QwFrameHandler(this);
	{
		connect(pipe.handler, &QwFrameHandler::allDataResponseFound, this, [this](const QwFrameHandler::AllData &d, void *sender){
			prgDebug() << "------ CtlClient" << "QwFrameHandler::allDataResponseFound signal:";
			Q_UNUSED(sender)
			tcpRestart(d.servAddr);
			emit this->allDataChanged(d);
		});
		connect(pipe.handler, &QwFrameHandler::vpnAddrResponseFound, this, [this](const QHostAddress &d, void *sender){
			prgDebug() << "------ CtlClient" << "QwFrameHandler::vpnAddrResponseFound signal:";
			Q_UNUSED(sender)
			emit this->vpnAddrChanged(d);
		});
		connect(pipe.handler, &QwFrameHandler::servAddrResponseFound, this, [this](const QwFrameHandler::ServerAddr &d, void *sender){
			prgDebug() << "------ CtlClient" << "QwFrameHandler::servAddrResponseFound signal:";
			Q_UNUSED(sender)
			tcpRestart(d);
		});
	}

	this->pipe.socket = new QLocalSocket(this);
	{
		connect(pipe.socket, &QLocalSocket::connected, this, [this](){
			prgDebug() << "------ CtlClient" << "QLocalSocket::connected signal:";
			QByteArray ret = pipe.handler->getAllDataRequest();
			this->pipe.socket->write(ret);
		});
		connect(pipe.socket, &QLocalSocket::disconnected, this, [this](){
			prgDebug() << "------ CtlClient" << "QLocalSocket::disconnected signal:";
			Q_UNUSED(this)
			// abort(); // shutdown due host fell down todo
		});
		connect(pipe.socket, &QLocalSocket::readyRead, this, [this](){
			prgDebug() << "------ CtlClient" << "QLocalSocket::readyRead signal:";
			this->pipe.handler->parse(this->pipe.socket->readAll());
		});
	}

	this->tcp.handler = new FrameHandler(this);
	{
		connect(tcp.handler, &FrameHandler::statResponseFound, this, [this](bool valid, const DevStruct::DeviceStat &d, void *sender){
			// prgDebug() << "------ CtlClient" << "FrameHandler::statResponseFound signal:";
			Q_UNUSED(sender)
			emit this->deviceStatChanged(valid, d);
		});
		#if TEST_UNUSED_FUNC
			connect(tcp.handler, &FrameHandler::devListResponseFound, this, [this](const QVector_u16 &ids, void *sender){
				Q_UNUSED(sender)
				int sz = ids.size();
				prgDebug() << "------ CtlClient" << sz << "devices found";
				QVector_u16 ids_sub;
				for (int i = sz/4; i < sz*3/4; ++i) {
					ids_sub.push_back( ids[i] );
				}
				QByteArray ret = tcp.handler->getSubscribeResponse(true, ids_sub);
				this->tcp.socket->write(ret);
				prgDebug() << "------ TEST_UNUSED_FUNC" << "step 1 -- start";
				QTimer::singleShot(500, this, [this, ids_sub](){
					QByteArray ret = tcp.handler->getSubscribeResponse(false, ids_sub);
					this->tcp.socket->write(ret);
					prgDebug() << "------ TEST_UNUSED_FUNC" << "step 2";
					QTimer::singleShot(500, this, [this](){
						QVector_u16 all;
						QByteArray ret = tcp.handler->getSubscribeResponse(true, all);
						this->tcp.socket->write(ret);
						prgDebug() << "------ TEST_UNUSED_FUNC" << "step 3";
						QTimer::singleShot(500, this, [this](){
							QVector_u16 all;
							QByteArray ret = tcp.handler->getSubscribeResponse(false, all);
							this->tcp.socket->write(ret);
							prgDebug() << "------ TEST_UNUSED_FUNC" << "step 4";
							QTimer::singleShot(500, this, [this](){
								QVector_u16 all;
                                QByteArray ret = tcp.handler->getDataRequest(FrameHandler::DataType::Stat, (quint16)3);
								this->tcp.socket->write(ret);
								prgDebug() << "------ TEST_UNUSED_FUNC" << "step 5";
								QTimer::singleShot(1000, this, [this](){
									QVector_u16 all;
									QByteArray ret = tcp.handler->getSubscribeResponse(true, all);
									this->tcp.socket->write(ret);
									prgDebug() << "------ TEST_UNUSED_FUNC" << "step 6 -- end";
								});
							});
						});
					});
				});
			});
		#endif
	}

	this->tcp.socket = new QTcpSocket(this);
	{
		connect(tcp.socket, &QTcpSocket::connected, this, [this](){
			prgDebug() << "------ CtlClient" << "QTcpSocket::connected signal:";

            #ifdef __linux__
            auto fd = this->tcp.socket->socketDescriptor();
            struct linger lin = { .l_onoff = 1, .l_linger = 0 }; // rst instead fin
            ::setsockopt((int)fd, SOL_SOCKET, SO_LINGER, (const char *)&lin, sizeof(struct linger));
            #endif

            #if TEST_UNUSED_FUNC
                QVector_u16 all;
                QByteArray ret;
                #if TEST_UNUSED_FUNC
                    ret = tcp.handler->getDevListRequest();
                #else
                    ret = tcp.handler->getSubscribeResponse(true, all);
                #endif
                this->tcp.socket->write(ret);
            #endif
		});
		connect(tcp.socket, &QTcpSocket::disconnected, this, [this](){
			prgDebug() << "------ CtlClient" << "QTcpSocket::disconnected signal:";
			restartTimer->start(1000);
		});
		connect(tcp.socket, static_cast<void (QTcpSocket::*)(QTcpSocket::SocketError)>(&QTcpSocket::error), this, [this](QTcpSocket::SocketError error){
			prgDebug() << "------ CtlClient" << "QTcpSocket::error signal:";
			Q_UNUSED(error)
			tcp.socket->close();
			restartTimer->start(1000);
		});
		connect(tcp.socket, &QTcpSocket::readyRead, this, [this](){
			// prgDebug() << "------ CtlClient" << "QTcpSocket::readyRead signal:";
			this->tcp.handler->parse(this->tcp.socket->readAll(), this->tcp.socket);
		});
	}

    connect(this, &CtlClient::p_addDevice, this, [this](const quint16 &id){
        if (!subedIds.contains(id)) {
            subedIds.push_back(id);
            QByteArray ret = tcp.handler->getSubscribeResponse(true, subedIds);
            this->tcp.socket->write(ret);
        }
    });
	connect(this, &CtlClient::p_setDeviceCtl, this, [this](const DevStruct::DeviceCtl &d){
		QByteArray ret = this->tcp.handler->getCtlResponse(true, d);
		this->tcp.socket->write(ret);
	});
	connect(this, &CtlClient::p_setRadioPrm, this, [this](const ServStruct::RadioParams &d){
		QByteArray ret = this->pipe.handler->getRadioPrmsResponse(d);
		this->pipe.socket->write(ret);
	});
	connect(this, &CtlClient::p_setNetParam, this, [this](const QwFrameHandler::DeviceNetParams &d){
		QByteArray ret = this->pipe.handler->getNetPrmsResponse(d);
		this->pipe.socket->write(ret);
	});
	connect(this, &CtlClient::p_setServerAddr, this, [this](const QwFrameHandler::ServerAddr &d){
		QByteArray ret = this->pipe.handler->getServAddrResponse(d);
		this->pipe.socket->write(ret);
	});
	connect(this, &CtlClient::p_setVpnFileName, this, [this](const QString &d){
		QByteArray ret = this->pipe.handler->getVpnCfgResponse(d);
		this->pipe.socket->write(ret);
	});
	connect(this, &CtlClient::p_setVpnEnabled, this, [this](bool d){
		QByteArray ret = this->pipe.handler->getVpnEnableResponse(d);
		this->pipe.socket->write(ret);
	});

	this->pipe.socket->connectToServer(QStringLiteral(CONFIG_CTLPIPE_NAME), QIODevice::ReadWrite);
	prgDebug() << "------ CtlClient::CtlClient done";
}


CtlClient::~CtlClient()
{}

