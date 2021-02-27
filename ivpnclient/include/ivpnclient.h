#ifndef IVPNCLIENT_H
#define IVPNCLIENT_H

#include <QObject>
#include <QString>

/*!
 * @class IVpnClient
 * @brief Абстрактный класс, определяющий функции запуска/останова vpn-клиент-приложения
*/
class IVpnClient : public QObject
{
	Q_OBJECT
public:
	IVpnClient(QObject *parent = nullptr) : QObject(parent) {}
	virtual ~IVpnClient() {}

	void setCfgFilePath(const QString &cfgFilePath) { this->cfgFilePath = cfgFilePath; }

public slots:
	virtual void start() = 0;
	virtual void stop() = 0;
	virtual inline void restart() { start(); }

signals:
	void started();
	void stopped();

protected:
	QString cfgFilePath;
};

#endif // IVPNCLIENT_H
