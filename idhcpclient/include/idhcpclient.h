#ifndef IDHCPCLIENT_H
#define IDHCPCLIENT_H

#include <QObject>
#include <QNetworkAddressEntry>

class PipeClient;
class PipeServer;

/*!
 * @class IDhcpClient
 * @brief Абстрактный класс, определяющий функции отправки/получения dhcp-событий.
 * @details Поведение: внешний dhcp-демон вызывает исполняемый файл dhcp-уведомителя. Уведомитель
 * через функции класса IDhcpClient сообщает о полученных настройках в основную программу.
*/
class IDhcpClient : public QObject
{
	Q_OBJECT
public:
	enum Role {
		Notifier,
		Implementer
	};
	enum DhcpState {
		Running,
		Stopped
	};

	/*!
	* @fn IDhcpClient::start
	* @brief запуск каналов сообщений между клиентом и сервером idhcp
	*/
	virtual void start(Role role = Role::Notifier);
	/*!
	* @fn IDhcpClient::notify
	* @brief уведомление о новых параметрах. вызывается клиентом idhcp
	*/
	void notify(const QString &iface, const QString &ip, const QString &mask, const QString &defGateway, bool add);

	virtual bool runDhcpDaemon(const QString &iface) = 0;
	virtual bool stopDhcpDaemon(const QString &iface) = 0;

	IDhcpClient(QObject *parent = nullptr);
	virtual ~IDhcpClient();

signals:
	void isReady();
	void complete();
	/*!
	* @fn IDhcpClient::notificationReceived
	* @brief уведомление о новых параметрах. вызывается сервером idhcp; получатель - рабочее ПО
	*/
	void notificationReceived(const QString &iface, const QNetworkAddressEntry &addr, const QHostAddress &defGateway, bool add);

	void dhcpStateChanged(DhcpState state, const QString &iface);

protected:
	Role role;

private slots:
	void onClientData(const QByteArray &data);

private:
	QString serverName;
	PipeClient *client;
	PipeServer *server;
};

#endif // IDHCPCLIENT_H
