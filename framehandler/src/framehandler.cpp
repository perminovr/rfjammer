#include "framehandler.h"

enum CmdId {
	Data = 1,
	DevicesIdList,
	Subscribe,
	RadioPrms,
};

enum GetSet { Get, Ack = Get, Set };


void FrameHandler::parse(const QByteArray &data, void *sender)
{
	int sz = data.size();
	if (sz > 0) {
		auto d = (const quint8 *)data.constData();
		int i = 0;
		#define dto2() (*((quint16 *)(d+i)))
		#define error() { i = sz; break; }
		while (i < sz) {
			CmdId cmd = scast(CmdId, d[i++]);
			switch (cmd) {

				default: error();
			}
		}
	}
}


#define pb(d) append((char)((int)d))
#define pb2(d) append((const char *)(d),2)
#define pbA(d,s) append((const char *)(d),s)


QByteArray FrameHandler::getDataRequest(int dt, int id)
{
	QByteArray ret;
	return ret;
}


QByteArray FrameHandler::getStatResponse(bool valid, const DevStruct::DeviceStat &d)
{
	QByteArray ret;
	return ret;
}


QByteArray FrameHandler::getCtlResponse(bool valid, const DevStruct::DeviceCtl &d)
{
	QByteArray ret;
	return ret;
}


QByteArray FrameHandler::getDevListRequest()
{
	quint16 sz = 0;
	QByteArray ret;
	return ret;
}


QByteArray FrameHandler::getDevListResponse(const QVector_u16 &ids)
{
	quint16 sz = ids.size();
	QByteArray ret;
	return ret;
}


QByteArray FrameHandler::getSubscribeResponse(bool sub, const QVector_u16 &ids)
{
	quint16 sz = ids.size();
	QByteArray ret;
	return ret;
}


QByteArray FrameHandler::getRadioPrmsResponse(const ServStruct::RadioParams &d)
{
	QByteArray ret;
	return ret;
}
