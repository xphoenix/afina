#include "StripedLRU.h"

namespace Afina {
namespace Backend {

bool StripedLRU::Put(const std::string &key, const std::string &value) {
    return this->_cashes[this->_hash(key) % this->_stripes_cnt]->Put(key, value);
}

bool StripedLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    return this->_cashes[this->_hash(key) % this->_stripes_cnt]->PutIfAbsent(key, value);
}

bool StripedLRU::Set(const std::string &key, const std::string &value) {
    return this->_cashes[this->_hash(key) % this->_stripes_cnt]->Set(key, value);
}

bool StripedLRU::Delete(const std::string &key) {
    return this->_cashes[this->_hash(key) % this->_stripes_cnt]->Delete(key);
}

bool StripedLRU::Get(const std::string &key, std::string &value) {
    return this->_cashes[this->_hash(key) % this->_stripes_cnt]->Get(key, value);
}

} // namespace Backend
} // namespace Afina
