#ifndef FRAMEHANDLER_H
#define FRAMEHANDLER_H

#include <QObject>
#include <QVector>
#include <QByteArray>

#include "common.h"

typedef QVector<quint16> QVector_u16;

/*!
 * @class FrameHandler
 * @brief Обработчик кадров для Tcp и радио кадров.
 * @details По получению кадра вызывается @ref FrameHandler::parse , который
 * 		эмитит сигналы по обнаружению соотвествующих кадров. Для формирования
 * 		кадров используются статические методы
*/
class FrameHandler : public QObject
{
	Q_OBJECT
public:
	enum DataType { Stat = 1, Cntrl };

	FrameHandler(QObject *parent = nullptr) : QObject(parent) {
		qRegisterMetaType<DevStruct::DeviceStat>("DevStruct::DeviceStat");
		qRegisterMetaType<DevStruct::DeviceCtl>("DevStruct::DeviceCtl");
		qRegisterMetaType<ServStruct::RadioParams>("ServStruct::RadioParams");
		qRegisterMetaType<QVector_u16>("QVector_u16");
	}
	virtual ~FrameHandler() = default;

	static inline QByteArray getDataRequest(DataType dt, quint16 id) { return getDataRequest(scast(int, dt), scast(int, id)); }
	static QByteArray getStatResponse(bool valid, const DevStruct::DeviceStat &d);
	static QByteArray getCtlResponse(bool valid, const DevStruct::DeviceCtl &d);

	static QByteArray getDevListRequest();
	static QByteArray getDevListResponse(const QVector_u16 &ids);

	static QByteArray getSubscribeResponse(bool sub, const QVector_u16 &ids);

	static QByteArray getRadioPrmsResponse(const ServStruct::RadioParams &d);

	void parse(const QByteArray &data, void *sender = nullptr);

signals:
	void dataRequestFound(int dt, int id, void *sender);
	void statResponseFound(bool valid, const DevStruct::DeviceStat &d, void *sender);
	void ctlResponseFound(bool valid, const DevStruct::DeviceCtl &d, void *sender);

	void devListRequestFound(void *sender);
	void devListResponseFound(const QVector_u16 &ids, void *sender);

	void subscribeResponseFound(bool sub, const QVector_u16 &ids, void *sender);

	void radioPrmsResponseFound(const ServStruct::RadioParams &d, void *sender);

protected:
	static QByteArray getDataRequest(int dt, int id);
};

#endif /* FRAMEHANDLER_H */
