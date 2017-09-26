//Error.h - исключение, которое должен
// бросать аллокатор при возникновении ошибок, например когда больше нет памяти.
// Сюда можно добавлять новые типы ошибок, если нужно

#ifndef AFINA_ALLOCATOR_ERROR_H
#define AFINA_ALLOCATOR_ERROR_H

#include <stdexcept>
#include <string>
#include <cstring>

namespace Afina {
namespace Allocator {

enum class AllocErrorType {
    InvalidFree,
    NoMemory,
};

class AllocError : std::runtime_error {
private:
    AllocErrorType type;

public:
    AllocError(AllocErrorType _type, std::string message) :
            runtime_error(message),
            type(_type) {

    }

    AllocErrorType getType() const { return type; }
};

} // namespace Allocator
} // namespace Afina

#endif //  AFINA_ALLOCATOR_ERROR_H
