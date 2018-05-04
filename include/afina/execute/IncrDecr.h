#ifndef AFINA_EXECUTE_INCRDECR_H
#define AFINA_EXECUTE_INCRDECR_H

#include <cstdint>
#include <string>

#include "KeyValueCommand.h"

namespace Afina {
namespace Execute {

/**
 * # Increment/Dectement data for the key
 * Commands "incr" and "decr" are used to change data for some item in-place, incrementing or decrementing it. The data for the item
 * is treated as decimal representation of a 64-bit unsigned integer.  If the current data value does not conform to such a representation,
 * the incr/decr commands return an error. Also, the item must already exist for incr/decr to work; these commands won't pretend
 * that a non-existent key exists with value 0; instead, they will fail.
 *
 * Command must write result to the output, which could be:
 * - "NOT_FOUND", to indicate the item with this value was not found
 * - "<value>" where <value> is the new value of the item's data, after the increment/decrement operation was carried out.
 */
class IncrDecr : public KeyValueCommand {
protected:
    bool _type;
public:
    //true - incr, false - decr
    IncrDecr(bool type) : _type(type) {}

    void Execute(Storage &storage, const std::string &args, std::string &out) const override;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_INCRDECR_H
