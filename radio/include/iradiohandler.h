#ifndef IRADIOHANDLER_H
#define IRADIOHANDLER_H

#include <QObject>
#include <QByteArray>

/*!
 * @class IRadioHandler
 * @brief Интерфейс радио обработчика Qt object .
*/
class IRadioHandler : public QObject
{
	Q_OBJECT
public:
    IRadioHandler(QObject *parent = nullptr) : QObject(parent) {};
    virtual ~IRadioHandler() = default;

    virtual void shutdown() = 0;

public slots:
	virtual void write(const QByteArray &data) = 0;

signals:
	void dataIsReady(const QByteArray &data);
};

#endif /* IRADIOHANDLER_H */
