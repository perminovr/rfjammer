#include "common.h"
#include "qtworker.h"
#include "prgdatabase.h"
#include "udhcpprovider.h"
#include "systemnetworking.h"
#include "ovpnprovider.h"
#include "ctlserver.h"
#include "rfjammersctl.h"
#include "staticmembers.h"
#include "statusdrv.h"
#include <cstdlib>
#include <QFile>


void QtWorker::restartCtlServer(const QHostAddress &addr, quint16 port, int s)
{
	prgDebug() << "QtWorker::restartCtlServer" << addr << port << s;
	if (addr != QHostAddress::Null && port) {
		auto slot = scast(QwFrameHandler::ServerAddr::Slot, s);
		auto d = db->serverAddr();
		if (d.addr != addr || d.port != port) {
			ctl->stop();
			ctl->start(addr, port);
			d.addr = addr;
			d.port = port;
			d.slot = slot;
			db->setServerAddr(d);
		}
	}
}


void QtWorker::start()
{
	db->read();

	// networking
	auto np = db->netParam();
	NetworkingCommon::IPMode mode = (np.ipMode == 0)? NetworkingCommon::IPMode::Static : NetworkingCommon::IPMode::Dhcp;
	networking->setSysIfaceIpMode(0, mode);
	networking->setAddress(0, np.localAddr);
	networking->setRoute(0, np.gw);

	// ctl server (starts on local firstly)
	auto sa = db->serverAddr(); // slot & port
	restartCtlServer(np.localAddr.ip(), sa.port, sa.slot); // temporarily, before networking events

	// vpn
	{
		auto d = db->vpnFileName();
		vpn->setCfgFilePath(d);
		if (db->vpnEnabled()) {
			vpn->restart();
		}
	}

	// radio jammers control
	{
		auto d = db->radioPrm();
		rfj->setRadioParams(d);
		auto ctl = db->devicesCtlAll();
		for (const auto &x : ctl) {
			rfj->setDevicesCtl(x);
		}
	}

	// wtgui
	{
		QFile netconf(QStringLiteral(CONFIG_STATIC_NETCONF));
		if (netconf.open(QFile::ReadOnly)) {
			// ip:port
			auto nc = (QString::fromUtf8(netconf.readAll().constData())).split('\n');
            if (nc.size() == 3) {
				QStringList args;
				args << "--docroot" << CONFIG_WTGUI_DOCROOT;
				args << "--approot" << CONFIG_WTGUI_APPROOT;
				args << "--resources-dir=" CONFIG_WTGUI_RESOURCES;
				args << "--http-address" << nc[0];
				args << "--http-port" << nc[2];
				gui.proc->setProgram(QStringLiteral(CONFIG_GUI_APP_PATH));
				gui.proc->setArguments(args);
				gui.proc->setStandardOutputFile(QProcess::nullDevice());
                gui.proc->setStandardErrorFile(QProcess::nullDevice());
				connect(gui.proc, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error){
					Q_UNUSED(error)
					gui.proc->kill();
                    gui.timer->start(500);
				});
				connect(gui.proc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [this](int exitCode, QProcess::ExitStatus status){
					Q_UNUSED(exitCode)
					Q_UNUSED(status)
                    gui.timer->start(500);
				});
				system("killall -9 " CONFIG_GUI_APP_NAME);				
				gui.proc->start();
			}
		}
	}
}


