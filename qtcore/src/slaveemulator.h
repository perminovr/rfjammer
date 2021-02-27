#ifndef SLAVEEMULATOR_H
#define SLAVEEMULATOR_H

#include "iradiohandler.h"
#include "framehandler.h"
#include <QTimer>
#include <QLinkedList>
#include <QMap>
#include <QDebug>

/*!
 * @class SlaveEmulator
 * @brief Эмулятор радио-устройства. В целях тестирования
*/
class SlaveEmulator : public IRadioHandler
{
	Q_OBJECT
public:
	SlaveEmulator(QObject *parent = nullptr) : IRadioHandler(parent) {
		h = new FrameHandler(this);
		connect(h, &FrameHandler::devListRequestFound, this, [this](void *sender){
			Q_UNUSED(sender)
			qDebug() << "SlaveEmulator FrameHandler::devListRequestFound";
			nextState(0, 100);
		});
		connect(h, &FrameHandler::dataRequestFound, this, [this](int dt, int id, void *sender){
			Q_UNUSED(sender)
			qDebug() << "SlaveEmulator FrameHandler::dataRequestFound" << dt << id;
			auto t = scast(FrameHandler::DataType, dt);
			if (t == FrameHandler::DataType::Stat) {
				if (stat.contains(id)) {
                    auto &s = stat[id];
					s.m.ch.T += 1+id;
					s.m.ch.outputRF += 10+id;
					s.m.ch.reflectedPwr += 100+id;
					s.m.aux.T += 2+id;
					s.m.aux.U += 11+id;
					r_stat = s;
					nextState(1, 100);
				}
			}
		});
		connect(h, &FrameHandler::ctlResponseFound, this, [this](bool valid, const DevStruct::DeviceCtl &d, void *sender){
			Q_UNUSED(sender)
            qDebug() << "SlaveEmulator FrameHandler::ctlResponseFound" << valid << d.m.id << d.m.ch.devEn << d.m.ch.signalLvl << d.m.packetId;
            if (stat.contains(d.m.id)) {
                auto &s = stat[d.m.id];
                s.m.ch.devEn = d.m.ch.devEn;
                s.m.ch.signalLvl = d.m.ch.signalLvl;
                r_ctl = FrameHandler::getCtlResponse(valid, d);
				nextState(2, 100);
			}
		});

		timer = new QTimer(this);
		timer->setSingleShot(true);
		connect(timer, &QTimer::timeout, this, [this](){
			QByteArray ret;
			switch (state) {
				case 0: {
					ret = FrameHandler::getDevListResponse(ids);
				} break;
				case 1: {
					ret = FrameHandler::getStatResponse(true, r_stat);
				} break;
				case 2: {
                    ret = r_ctl;
				} break;
				default: {} break;
			}
			emit this->dataIsReady( encodeData(ret) );
		}, QueuedUniqueConnection);

		for (int i = 0; i < 7; ++i) {
            stat[i].m.id = i;
            stat[i].m.ch.devEn = true;
            stat[i].m.ch.signalLvl = 99;
			ids.push_back(i);
		}
	}
	virtual ~SlaveEmulator() = default;

	virtual void write(const QByteArray &data) override { h->parse( decodeData(data) ); }
    virtual void shutdown() override {}

signals:

private:
	int state;
	QVector_u16 ids;
    QMap <int, DevStruct::DeviceStat> stat;

	DevStruct::DeviceStat r_stat;
    QByteArray r_ctl;
	QTimer *timer;
	FrameHandler *h;

	inline void nextState(int s, int delay) { state = s; timer->start(delay); }

	QByteArray encodeData(const QByteArray &d) {
		return d;
	}

	QByteArray decodeData(const QByteArray &d) {
		return d;
	}
};

#endif /* SLAVEEMULATOR_H */
