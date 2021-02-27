#include "radiohandler.h"
#include <QThread>
#include <unistd.h>


#define scast(t,v) static_cast<t>(v)
#define QueuedUniqueConnection \
	scast(Qt::ConnectionType, (scast(int, Qt::UniqueConnection) | scast(int, Qt::QueuedConnection)))
#define DirectUniqueConnection \
	scast(Qt::ConnectionType, (scast(int, Qt::UniqueConnection) | scast(int, Qt::DirectConnection)))


void RadioHandler::shutdown()
{
	thread->quit();
	thread->wait();
    delete thread;
	destroyed = true;
}


RadioHandler::RadioHandler(IRadioChip *chip, const IRadioChip::Config *config, QObject *parent) : IRadioHandler(parent)
{
    destroyed = false;
    thread = new QThread();
	connect(chip, &IRadioChip::dataIsReady, this, &RadioHandler::dataIsReady, QueuedUniqueConnection);
	connect(this, &RadioHandler::p_write, chip, &IRadioChip::sendData, QueuedUniqueConnection);
    connect(thread, &QThread::started, chip, [chip, config](){
        chip->init(config);
	}, QueuedUniqueConnection);
    chip->moveToThread(thread);
	thread->start();
}


RadioHandler::~RadioHandler()
{
	if (!destroyed) shutdown();
}

