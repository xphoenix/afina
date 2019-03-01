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

    /**
     * Stores association between given key/value pair.
     * If key is already present in storage then replace existing value by
     * the new one.
     *
     * Method returns true if success and false in case of any error. Once
     * method returns true any subsequent access to storage must indicates that
     * key->value association exists
     *
     * @param key to be associated with value
     * @param value to be assigned for the key
     */
    virtual bool Put(const std::string &key, const std::string &value) = 0;

    /**
     * Stores association between given key/value pair if key isn't present in
     * storage.
     *
     * If given key is already present in the storage then method resturns false
     * and doesn't change anything inside. Otherwise new association key->value
     * created and if successfull then true returns.
     *
     * @param key to be associated with value
     * @param value to be assigned for the key
     */
    virtual bool PutIfAbsent(const std::string &key, const std::string &value) = 0;

    /**
     * Updates existing association between given key/value pair
     * If requested key doesn't present in storage method returns false and
     * doesnt change anything.
     *
     * If given key found then existing association gets update to point to
     * the given value.
     *
     * @param key to be associated with value
     * @param value to be assigned for the key
     */
    virtual bool Set(const std::string &key, const std::string &value) = 0;

    /**
     * Removes association for the given key
     * If requested key doesn't present in storage method returns false and
     * doesnt change anything.
     *
     * If given key found then existing association gets deleted and method
     * returns true.
     *
     * Once method returns true any subsequent call to the storage must not
     * see deleted association until it created again by Put or PutIfAdsent
     * calls
     *
     * @param key to be removed
     */
    virtual bool Delete(const std::string &key) = 0;

    /**
     * Retrive key for the given value
     * If there is an association for the given key then method copies value
     * into given output parameter (possibly extends its size) and return true
     *
     * In case if given key not found method returns false and doesn't perform
     * any changes on the output parameter
     *
     * @param key to retrive1 value for
     * @param value output parameter to copy value to
     */
    virtual bool Get(const std::string &key, std::string &value) = 0;
};

} // namespace Afina

#endif // AFINA_STORAGE_H
