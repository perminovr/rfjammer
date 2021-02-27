#ifndef QEPOLL_H
#define QEPOLL_H

#include <QObject>
#include <QList>
#include <QThread>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>

/*!
 * @class QEpoll
 * @brief linux epoll qt class; move it to qthread
*/
class QEpoll : public QObject
{
	Q_OBJECT
public:
	/*!
	* @struct EpollDesc
	* @brief Описатель epoll события
	*/
	struct EpollDesc {
		int fd = -1;
		uint32_t events = EPOLLIN;
		bool add = true;
	};
	/*!
	* @fn QEpoll::toEpoll
	* @brief add or del to epoll desc
	*/
	void toEpoll(const EpollDesc &ed) {
		epollDescQue.push_back(ed);
		raiseEv(evUpd);
	}

	QEpoll(QObject *parent = nullptr) : QObject(parent) {
		evUpd = eventfd(0,0);
		evKill = eventfd(0,0);
		epfd = epoll_create(1);
		p_toEpoll(evUpd, EPOLLIN, true);
		p_toEpoll(evKill, EPOLLIN, true);
		cnt = 0;
		runnning = false;
		epollDescQue.reserve(8);
		connect(this, &QEpoll::p_nextJob, this, &QEpoll::doJob, Qt::QueuedConnection);
		qRegisterMetaType<uint32_t>("uint32_t");
	}
	virtual ~QEpoll() {
		close(epfd);
		close(evUpd);
		close(evKill);
	}

public slots:
	void run() {
		if (!runnning) {
			runnning = true;
			emit p_nextJob();
		}
	}
	void stop() const {
		if (runnning) {
			raiseEv(evKill);
		}
	}

signals:
	void eventsOccured(int fd, uint32_t events);

private slots:
	void doJob() {
		if (this->runnning) {
			struct epoll_event ev;
            int res = epoll_wait(this->epfd, &ev, 1, -1);
            if (res > 0 && ev.events) {
				if (ev.data.fd == evUpd) {
					for (auto &x : epollDescQue) {
						p_toEpoll(x.fd, x.events, x.add);
					}
					epollDescQue.clear();
					endEv(evUpd);
				} else if (ev.data.fd == evKill) {
					this->runnning = false;
					endEv(evKill);
					return; // done
                } else {
					emit eventsOccured(ev.data.fd, ev.events);
				}
			} else { usleep(1*1000); }
			emit p_nextJob();
		}
	}

signals:
	void p_nextJob();

private:
	bool runnning;	//!< 
	int evUpd;		//!< 
	int evKill;		//!< 
	int epfd;		//!< 
	int cnt;		//!< 
	QList <EpollDesc> epollDescQue;	//!< 

	ssize_t raiseEv(int fd) const {
		uint8_t buf = 1;
		return write(fd, &buf, 8);
	}
	ssize_t endEv(int fd) const {
		uint8_t buf[8];
		return read(fd, &buf, 8);
	}
	void p_toEpoll(int fd, uint32_t events, bool add) const {
		struct epoll_event ev;
		bzero(&ev, sizeof(ev));
		ev.data.fd = fd;
		ev.events = events;
		epoll_ctl(this->epfd, ((add)? EPOLL_CTL_ADD : EPOLL_CTL_DEL), fd, &ev);
	}
};

#endif /* QEPOLL_H */