QtWorker::QtWorker(QObject *parent) : QObject(parent)
{
	status = new StatusDrv(this);

	db = new PrgDatabase(this);
	{
		// connect(db, &PrgDatabase::tunlNameChanged, this, [this](){});
		// connect(db, &PrgDatabase::ethNameChanged, this, [this](){});
		// connect(db, &PrgDatabase::vpnEnabledChanged, this, [this](){});
		// connect(db, &PrgDatabase::vpnFileNameChanged, this, [this](){});
		// connect(db, &PrgDatabase::vpnAddrChanged, this, [this](){});
		// connect(db, &PrgDatabase::netParamChanged, this, [this](){});
		// connect(db, &PrgDatabase::serverAddrChanged, this, [this](){});
		// connect(db, &PrgDatabase::radioPrmChanged, this, [this](){});
		// connect(db, &PrgDatabase::devicesCtlChanged, this, [this](const int &id){});
	}

	db->readIfaces();
	networking = SystemNetworking::instance(this, { db->ethName(), db->tunlName() });
	{
		IDhcpClient *dhcp = new UdhcpProvider(this);
		dhcp->start(IDhcpClient::Implementer);
		networking->setDhcpProvider(dhcp);

		// void setAddress(int iface, const QNetworkAddressEntry &addr, QString *error = nullptr);
		// void setRoute(int iface, const QHostAddress &gateway, QString *error = nullptr);
		// void setSysIfaceIpMode(int iface, NetworkingCommon::IPMode mode);

		connect(networking, &SystemNetworking::addressChanged, this, [this](int iface){ // static/dhcp
			prgDebug() << "QtWorker SystemNetworking::addressChanged signal:" << iface;
			if (iface == 0) { // ethernet
				auto d = db->serverAddr();
				if (d.slot == QwFrameHandler::ServerAddr::Slot::Device) {
					QNetworkAddressEntry addr;
					if (networking->address(iface, addr)) {
						restartCtlServer(addr.ip(), d.port, d.slot);
					}
				}
			}
		});
		connect(networking, &SystemNetworking::ifaceAddrChanged, this, [this](int iface, const QNetworkAddressEntry &addr, bool add){ // from vpn server
			prgDebug() << "QtWorker SystemNetworking::ifaceAddrChanged signal:" << iface << add << addr.ip();
			if (add && iface == 1) { // tunnel
				auto a = addr.ip();
				auto d = db->serverAddr();
				if (d.slot == QwFrameHandler::ServerAddr::Slot::Vpn) {
					restartCtlServer(a, d.port, d.slot);
				}
				db->setVpnAddr(a);
			}
		});
	}

	ctl = new CtlServer(db, this);
	{
		// void start(const QHostAddress &addr, quint16 port);
		// void stop();
		// void restart();
		// void onDevicesStatReceived(bool valid, const DevStruct::DeviceStat &d);
		// void onDevicesListReceived(const QVector_u16 &ids);

		connect(ctl, &CtlServer::radioPrmsChangeRequested, this, [this](const ServStruct::RadioParams &d){
			prgDebug() << "QtWorker CtlServer::radioPrmsChangeRequested signal:";
			db->setRadioPrm(d);
			rfj->setRadioParams(d);
		});
		connect(ctl, &CtlServer::netPrmsChangeRequested, this, [this](const QwFrameHandler::DeviceNetParams &d){
			prgDebug() << "QtWorker CtlServer::netPrmsChangeRequested signal:";
			db->setNetParam(d);
			networking->setAddress(0, d.localAddr);
			networking->setRoute(0, d.gw);
			NetworkingCommon::IPMode mode = (d.ipMode == 0)? NetworkingCommon::IPMode::Static : NetworkingCommon::IPMode::Dhcp;
			networking->setSysIfaceIpMode(0, mode);
		});
		connect(ctl, &CtlServer::servAddrChangeRequested, this, [this](const QwFrameHandler::ServerAddr &d){
			prgDebug() << "QtWorker CtlServer::servAddrChangeRequested signal:";
			if (d.slot == QwFrameHandler::ServerAddr::Slot::Device) {
				QNetworkAddressEntry addr;
				if (networking->address(0, addr)) {
					restartCtlServer(addr.ip(), d.port, d.slot);
				}
			} else
			if (d.slot == QwFrameHandler::ServerAddr::Slot::Vpn) {
				auto a = db->vpnAddr();
				restartCtlServer(a, d.port, d.slot);
			}
		});
		connect(ctl, &CtlServer::vpnCfgChangeRequested, this, [this](const QString &d){
			prgDebug() << "QtWorker CtlServer::vpnCfgChangeRequested signal:";
			QString newName = QStringLiteral(CONFIG_VPN_CONF_FILE_PATH);
			QFile newConf(d);
			QFile oldConf(newName);
			if (newConf.exists()) {
				oldConf.remove();
				newConf.rename(newName);
				db->setVpnFileName(newName);
				vpn->setCfgFilePath(newName);
				if (db->vpnEnabled()) {
					vpn->restart();
				}
			}
		});
		connect(ctl, &CtlServer::vpnStateChangeRequested, this, [this](bool d){
			prgDebug() << "QtWorker CtlServer::vpnStateChangeRequested signal:";
			db->setVpnEnabled(d);
            if (d) {
                vpn->restart();
            } else {
                db->setVpnAddr(QHostAddress::Null);
                vpn->stop();
            }
		});
		connect(ctl, &CtlServer::devStatRequested, this, [this](quint16 id){
			prgDebug() << "QtWorker CtlServer::devStatRequested signal:";
			rfj->requestDevStat(id);
		});
		connect(ctl, &CtlServer::ctlChangeRequested, this, [this](bool valid, const DevStruct::DeviceCtl &d){
			prgDebug() << "QtWorker CtlServer::ctlChangeRequested signal:";
			Q_UNUSED(valid)
			db->setDevicesCtl(d);
			rfj->setDevicesCtl(d);
		});
		connect(ctl, &CtlServer::subscribtionsListChanged, this, [this](const QVector_u16 &ids){
			prgDebug() << "QtWorker CtlServer::subscribtionsListChanged signal:";
			rfj->setSubscribtionsList(ids);
		});
		connect(ctl, &CtlServer::devicesListRequested, this, [this](){
			prgDebug() << "QtWorker CtlServer::devicesListRequested signal:";
			rfj->requestDevicesList();
		});
	}

	vpn = new OvpnProvider(this);
	{
		// void start();
		// void stop();
		// void restart()

		// connect(vpn, &OvpnProvider::started, this, [this](){});
		// connect(vpn, &OvpnProvider::stopped, this, [this](){});
	}

	rfj = new RfJammersCtl(this);
	{
		// void setRadioParams(const ServStruct::RadioParams &d);
		// void setSubscribtionsList(const QVector_u16 &ids);
		// void requestDevicesList();
		// void requestDevStat(quint16 id);
		// void setDevicesCtl(const DevStruct::DeviceCtl &d);

		connect(rfj, &RfJammersCtl::devicesListReceived, this, [this](const QVector_u16 &ids){
			prgDebug() << "QtWorker RfJammersCtl::devicesListReceived signal:";
			ctl->onDevicesListReceived(ids);
		});
		connect(rfj, &RfJammersCtl::devicesStatReceived, this, [this](bool valid, const DevStruct::DeviceStat &d){
			// prgDebug() << "QtWorker RfJammersCtl::devicesStatReceived signal:";
			ctl->onDevicesStatReceived(valid, d);
		});
	}

	gui.proc = new QProcess(this);
	gui.timer = new QTimer(this);
	gui.timer->setSingleShot(true);
	connect(gui.timer, &QTimer::timeout, this, [this](){
		gui.proc->start();
	});

	prgDebug() << "QtWorker::QtWorker done";
}


QtWorker::~QtWorker()
{}
