#include "Worker.h"

#include <cassert>
#include <functional>
#include <iostream>

#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <spdlog/logger.h>

#include <afina/logging/Service.h>

#include "Connection.h"
#include "Utils.h"

namespace Afina {
namespace Network {
namespace MTnonblock {

// See Worker.h
Worker::Worker(std::shared_ptr<Afina::Storage> ps, std::shared_ptr<Afina::Logging::Service> pl)
    : _pStorage(ps), _pLogging(pl), isRunning(false), _epoll_fd(-1) {
    // TODO: implementation here
}

// See Worker.h
Worker::~Worker() {
    // TODO: implementation here
}

// See Worker.h
Worker::Worker(Worker &&other) { *this = std::move(other); }

// See Worker.h
Worker &Worker::operator=(Worker &&other) {
    _pStorage = std::move(other._pStorage);
    _pLogging = std::move(other._pLogging);
    _logger = std::move(other._logger);
    _thread = std::move(other._thread);
    _epoll_fd = other._epoll_fd;

    other._epoll_fd = -1;
    return *this;
}

// See Worker.h
void Worker::Start(int epoll_fd) {
    if (isRunning.exchange(true) == false) {
        assert(_epoll_fd == -1);
        _epoll_fd = epoll_fd;
        _logger = _pLogging->select("network.worker");
        _thread = std::thread(&Worker::OnRun, this);
    }
}

// See Worker.h
void Worker::Stop() { isRunning = false; }

// See Worker.h
void Worker::Join() {
    assert(_thread.joinable());
    _thread.join();
}

// See Worker.h
void Worker::OnRun() {
    assert(_epoll_fd >= 0);
    _logger->trace("OnRun");

    // Process connection events
    //
    // Do not forget to use EPOLLEXCLUSIVE flag when register socket
    // for events to avoid thundering herd type behavior.
    int timeout = -1;
    std::array<struct epoll_event, 64> mod_list;
    while (isRunning) {
        int nmod = epoll_wait(_epoll_fd, &mod_list[0], mod_list.size(), timeout);
        _logger->debug("Worker wokeup: {} events", nmod);

        for (int i = 0; i < nmod; i++) {
            struct epoll_event &current_event = mod_list[i];

            // nullptr is used by server for event_fd "interface", if we got here then server
            // signals us to wakeup to process some state change, ignore it in INNER loop, react
            // on changes in OUTHER loop
            if (current_event.data.ptr == nullptr) {
                continue;
            }

            // Some connection gets new data
            Connection *pconn = static_cast<Connection *>(current_event.data.ptr);
            if ((current_event.events & EPOLLERR) || (current_event.events & EPOLLHUP)) {
                pconn->OnError();
            } else if (current_event.events & EPOLLRDHUP) {
                pconn->OnClose();
            } else {
                // Depends on what connection wants...
                if (current_event.events & EPOLLIN) {
                    pconn->DoRead();
                }
                if (current_event.events & EPOLLOUT) {
                    pconn->DoWrite();
                }
            }

            // Rearm connection
            if (pconn->isAlive()) {
                pconn->_event.events |= EPOLLONESHOT;
                if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, pconn->_socket, &pconn->_event)) {
                    pconn->OnError();
                    delete pconn;
                }
            }
            // Or delete closed one
            else {
                if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, pconn->_socket, &pconn->_event)) {
                    std::cerr << "Failed to delete connection!" << std::endl;
                }
                delete pconn;
            }
        }
        // TODO: Select timeout...
    }
    _logger->warn("Worker stopped");
}

} // namespace MTnonblock
} // namespace Network
} // namespace Afina
