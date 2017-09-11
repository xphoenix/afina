#ifndef AFINA_STORAGE_H
#define AFINA_STORAGE_H

#include <string>

namespace Afina {

/**
 *
 */
class Storage {
public:
    Storage() {}
    virtual ~Storage() {}

    virtual void Start() {}
    virtual void Stop() {}

    virtual const std::string &Put(const std::string &key, const std::string &value) = 0;

    virtual const std::string &PutIfAbsent(const std::string &key, const std::string &value) = 0;

    virtual bool Get(const std::string &key, std::string &value) = 0;
};

} // namespace Afina

#endif // AFINA_STORAGE_H
