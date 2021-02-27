#include "si4463.h"
#include "si4463_slabs.h"
#include "qepoll.h"
#include "gpiodrv.h"


#define _SI4463_StartRx(p) SI4463_StartRx(p, 20, false, false, false);

void Si4463::reinit(void) const
{
    sdn->setVal(true);
    SI4463_Init(si4463_slabs);
    SI4463_ClearRxFifo(si4463_slabs);
    SI4463_ClearInterrupts(si4463_slabs);
}


void Si4463::init(const IRadioChip::Config *config)
{
    auto cfg = dynamic_cast<const Si4463::Config *>(config);
    if (cfg) {
        irq = new LinuxGpioIn(cfg->irqPinNum);
        cts = new LinuxGpioIn(cfg->ctsPinNum);
        sdn = new LinuxGpioOut(cfg->sdnPinNum);
        cs = new LinuxGpioOut(cfg->csPinNum);

        cs->setVal(true);

        int fd = irq->prepForEpoll(LinuxGpioIn::Edge::Falling);
        QEpoll::EpollDesc desc;
        desc.fd = fd;
        desc.events = EPOLLET;
        desc.add = true;
        qepoll->toEpoll(desc);

        thread->start();

        reinit();
    }
}

#include <iostream>

int Si4463::getReceivedData(uint8_t *buf, int sz) const // actually it is irq handler
{
    int ret = 0;
	SI4463_GetInterrupts(si4463_slabs);
	if (si4463_slabs->interrupts.packetSent) {
		SI4463_ClearTxFifo(si4463_slabs);
        _SI4463_StartRx(si4463_slabs);
	}
	if (si4463_slabs->interrupts.packetRx) {
		SI4463_ReadRxFifo(si4463_slabs, (uint8_t *)buf, sz);
		SI4463_ClearRxFifo(si4463_slabs);
        _SI4463_StartRx(si4463_slabs);
        std::cout << "packetRx ";
        ret = 32;
	}
	if (si4463_slabs->interrupts.cal) {
		SI4463_GetChipStatus(si4463_slabs);
	}
	if (si4463_slabs->interrupts.fifoUnderflowOverflowError) {
		SI4463_ClearRxFifo(si4463_slabs);
		SI4463_GetChipStatus(si4463_slabs);
	}
	if (si4463_slabs->interrupts.stateChange) {
		SI4463_GetChipStatus(si4463_slabs);
	}
	if (si4463_slabs->interrupts.cmdError) {
		SI4463_GetChipStatus(si4463_slabs);
	}
	if (si4463_slabs->interrupts.chipReady) {
		SI4463_GetChipStatus(si4463_slabs);
	}
	if (si4463_slabs->interrupts.lowBatt) {
		SI4463_GetChipStatus(si4463_slabs);
	}
	if (si4463_slabs->interrupts.wut) {
		SI4463_GetChipStatus(si4463_slabs);
	}
	memset(&si4463_slabs->interrupts, 0, sizeof(si4463_slabs->interrupts));
    SI4463_ClearAllInterrupts(si4463_slabs);
    return ret;
}

#include <QDebug>

int Si4463::sendData(const QByteArray &data) const
{
    QByteArray d(data, 32);
    int ret = SI4463_Transmit(si4463_slabs, (const uint8_t *)d.constData(), d.size());
    if (ret == 0) {
        return d.size();
    } else {
        qDebug() << "deinit";
        reinit();
        return 0;
    }
}


void Si4463::si4463_slabs_WriteRead(const uint8_t *pTxData, uint8_t *pRxData, const uint16_t txSize)
{
	bus->readWrite((const char *)pTxData, (char *)pRxData, txSize);
}


void Si4463::si4463_slabs_SetShutdown(void)
{
	sdn->setVal(true);
}


void Si4463::si4463_slabs_ClearShutdown(void)
{
	sdn->setVal(false);
}


void Si4463::si4463_slabs_Select(void)
{
    cs->setVal(false);
}


void Si4463::si4463_slabs_Deselect(void)
{
    cs->setVal(true);
}


void Si4463::si4463_slabs_DelayMs(uint32_t delayMs)
{
	usleep(delayMs*1000);
}


uint8_t Si4463::si4463_slabs_IsClearToSend(void)
{
	bool val = false;
	cts->getVal(val);
    return (uint8_t)val;
}


Si4463::Si4463(IBus *bus) : IRadioChip(bus)
{
    // later
    cts = nullptr;
    sdn = nullptr;
    irq = nullptr;
    cs = nullptr;

    thread = new QThread();
	thread->setStackSize(65535);
	qepoll = new QEpoll();
	connect(qepoll, &QEpoll::eventsOccured, this, [this](int fd, uint32_t events){
		Q_UNUSED(fd)
		Q_UNUSED(events)
        uint8_t buf[32];
        int rc = getReceivedData(buf, 32);
		if (rc) {
            emit dataIsReady(QByteArray((char *)buf, rc));
		}
	}, Qt::QueuedConnection);
	QObject::connect(thread, &QThread::started, qepoll, &QEpoll::run);
	qepoll->moveToThread(thread);
    // starting later

	si4463_slabs = new si4463_t();

	using std::placeholders::_1;
	using std::placeholders::_2;
	using std::placeholders::_3;
    si4463_slabs->WriteRead = std::bind(&Si4463::si4463_slabs_WriteRead, this, _1, _2, _3);
    si4463_slabs->SetShutdown = std::bind(&Si4463::si4463_slabs_SetShutdown, this);
    si4463_slabs->ClearShutdown = std::bind(&Si4463::si4463_slabs_ClearShutdown, this);
    si4463_slabs->Select = std::bind(&Si4463::si4463_slabs_Select, this);
    si4463_slabs->Deselect = std::bind(&Si4463::si4463_slabs_Deselect, this);
    si4463_slabs->DelayMs = std::bind(&Si4463::si4463_slabs_DelayMs, this, _1);
    si4463_slabs->IsClearToSend = std::bind(&Si4463::si4463_slabs_IsClearToSend, this);
}


Si4463::~Si4463()
{
    qepoll->stop();
	thread->quit();
	thread->wait();
	delete thread;
    delete qepoll;
	delete si4463_slabs;
	delete irq;
	delete cts;
	delete sdn;
}

