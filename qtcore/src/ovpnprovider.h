#ifndef OVPNPROVIDER_H
#define OVPNPROVIDER_H

#include "ivpnclient.h"
#include <QProcess>

/*!
 * @class OvpnProvider
 * @brief Класс управления процессом openvpn
*/
class OvpnProvider : public IVpnClient
{
	Q_OBJECT
public:
	OvpnProvider(QObject *parent = nullptr);
	virtual ~OvpnProvider();

public slots:
	virtual void start() override;
	virtual void stop() override;

private slots:
	void onProcFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
	QProcess *proc;
	bool running;

	void p_stop();
	void p_start();
};

#endif /* OVPNPROVIDER_H */