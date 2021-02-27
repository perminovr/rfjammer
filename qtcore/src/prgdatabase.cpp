#include "prgdatabase.h"
#include <QVariant>
#include <QtCore>
#include <unistd.h>

#define readParamInit \
	QVariant v; \
	QString prm;

#define readParam(k, def, strg) {\
	v = (strg)->value(k, def);\
	if (v.type() == QVariant::String && v.toString().size())\
		{prm = v.toString();}\
	else\
		{prm = def;} \
}

#define VPN_SECTION "vpn/"
# define VPN_IFNAME "ifname"
# define VPN_FNAME "fname"
# define VPN_EN "en"
#define DEVCMN_SECTION "devices/"
# define DEVCMN_CNT "cnt"
#define DEV_SECTION "device_%1/"
# define DEV_ID "id"
# define DEV_EN "en"
# define DEV_LVL "lvl"
#define RADIO_SECTION "radio/"
# define RADIO_TOA "toa"
# define RADIO_TOR "tor"
# define RADIO_SP "sp"
# define RADIO_PP "pp"
#define ETH_SECTION "eth/"
# define ETH_NAME "name"
# define ETH_MODE "mode"
# define ETH_IP "ip"
# define ETH_MASK "mask"
# define ETH_GW "gw"
#define SERV_SECTION "server/"
# define SERV_SLOT "slot"
# define SERV_PORT "port"


void PrgDatabase::readIfaces()
{
	readParamInit;

	readParam(VPN_SECTION VPN_IFNAME, "tun0", this->settings);
	this->m_tunlName = prm;

	readParam(ETH_SECTION ETH_NAME, "eth0", this->settings);
	this->m_ethName = prm;
}


void PrgDatabase::read()
{
	readParamInit;

	// vpn

	readParam(VPN_SECTION VPN_FNAME, "", this->settings);
	this->m_vpnFileName = prm;

	readParam(VPN_SECTION VPN_EN, "0", this->settings);
	this->m_vpnEnabled = scast(bool, prm.toInt());

	// devices

	readParam(DEVCMN_SECTION DEVCMN_CNT, "0", this->settings);
	int sz = prm.toInt();

	for (int i = 0; i < sz; ++i) {
		QString pname;
		DevStruct::DeviceCtl ctl;

		pname = QString(DEV_SECTION DEV_ID).arg(i);
		readParam(pname, "0", this->settings);
		ctl.getM(id) = scast(quint16, prm.toInt());

		pname = QString(DEV_SECTION DEV_EN).arg(i);
		readParam(pname, "0", this->settings);
		ctl.getM(ch.devEn) = scast(bool, prm.toInt());

		pname = QString(DEV_SECTION DEV_LVL).arg(i);
		readParam(pname, "0", this->settings);
		ctl.getM(ch.signalLvl) = scast(quint8, prm.toInt());

		this->m_devicesCtl.insert(ctl.getM(id), ctl);
	}

	// radio

	readParam(RADIO_SECTION RADIO_TOA, "200", this->settings);
	this->m_radioPrm.timeoutAck = prm.toInt();

	readParam(RADIO_SECTION RADIO_TOR, "1000", this->settings);
	this->m_radioPrm.timeoutRepeat = prm.toInt();

	readParam(RADIO_SECTION RADIO_SP, "0", this->settings);
	this->m_radioPrm.slavePause = prm.toInt();

	readParam(RADIO_SECTION RADIO_PP, "1000", this->settings);
	this->m_radioPrm.pollPause = prm.toInt();

	// eth

	readParam(ETH_SECTION ETH_MODE, "0", this->settings);
	this->m_netParam.ipMode = prm.toInt();

	readParam(ETH_SECTION ETH_IP, "192.168.1.10", this->settings);
	this->m_netParam.localAddr.setIp(QHostAddress(prm));

	readParam(ETH_SECTION ETH_MASK, "255.255.255.0", this->settings);
    QHostAddress tm = QHostAddress(prm);
    QString o = tm.toString();
    qDebug() << o;
	this->m_netParam.localAddr.setNetmask(QHostAddress(prm));

	readParam(ETH_SECTION ETH_GW, "192.168.1.1", this->settings);
	this->m_netParam.gw = QHostAddress(prm);

	// server

	readParam(SERV_SECTION SERV_SLOT, "1", this->settings);
	this->m_serverAddr.slot = scast(QwFrameHandler::ServerAddr::Slot, prm.toInt());

	readParam(SERV_SECTION SERV_PORT, "22000", this->settings);
	this->m_serverAddr.port = prm.toInt();

	this->save();
}


void PrgDatabase::save()
{
	// vpn
	this->settings->setValue(VPN_SECTION VPN_IFNAME, this->m_tunlName);
	this->settings->setValue(VPN_SECTION VPN_FNAME, this->m_vpnFileName);
	this->settings->setValue(VPN_SECTION VPN_EN, scast(int, this->m_vpnEnabled));
	// devices
	int sz = this->m_devicesCtl.size();
	this->settings->setValue(DEVCMN_SECTION DEVCMN_CNT, sz);
	int i = 0;
	for (auto &x : this->m_devicesCtl) {
		QString pname;
		pname = QString(DEV_SECTION DEV_ID).arg(i);
		this->settings->setValue(pname, scast(int, x.getM(id)));
		pname = QString(DEV_SECTION DEV_EN).arg(i);
		this->settings->setValue(pname, scast(int, x.getM(ch.devEn)));
		pname = QString(DEV_SECTION DEV_LVL).arg(i);
		this->settings->setValue(pname, scast(int, x.getM(ch.signalLvl)));
		i++;
	}
	// radio
	this->settings->setValue(RADIO_SECTION RADIO_TOA, this->m_radioPrm.timeoutAck);
	this->settings->setValue(RADIO_SECTION RADIO_TOR, this->m_radioPrm.timeoutRepeat);
	this->settings->setValue(RADIO_SECTION RADIO_SP, this->m_radioPrm.slavePause);
	this->settings->setValue(RADIO_SECTION RADIO_PP, this->m_radioPrm.pollPause);
	// eth
	this->settings->setValue(ETH_SECTION ETH_NAME, this->m_ethName);
	this->settings->setValue(ETH_SECTION ETH_MODE, this->m_netParam.ipMode);
	this->settings->setValue(ETH_SECTION ETH_IP, this->m_netParam.localAddr.ip().toString());
	this->settings->setValue(ETH_SECTION ETH_MASK, this->m_netParam.localAddr.netmask().toString());
	this->settings->setValue(ETH_SECTION ETH_GW, this->m_netParam.gw.toString());
	// server
	this->settings->setValue(SERV_SECTION SERV_SLOT, scast(int, this->m_serverAddr.slot));
	this->settings->setValue(SERV_SECTION SERV_PORT, this->m_serverAddr.port);

	this->settings->sync();
	sync();
}


PrgDatabase::PrgDatabase(QObject *parent) : QObject(parent)
{
	QCoreApplication::setOrganizationName("E");
	QCoreApplication::setOrganizationDomain("E.ru");
	QCoreApplication::setApplicationName(QStringLiteral(CONFIG_PROJECT_NAME));
	settings = new QSettings(QStringLiteral(CONFIG_PARAMS_FILE_PATH), QSettings::IniFormat, this);
	connect(this, &PrgDatabase::smthChanged, this, &PrgDatabase::save);
}


PrgDatabase::~PrgDatabase()
{}
