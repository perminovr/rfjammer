#include "ovpnprovider.h"
#include "config.h"

#define OPENVPN_BIN 	"/usr/sbin/openvpn"
#define ONVPNCONFIG		CONFIG_SCRIPTS_DIR "onvpnconfig.sh"


void OvpnProvider::start()
{
	if (running) {
		p_stop();
	} else {
		system(ONVPNCONFIG " start");
		p_start();
	}
}


void OvpnProvider::stop()
{
	if (running) {
		running = false;
		p_stop();
	}
}


void OvpnProvider::p_start()
{
	if (cfgFilePath.size() > 0) {
		QStringList args;
		args << "--config" << cfgFilePath;
		proc->setArguments(args);
		system("killall -9 " OPENVPN_BIN);
		proc->start();
		running = true;
	}
}


void OvpnProvider::p_stop()
{
	proc->kill();
}


void OvpnProvider::onProcFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	Q_UNUSED(exitCode)
	Q_UNUSED(exitStatus)
	// wanna to restart if crashed / restarted
	if (this->running) {
		this->p_start();
	} else {
		system(ONVPNCONFIG " stop");
		emit this->stopped();
	}
}


OvpnProvider::OvpnProvider(QObject *parent) : IVpnClient(parent)
{
	running = false;
	proc = new QProcess(this);
	proc->setStandardOutputFile(QProcess::nullDevice());
	proc->setStandardErrorFile(QProcess::nullDevice());
	proc->setProgram(OPENVPN_BIN);
	connect(proc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &OvpnProvider::onProcFinished);
	connect(proc, &QProcess::started, this, [](){
		system(ONVPNCONFIG " running");
	});
	connect(proc, &QProcess::started, this, &OvpnProvider::started);
}


OvpnProvider::~OvpnProvider()
{
	stop();
}