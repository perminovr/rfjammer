#include "rfjammersctl.h"
#include "slaveemulator.h"
#include "spibus.h"
#include "si4463.h"
#include <iostream>


#define USE_SLAVE_EMULATOR 0


static QByteArray encodeData(const QByteArray &d)
{
	return d;
}


static QByteArray decodeData(const QByteArray &d)
{
	return d;
}


void RfJammersCtl::setRadioParams(const ServStruct::RadioParams &d)
{
	rprms.self.timeoutAck = (d.timeoutAck)? d.timeoutAck : 1;
	rprms.self.timeoutRepeat = (d.timeoutRepeat)? d.timeoutRepeat : 1;
	rprms.self.slavePause = (d.slavePause)? d.slavePause : 1;
	rprms.self.pollPause = (d.pollPause)? d.pollPause : 1;
	rprms.repeatTotal = scast(quint16, rprms.self.timeoutRepeat) / scast(quint16, rprms.self.timeoutAck);
	if (!timer->isActive()) {
		setNextState(State::Init, 0);
	}
}


void RfJammersCtl::setTimeoutHandler(FrameData &fd, void (RfJammersCtl::*h)(int))
{
	using std::placeholders::_1;
	fd.timeoutHandler = std::bind(h, this, _1);
}


void RfJammersCtl::handleDevStatTimeout(int id)
{
	auto it = devStats.find(id);
	auto d = (it != devStats.end())? *it : DevStruct::DeviceStat();
	d.getM(id) = id;
	emit devicesStatReceived(false, d);
}


void RfJammersCtl::handleDevListTimeout(int u)
{
	Q_UNUSED(u)
	emit devicesListReceived( QVector_u16() );
}


void RfJammersCtl::requestDevicesList()
{
	FrameData fd;
	fd.d = encodeData(fhandler->getDevListRequest());
	// setTimeoutHandler(fd, &RfJammersCtl::handleDevListTimeout);
	once.list.push_back(fd);
}


void RfJammersCtl::requestDevStat(quint16 id)
{
	FrameData fd;
	fd.id = id;
	fd.d = encodeData(fhandler->getDataRequest(FrameHandler::DataType::Stat, id));
	setTimeoutHandler(fd, &RfJammersCtl::handleDevStatTimeout);
	once.list.push_back(fd);
}


void RfJammersCtl::setDevicesCtl(const DevStruct::DeviceCtl &d)
{
	FrameData fd;
    auto dcopy = d;
    dcopy.getM(packetId) = getGlobalPacketId();
    fd.id = dcopy.getM(id);
    fd.d = encodeData(fhandler->getCtlResponse(true, dcopy));
    fd.packetId = dcopy.getM(packetId);
	once.list.push_back(fd);
}


void RfJammersCtl::setSubscribtionsList(const QVector_u16 &ids)
{
	poll.newSubsList = ids; // on Init for sync
	poll.update = true;
}


void RfJammersCtl::rmCurAndSetNextOnceReq()
{
	auto del = once.cur;
	if (del != once.list.end()) {
		once.cur++;
		once.list.erase(del);
	}
}


void RfJammersCtl::setNextState(State state, int delay)
{
	this->state = state;
	this->timer->start(delay);
}


void RfJammersCtl::handleReceiveEvent()
{
	switch (state) {
		case State::PollTimeout: {
			setNextState(State::Init, rprms.self.slavePause);
		} break;
		case State::OnceTimeout: {
			rmCurAndSetNextOnceReq();
			setNextState(State::OnceNext, rprms.self.slavePause);
		} break;
		case State::Init:
		case State::OnceNext: {
			// unexpected -> waiting
			setNextState(State::Init, rprms.self.pollPause + rprms.self.timeoutRepeat);
		} break;
	}
}


uint8_t RfJammersCtl::getGlobalPacketId()
{
	uint8_t ret = this->globalPacketId++;
	if (ret == 0) { ret = this->globalPacketId++; }
	return ret;
}


