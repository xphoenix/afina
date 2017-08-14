#include <iostream>
#include <memory>

#include <afina/Storage.h>
#include <afina/Version.h>

#include "network/Server.h"
#include "storage/MapBasedGlobalLockImpl.h"

int main(int argc, char **argv) {
    std::cout << "Starting Afina " << Afina::Version_Major << "." << Afina::Version_Minor << "."
              << Afina::Version_Patch;
    if (Afina::Version_SHA.size() > 0) {
        std::cout << "-" << Afina::Version_SHA;
    }
    std::cout << std::endl;

    // Build new storage instance
    std::shared_ptr<Afina::Storage> pBackend = std::make_shared<Afina::Backend::MapBasedGlobalLockImpl>();

    // Build  & start network layer
    Afina::Network::Server srv(pBackend);
    srv.Start(1);

    // TODO: Once network layer becomes multithreaded and running in background, main thread must be busy
    // with something until network server is working

    return 0;
}
