#ifndef AFINA_STRIPEDLRU_H
#define AFINA_STRIPEDLRU_H

#include <functional>
#include <string>
#include <vector>

#include "ThreadSafeSimpleLRU.h"

namespace Afina {
namespace Backend {

class StripedLRU : public Afina::Storage {
public:
    explicit StripedLRU(size_t max_size = 1024, size_t strip_count = 1);
    ~StripedLRU();

    bool Put(const std::string &key, const std::string &value) override;

    bool PutIfAbsent(const std::string &key, const std::string &value) override;

    bool Set(const std::string &key, const std::string &value) override;

    bool Delete(const std::string &key) override;

    bool Get(const std::string &key, std::string &value) override;

private:
    std::vector<ThreadSafeSimplLRU> shards;
    size_t strip_count;
    std::hash<std::string> hasher;
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STRIPEDLRU_H