RfJammersCtl::RfJammersCtl(QObject *parent) : QObject(parent)
{
	globalPacketId = 0;

	fhandler = new FrameHandler(this);
	{
		connect(fhandler, &FrameHandler::statResponseFound, this, [this](bool valid, const DevStruct::DeviceStat &d, void *sender){
            Q_UNUSED(sender)
			devStats[d.getM(id)] = d;
			handleReceiveEvent();
            std::cout << " - statResponseFound id:" << std::hex << d.getM(id) << std::endl;
            emit devicesStatReceived(valid, d);
		});
		connect(fhandler, &FrameHandler::devListResponseFound, this, [this](const QVector_u16 &ids, void *sender){
			Q_UNUSED(sender)
			handleReceiveEvent();
			emit devicesListReceived(ids);
		});
		connect(fhandler, &FrameHandler::ctlResponseFound, this, [this](bool valid, const DevStruct::DeviceCtl &d, void *sender){ // means ctrl ack
			Q_UNUSED(valid)
			Q_UNUSED(sender)
            auto f = once.cur;
            std::cout << " - ctlResponseFound " << std::hex << d.getM(id) << std::endl;
			if (f->id == d.getM(id) && f->packetId == d.getM(packetId)) {
				handleReceiveEvent();
			}
		});
	}

#if USE_SLAVE_EMULATOR

	chip = nullptr;
	config = nullptr;
	radio = new SlaveEmulator(this);
    connect(radio, &SlaveEmulator::dataIsReady, this, [this](const QByteArray &data) {
		fhandler->parse( decodeData(data) );
	}, QueuedUniqueConnection);

#else

	chip = new Si4463(new SpiBus(CONFIG_SPIDEV_NAME));
	Si4463::Config *cfg = new Si4463::Config();
	config = cfg;
    cfg->irqPinNum = CONFIG_SI4463_IRQ_GPIO;
    cfg->ctsPinNum = CONFIG_SI4463_CTS_GPIO;
    cfg->sdnPinNum = CONFIG_SI4463_SDN_GPIO;
    cfg->csPinNum = CONFIG_SI4463_CS_GPIO;
    radio = new RadioHandler(chip, cfg, this);
	connect(radio, &RadioHandler::dataIsReady, this, [this](const QByteArray &data) {
        // si4463 driver receives 32 bytes everytime. lets compute size
		quint16 size = ((quint16)data[4]) + ((quint16)data[5] << 8);
        QByteArray d(data, size + 6);
		fhandler->parse( decodeData(d) );
	}, QueuedUniqueConnection);

#endif

	timer = new QTimer(this);
	timer->setSingleShot(true);
	/* Main Automat */
	connect(timer, &QTimer::timeout, this, [this](){
		switch (state) {
			case State::Init: {
				if (poll.update) { // set/update subscription list
					poll.update = false;
					poll.list.clear();
					for (auto &x : poll.newSubsList) { // copy list
						FrameData fd;
						fd.id = x;
						fd.d = encodeData(fhandler->getDataRequest(FrameHandler::DataType::Stat, x));
						setTimeoutHandler(fd, &RfJammersCtl::handleDevStatTimeout);
						poll.list.push_back(fd);
					}
					poll.cur = poll.list.end(); // restart from begining
				}
				if (!poll.list.isEmpty()) {
					if (poll.cur == poll.list.end()) {
						poll.cur = poll.list.begin(); // on new poll / on sublist reset
					} else {
						poll.cur++; // on next poll step
					}
					if (poll.cur != poll.list.end()) { // check for poll step
						repeatCnt = -1; // to send at least once
						setNextState(State::PollTimeout, 0);
					} else {
						once.cur = once.list.begin(); // reset on the 'once' part begining
						setNextState(State::OnceNext, rprms.self.slavePause); // end poll -> goto 'once' part
					}
				} else {
					once.cur = once.list.begin(); // reset on the 'once' part begining
					setNextState(State::OnceNext, 0);
				}
			} break;
			case State::PollTimeout: {
				if (repeatCnt < scast(int, rprms.repeatTotal)) { // repeat
					repeatCnt++;
                    std::cout << "send poll id:"
                              << std::hex << poll.cur->id << " sz:"
                              << std::dec << poll.cur->d.size()
                              << std::endl;
					radio->write( poll.cur->d );
					setNextState(State::PollTimeout, rprms.self.timeoutAck); // wait for frame/timeout
				} else { // timeout
					poll.cur->timeoutHandler(poll.cur->id);
					setNextState(State::Init, rprms.self.slavePause); // next poll step
				}
			} break;
			case State::OnceNext: {
				if (once.cur == once.list.end()) {
					setNextState(State::Init, rprms.self.pollPause); // start from poll
				} else {
					repeatCnt = -1; // to send at least once
					setNextState(State::OnceTimeout, 0);
				}
			} break;
			case State::OnceTimeout: {
				auto f = once.cur;
				if (repeatCnt < scast(int, rprms.repeatTotal)) { // repeat
                    repeatCnt++;
                    std::cout << "send once id:"
                              << std::hex << f->id << " sz:"
                              << std::dec << f->d.size()
                              << std::endl;
					radio->write( f->d );
					setNextState(State::OnceTimeout, rprms.self.timeoutAck);
				} else { // timeout
					if (f->timeoutHandler) {
						f->timeoutHandler(f->id);
						rmCurAndSetNextOnceReq(); // do not remove if handler unspecified
					} else {
						once.cur++; // on next 'once' part step
					}
					setNextState(State::OnceNext, rprms.self.slavePause); // next step
				}
			} break;
		}
	}, QueuedUniqueConnection);
}


RfJammersCtl::~RfJammersCtl()
{
	radio->shutdown();
	delete chip;
    delete config;
}
