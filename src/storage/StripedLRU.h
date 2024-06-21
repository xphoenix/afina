#ifndef AFINA_STORAGE_STRIPED_LRU_H
#define AFINA_STORAGE_STRIPED_LRU_H

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include <afina/Storage.h>

#include "ThreadSafeSimpleLRU.h"

namespace Afina {
namespace Backend {

static std::size_t min_size_stripe;

class StripedLRU : public Afina::Storage {
private:
    StripedLRU(std::size_t stripe_limit, std::size_t stripes_cnt)
        : _stripes_cnt(stripes_cnt)  {
            this->_cashes.reserve(stripes_cnt);
            for (size_t i = 0; i < stripes_cnt; i++) {
                _cashes.emplace_back(new ThreadSafeSimplLRU(stripe_limit));
            }
        }

public:
    ~StripedLRU() {}
    static std::unique_ptr<StripedLRU>
    BuildStripedLRU(std::size_t memory_limit =
            16 * StripedLRU::min_size_stripe, std::size_t stripes_cnt = 4) {
        std::size_t stripe_limit = memory_limit / stripes_cnt;

        if (stripe_limit < StripedLRU::min_size_stripe) {
            throw std::runtime_error("Attention! Stripe size is less then minimum required.");
        }

        return std::move(std::unique_ptr<StripedLRU>(new StripedLRU(stripe_limit, stripes_cnt)));
    }

    bool Put(const std::string &key, const std::string &value) override;

    bool PutIfAbsent(const std::string &key, const std::string &value) override;

    bool Set(const std::string &key, const std::string &value) override;

    bool Delete(const std::string &key) override;

    bool Get(const std::string &key, std::string &value) override;

private:
    std::vector<std::unique_ptr<ThreadSafeSimplLRU>> _cashes;
    std::hash<std::string> _hash;
    std::size_t _stripes_cnt;
};

std::size_t StripedLRU::min_size_stripe = 1024UL * 1024;

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_STRIPED_LRU_H
