#include "ctlserver.h"
#include <QVector>
#include <QDebug>


enum ClientEvents {
	GetDeviceList = INT32_MIN,
	SubscribeForDeviceList,
	_ce_unused_start = 0,
	_ce_unused_end = INT32_MAX,
};


void CtlServer::onDevicesStatReceived(bool valid, const DevStruct::DeviceStat &d)
{
	QByteArray ret = tcp.handler->getStatResponse(valid, d);
	for (auto &x : tcp.clients) {
		for (auto it = x.subList.begin(); it != x.subList.end(); it++) {
            if (it.value()) {
				if (d.getM(id) == scast(uint16_t, it.key())) {
					x.socket->write(ret);
				}
			}
		}
        QVector < decltype(x.tempSubList)::iterator > del;
		for (auto it = x.tempSubList.begin(); it != x.tempSubList.end(); it++) {
			if (it.value() && it.key() >= 0) {
				if (d.getM(id) == scast(uint16_t, it.key())) {
					x.socket->write(ret);
					del.push_back(it);
				}
			}
		}
		for (auto &d : del) {
			x.tempSubList.remove(d.key());
		}
	}
}


void CtlServer::onDevicesListReceived(const QVector_u16 &ids)
{
	if (ids.size()) { // steal wait
		bool isNeedToHandleSubscriptionsList = false;
		for (auto &x : tcp.clients) {
			QVector < QMap<int,bool>::iterator > del;
			for (auto it = x.tempSubList.begin(); it != x.tempSubList.end(); it++) {
				if (it.value()) {
					auto e = scast(ClientEvents, it.key());
					switch (e) {
						case ClientEvents::GetDeviceList: {
							QByteArray ret = tcp.handler->getDevListResponse(ids);
							x.socket->write(ret);
							del.push_back(it);
						} break;
						case ClientEvents::SubscribeForDeviceList: {
							x.subList.clear(); // paranoic clear
							for (const auto &id : ids) {
								x.subList[id] = true;
							}
							isNeedToHandleSubscriptionsList = true;
							del.push_back(it);
						} break;
						default: {} break;
					}
				}
			}
			for (auto &d : del) {
				x.tempSubList.remove(d.key());
			}
		}
		if (isNeedToHandleSubscriptionsList)
			handleSubscriptionsList();
	}
}


void CtlServer::handleSubscriptionsList()
{
	QMap <quint16, bool> mret;
	for (auto &x : tcp.clients) {
		for (auto it = x.subList.begin(); it != x.subList.end(); it++) {
			if (it.value()) {
				mret[it.key()] = true;
			}
		}
	}
	QVector_u16 ret;
	ret.reserve(mret.size());
	for (auto it = mret.begin(); it != mret.end(); it++) {
		ret.push_back(it.key());
	}
	emit subscribtionsListChanged(ret);
}


void CtlServer::start(const QHostAddress &addr, quint16 port)
{
	this->tcp.addr = addr;
	this->tcp.port = port;
	stop();
	this->tcp.socket->listen(addr, port);
}


void CtlServer::stop()
{
	this->tcp.socket->close();
}


