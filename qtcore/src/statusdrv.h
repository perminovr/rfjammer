#ifndef STATUSDRV_H
#define STATUSDRV_H

#include <QObject>
#include <QTimer>
#include "common.h"
#include "gpiodrv.h"

class LinuxGpioOut;

class StatusDrv : public QObject
{
	Q_OBJECT
public:
	void setOkay() {
        state = State::Ok;
		lred->setVal(false);
	}
	void setError() {
        state = State::Error;
		lgreen->setVal(false);
	}

	StatusDrv(QObject *parent) : QObject(parent) {
        state = State::Ok;
		
		lred = new LinuxGpioOut(CONFIG_RED_LED_GPIO);
		lgreen = new LinuxGpioOut(CONFIG_GREEN_LED_GPIO);

		lred->setVal(false);
		lgreen->setVal(false);

		timer = new QTimer(this);
		timer->setInterval(300);
		QObject::connect(timer, &QTimer::timeout, this, [this](){
            (state == State::Ok)? lgreen->toggle() : lred->toggle();
		});
		timer->start();
	}
	~StatusDrv() {
		delete lred;
		delete lgreen;
	}

private:
	enum State {
        Ok, Error
	} state;
	QTimer *timer;
	LinuxGpioOut *lred;
	LinuxGpioOut *lgreen;
};

#endif // STATUSDRV_H
