#include "qwframehandler.h"
#include <QDataStream>

enum CmdId {
	AllData = 1,
	RadioPrms,
	NetPrms,
	ServAddr,
	VpnAddr,
	VpnCfg,
	VpnEnable
};

enum GetSet { Get, Set };


#define popA(d,s) readRawData((char *)(d),s)


void QwFrameHandler::parse(const QByteArray &data, void *sender)
{
	QDataStream in(data);
	in.setVersion(QDataStream::Qt_5_3);
	if (data.size() > scast(int, sizeof(unsigned))) {
		unsigned blockSize;
		int icmd;
		int igs;
		while (!in.atEnd()) {
			in >> blockSize >> icmd >> igs;
			CmdId cmd = scast(CmdId, icmd);
			GetSet gs = scast(GetSet, igs);
			switch (cmd) {
				
				default: { break; }
			}
		}
	}
}


#define TO_PIPE_FTYPE_INIT() \
	QByteArray ret;\
	QDataStream out(&ret, QIODevice::WriteOnly);\
	out.setVersion(QDataStream::Qt_5_3);\
	out << unsigned(0);

#define TO_PIPE_FTYPE_PRERET() \
	out.device()->seek(0);\
	out << unsigned(ret.size()-sizeof(unsigned));

#define TO_BYTES_RET() \
	TO_PIPE_FTYPE_PRERET() \
	return ret;

#define pbA(d,s) writeRawData((const char *)(d),s)


QByteArray QwFrameHandler::getAllDataRequest()
{
	TO_PIPE_FTYPE_INIT()
	TO_BYTES_RET()
}


QByteArray QwFrameHandler::getAllDataResponse(const AllData &d)
{
	TO_PIPE_FTYPE_INIT()
	TO_BYTES_RET()
}


QByteArray QwFrameHandler::getRadioPrmsResponse(const ServStruct::RadioParams &d)
{
	TO_PIPE_FTYPE_INIT()
	TO_BYTES_RET()
}


QByteArray QwFrameHandler::getNetPrmsResponse(const DeviceNetParams &d)
{
	TO_PIPE_FTYPE_INIT()
	TO_BYTES_RET()
}


QByteArray QwFrameHandler::getServAddrResponse(const ServerAddr &d)
{
	TO_PIPE_FTYPE_INIT()
	TO_BYTES_RET()
}


QByteArray QwFrameHandler::getVpnAddrResponse(const QHostAddress &d)
{
	TO_PIPE_FTYPE_INIT()
	TO_BYTES_RET()
}


QByteArray QwFrameHandler::getVpnCfgResponse(const QString &d)
{
	TO_PIPE_FTYPE_INIT()
	TO_BYTES_RET()
}


QByteArray QwFrameHandler::getVpnEnableResponse(bool d)
{
	TO_PIPE_FTYPE_INIT()
	TO_BYTES_RET()
}