CtlServer::CtlServer(const PrgDatabase *db, QObject *parent) : QObject(parent)
{
	this->db = db;
	{ /* Pipe clients notifying about tcp changes	*/		
		connect(db, &PrgDatabase::vpnAddrChanged, this, [this](){
			auto a = this->db->vpnAddr();
			QByteArray ret = this->pipe.handler->getVpnAddrResponse(a);
			for (auto &x : pipe.clients) {
				x->write(ret);
			}
		});
		connect(db, &PrgDatabase::serverAddrChanged, this, [this](){
			auto a = this->db->serverAddr();
			QByteArray ret = this->pipe.handler->getServAddrResponse(a);
			for (auto &x : pipe.clients) {
				x->write(ret);
			}
		});
	}

	this->pipe.handler = new QwFrameHandler(this);
	{
		connect(pipe.handler, &QwFrameHandler::allDataRequestFound, this, [this](void *sender){
			auto client = (QLocalSocket *)sender;
			QwFrameHandler::AllData d;
			d.rp = this->db->radioPrm();
			d.net = this->db->netParam();
			d.servAddr = this->db->serverAddr();
			d.vpnAddr = this->db->vpnAddr();
			d.vpnEnabled = this->db->vpnEnabled();
			QByteArray ret = this->pipe.handler->getAllDataResponse(d);
			client->write(ret);
		});
		connect(pipe.handler, &QwFrameHandler::radioPrmsResponseFound, this, [this](const ServStruct::RadioParams &d, void *sender){
			Q_UNUSED(sender)
			emit this->radioPrmsChangeRequested(d);
		});
		connect(pipe.handler, &QwFrameHandler::netPrmsResponseFound, this, [this](const QwFrameHandler::DeviceNetParams &d, void *sender){
			Q_UNUSED(sender)
			emit this->netPrmsChangeRequested(d);
		});
		connect(pipe.handler, &QwFrameHandler::servAddrResponseFound, this, [this](const QwFrameHandler::ServerAddr &d, void *sender){
			Q_UNUSED(sender)
			emit this->servAddrChangeRequested(d);
		});
		connect(pipe.handler, &QwFrameHandler::vpnCfgResponseFound, this, [this](const QString &d, void *sender){
			Q_UNUSED(sender)
			emit this->vpnCfgChangeRequested(d);
		});
		connect(pipe.handler, &QwFrameHandler::vpnEnableResponseFound, this, [this](bool d, void *sender){
			Q_UNUSED(sender)
			emit this->vpnStateChangeRequested(d);
		});
	}

	this->pipe.socket = new QLocalServer(this);
	{
		QLocalServer::removeServer(QStringLiteral(CONFIG_CTLPIPE_NAME));
		if (pipe.socket->listen(QStringLiteral(CONFIG_CTLPIPE_NAME))) {
			connect(pipe.socket, &QLocalServer::newConnection, this, [this](){
				QLocalSocket *client = pipe.socket->nextPendingConnection();
				if (client) {
					pipe.clients.push_back(client);
					connect(client, &QLocalSocket::disconnected, this, [this](){
						QLocalSocket *client = qobject_cast<QLocalSocket*>(sender());
						if (client) {
							this->pipe.clients.removeOne(client);
							client->deleteLater();
						}
					});
					connect(client, &QLocalSocket::readyRead, this, [this](){
						QLocalSocket *client = qobject_cast<QLocalSocket*>(sender());
						if (client) {
							this->pipe.handler->parse(client->readAll(), client);
						}
					});
				}
			});
		}
	}


	this->tcp.handler = new FrameHandler(this);
	{
		connect(tcp.handler, &FrameHandler::dataRequestFound, this, [this](int dt, int id, void *sender){
			auto client = (QTcpSocket *)sender;
			auto t = scast(FrameHandler::DataType, dt);
			switch (t) {
				case FrameHandler::DataType::Cntrl: {
					auto d = this->db->devicesCtl(id);
					auto ret = this->tcp.handler->getCtlResponse(true, d);
					client->write(ret);
				} break;
				case FrameHandler::DataType::Stat: {
					for (auto &x : this->tcp.clients) {
						if (x == client) {
							quint16 uid = scast(quint16, id);
							x.tempSubList[uid] = true;
							emit this->devStatRequested(uid);
							break;
						}
					}
				} break;
				default: {} break;
			}
		});
		connect(tcp.handler, &FrameHandler::ctlResponseFound, this, [this](bool valid, const DevStruct::DeviceCtl &d, void *sender){
			Q_UNUSED(sender);
			emit this->ctlChangeRequested(valid, d);
		});
		connect(tcp.handler, &FrameHandler::devListRequestFound, this, [this](void *sender){
			auto client = (QTcpSocket *)sender;
			for (auto &x : this->tcp.clients) {
				if (x == client) {
					x.tempSubList[scast(int, ClientEvents::GetDeviceList)] = true;
					emit this->devicesListRequested();
					break;
				}
			}
		});
		connect(tcp.handler, &FrameHandler::subscribeResponseFound, this, [this](bool sub, const QVector_u16 &ids, void *sender){
			auto client = (QTcpSocket *)sender;
			for (auto &x : this->tcp.clients) {
				if (x == client) {
					if (sub) {
						if (ids.size()) { // sub on specified slaves events
							for (const auto &id : ids) {
								x.subList[id] = true;
							}
						} else { // getting list of slaves firstly then sub on them
							x.subList.clear();
							x.tempSubList[scast(int, ClientEvents::SubscribeForDeviceList)] = true;
							emit this->devicesListRequested();
							break;
						}
					} else {
						if (ids.size()) { // 'remove' specified slaves from sub list
                            for (const auto &id : ids) {
                                auto it = x.subList.find(id);
                                if (it != x.subList.end()) {
                                    x.subList[id] = false;
                                }
                            }
                        } else { // just remove all subs
                            x.subList.clear();
						}
					}
					this->handleSubscriptionsList();
					break;
				}
			}
		});
		connect(tcp.handler, &FrameHandler::radioPrmsResponseFound, this, [this](const ServStruct::RadioParams &d, void *sender){
			Q_UNUSED(sender);
			emit this->radioPrmsChangeRequested(d);
		});
	}

	this->tcp.socket = new QTcpServer(this);
	{
		connect(tcp.socket, &QTcpServer::newConnection, this, [this](){
			QTcpSocket *client = this->tcp.socket->nextPendingConnection();
			if (client) {
				TcpClient tclient;
				tclient.socket = client;
				tcp.clients.push_back(tclient);
				connect(client, &QTcpSocket::disconnected, this, [this](){
					QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
					if (client) {
						for (auto &x : this->tcp.clients) {
							if (x == client) {
								this->tcp.clients.removeOne(x);
								this->handleSubscriptionsList();
								break;
							}
						}
						client->deleteLater();
					}
				});
				connect(client, &QTcpSocket::readyRead, this, [this](){
					QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
					if (client) {
						auto data = client->readAll();
						this->tcp.handler->parse(data, client);
					}
				});
			}
		});
	}
}


CtlServer::~CtlServer()
{}
