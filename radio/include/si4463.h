#ifndef SI4463_H
#define SI4463_H

#include "iradiochip.h"
#include <cstdint>

struct si4463_t;
class LinuxGpioIn;
class LinuxGpioOut;

class QThread;
class QEpoll;

/*!
 * @class Si4463
 * @brief Драйвер чипа Si4463
 * @details используется драйвер от silicon labs @ref si4463_t .
 * Данный класс представляет собой qt обертку
*/
class Si4463 : public IRadioChip
{
public:
	struct Config : public IRadioChip::Config {
		int irqPinNum;
		int ctsPinNum;
		int sdnPinNum;
        int csPinNum;
        Config() : IRadioChip::Config() {};
        virtual ~Config() = default;
	};

	Si4463(IBus *bus);
	virtual ~Si4463();

public slots:
    virtual void init(const IRadioChip::Config *config) override;
    virtual int sendData(const QByteArray &data) const override;

private:
	si4463_t *si4463_slabs;

    QThread *thread;
	QEpoll *qepoll;

	LinuxGpioIn *irq;
	LinuxGpioIn *cts;
	LinuxGpioOut *sdn;
    LinuxGpioOut *cs;

    void si4463_slabs_WriteRead(const uint8_t *pTxData, uint8_t *pRxData, const uint16_t txSize);
	void si4463_slabs_SetShutdown(void);
	void si4463_slabs_ClearShutdown(void);
	void si4463_slabs_Select(void);
	void si4463_slabs_Deselect(void);
	void si4463_slabs_DelayMs(uint32_t delayMs);
	uint8_t si4463_slabs_IsClearToSend(void);

    int getReceivedData(uint8_t *buf, int sz) const;
    void reinit(void) const;
};

#endif /* SI4463_H */
