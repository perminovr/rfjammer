#ifndef RADIOHANDLER_H
#define RADIOHANDLER_H

#include "iradiochip.h"
#include "iradiohandler.h"
#include <QObject>
#include <QByteArray>

/*!
 * @class RadioHandler
 * @brief Радио обработчик. Чип, используемый обработчиком, помещается в отдельный поток
*/
class RadioHandler : public IRadioHandler
{
	Q_OBJECT
public:
    RadioHandler(IRadioChip *chip, const IRadioChip::Config *config, QObject *parent = nullptr);
	virtual ~RadioHandler();

    void shutdown() override;

public slots:
	virtual void write(const QByteArray &data) override { emit p_write(data); }

signals:
	void p_write(const QByteArray &data);

private:
    bool destroyed;
    QThread *thread;
};

#endif /* RADIOHANDLER_H */
