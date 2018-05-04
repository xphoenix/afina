#ifndef AFINA_EXECUTE_APPENDPREPEND_H
#define AFINA_EXECUTE_APPENDPREPEND_H

#include <cstdint>
#include <string>

#include "InsertCommand.h"

namespace Afina {
namespace Execute {

/**
 * # Append/Prepend data for the key
 * Append new data to (the end of)/(before the) value for the given key. If key wasn't found
 * then command does nothing
 *
 * Command must write result to the output, which could be:
 * - "STORED", to indicate success.
 * - "NOT_STORED" to indicate the data was not stored, but not because of an
 * error. This normally means that the condition for the command wasn't met.
 */
class AppendPrepend : public InsertCommand {
protected:
    bool _type;
public:
    //true - append, false - prepend
    AppendPrepend(bool type) : _type(type) {}

    void Execute(Storage &storage, const std::string &args, std::string &out) const override;
};

} // namespace Execute
} // namespace Afina

#endif // AFINA_EXECUTE_APPENDPREPEND_H
