#ifndef RFJAMMERSCTL_H
#define RFJAMMERSCTL_H

#include "radiohandler.h"
#include "framehandler.h"
#include <QTimer>
#include <QLinkedList>
#include <QMap>
#include <functional>

/*!
 * @class RfJammersCtl
 * @brief Класс управления радио устройствами. Обеспечивает автомат
 * отправки и получения сообщений
*/
class RfJammersCtl : public QObject
{
	Q_OBJECT
public:
	RfJammersCtl(QObject *parent = nullptr);
	virtual ~RfJammersCtl();

public slots:
	void setRadioParams(const ServStruct::RadioParams &d);
	/*!
	* @fn RfJammersCtl::setSubscribtionsList
	* @brief Устанока списка подписок на события от радио устройств.
	* @param ids - идентификаторы устройств
	*/
	void setSubscribtionsList(const QVector_u16 &ids);

	void requestDevicesList();
	void requestDevStat(quint16 id);
	void setDevicesCtl(const DevStruct::DeviceCtl &d);

signals:
	void devicesListReceived(const QVector_u16 &ids);
	void devicesStatReceived(bool valid, const DevStruct::DeviceStat &d);

private:
	uint8_t globalPacketId;			//!< packet iterator
	IRadioChip *chip;				//!< 
	IRadioChip::Config *config;		//!< 
    IRadioHandler *radio;			//!< 
	FrameHandler *fhandler;			//!< 

	struct {
		ServStruct::RadioParams self;
		quint16 repeatTotal;		//!< counted total repeat number
	} rprms;

	QMap <int, DevStruct::DeviceStat> devStats; //!< storage for devices stats

	QTimer *timer;
	enum State {
		Init, PollTimeout, OnceNext, OnceTimeout
	} state;
	struct FrameData {
		int id;			 	//!< dev id
		uint8_t packetId; 	//!< 
		QByteArray d;		//!< prepared data
		std::function <void(int)> timeoutHandler = nullptr;
	};
	struct {
		bool update;
		QVector_u16 newSubsList;
		QLinkedList <FrameData> list;
		QLinkedList <FrameData>::const_iterator cur;
	} poll;	//!< cycled requests
	struct {
		QLinkedList <FrameData> list;
		QLinkedList <FrameData>::iterator cur;
	} once; //!< single requests
	int repeatCnt;	//!< current repeat counter

	uint8_t getGlobalPacketId();

	void setNextState(State state, int delay);
	void handleReceiveEvent();
	void rmCurAndSetNextOnceReq();

	void setTimeoutHandler(FrameData &fd, void (RfJammersCtl::*h)(int));
	void handleDevStatTimeout(int id);
	void handleDevListTimeout(int u);
};

#endif /* RFJAMMERSCTL_H */
