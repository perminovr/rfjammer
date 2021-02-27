#include "qtworker.h"
#include <QCoreApplication>
#include <QDir>

#include <initializer_list>
#include <signal.h>
#include <unistd.h>

static Q_DECL_UNUSED void catchUnixSignals(std::initializer_list<int> quitSignals)
{
	auto handler = [](int sig){
		Q_UNUSED(sig)
		QCoreApplication::exit(0);
	};

	sigset_t blocking_mask;
	sigemptyset(&blocking_mask);
	for (auto sig : quitSignals)
		sigaddset(&blocking_mask, sig);

	struct sigaction sa;
	bzero(&sa, sizeof(sa));
	sa.sa_handler = handler;
	sa.sa_mask = blocking_mask;

	for (auto sig : quitSignals)
		sigaction(sig, &sa, nullptr);
}

int main(int argc, char **argv)
{
	QCoreApplication app(argc, argv);

	catchUnixSignals({SIGINT, SIGABRT, SIGHUP, SIGTERM, SIGQUIT, SIGKILL});

	QtWorker *worker = new QtWorker(&app);
	worker->start();

	return app.exec();
}

