#ifndef IRADIOCHIP_H
#define IRADIOCHIP_H

#include "ibus.h"
#include <QObject>
#include <QByteArray>

/*!
 * @class IRadioChip
 * @brief Интерфейс радио чипа Qt object 
*/
class IRadioChip : public QObject
{
	Q_OBJECT
public:

	IRadioChip(IBus *b) : QObject(nullptr) {
		bus = b;
	}
	virtual ~IRadioChip() {
		delete bus;
	}

	struct Config {
        Config() = default;
        virtual ~Config() = default;
	};

public slots:
    virtual void init(const IRadioChip::Config *config) = 0;
	virtual int sendData(const QByteArray &data) const = 0;

signals:
	void dataIsReady(const QByteArray &data);

protected:
	IBus *bus;
};

#endif /* IRADIOCHIP_H */
