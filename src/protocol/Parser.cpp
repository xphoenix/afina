#include "Parser.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

#include <afina/execute/Add.h>
#include <afina/execute/Append.h>
#include <afina/execute/Command.h>
#include <afina/execute/Delete.h>
#include <afina/execute/Get.h>
#include <afina/execute/Set.h>

namespace Afina {
namespace Protocol {

// See Parse.h
bool Parser::Parse(const char *input, const size_t size, size_t &parsed) {
    size_t pos;
    bool negative;
    std::string curKey;

    bytes = 0;
    bool parse_complete = false;
    for (pos = 0; pos < size && !parse_complete; pos++) {
        char c = input[pos];

        switch (state) {
        case State::sName: {
            if (c == ' ') {
                // std::cout << "parser debug: name='" << name << "'" << std::endl;
                if (name == "set" || name == "add" || name == "append" || name == "prepend") {
                    state = State::spKey;
                } else if (name == "get" || name == "gets") {
                    state = State::sgKey;
                } else {
                    throw std::runtime_error("Unknown command name");
                }
            } else {
                name.push_back(c);
            }
            break;
        }

        case State::spKey: {
            if (c == ' ') {
                state = State::spFlags;
                keys.push_back(curKey);
                // std::cout << "parser debug: key[" << keys.size() - 1 << "]='" << curKey << "'" << std::endl;
            } else {
                curKey.push_back(c);
            }
            break;
        }

        case State::sgKey: {
            if (c == '\r') {
                keys.push_back(curKey);
                // std::cout << "parser debug: total '" << keys.size() << " keys" << std::endl;

                if (keys.size() == 0) {
                    throw std::runtime_error("Client provides no key to retrive");
                }

                curKey.clear();
                state = State::sLF;
            } else if (c == ' ') {
                // std::cout << "parser debug: key[" << keys.size() << "]='" << curKey << "'" << std::endl;
                state = State::sgKey;
                keys.push_back(curKey);
                curKey.clear();
            } else {
                curKey.push_back(c);
            }
            break;
        }

        case State::spFlags: {
            if (c == ' ') {
                negative = false;
                state = State::spExprTimeStart;
                // std::cout << "parser debug: flags='" << flags << "'" << std::endl;
            } else if (c >= '0' && c <= '9') {
                uint32_t f = (flags * 10) + (c - '0');
                if (f < flags) {
                    // Overflow
                    throw std::runtime_error("Flags field overflow");
                }
                flags = f;
            }
            break;
        }

        case State::spExprTimeStart: {
            if (c == '-') {
                negative = true;
                state = State::spExprTime;
            } else if (c >= '0' && c <= '9') {
                exprtime = (c - '0');
                state = State::spExprTime;
            }
            break;
        }

        case State::spExprTime: {
            if (c == ' ') {
                state = State::spBytes;
                // std::cout << "parser debug: ExprTime='" << exprtime << "'" << std::endl;
            } else if (c >= '0' && c <= '9') {
                int32_t et = exprtime;
                if (negative) {
                    et -= (c - '0');
                    if (et > exprtime) {
                        throw std::runtime_error("Expire time field overflow");
                    }
                } else {
                    et += (c - '0');
                    if (et < exprtime) {
                        throw std::runtime_error("Expire time field overflow");
                    }
                }
                exprtime = et;
            }
            break;
        }

        case State::spBytes: {
            if (c == '\r') {
                state = State::sLF;
                // std::cout << "parser debug: bytes='" << bytes << "'" << std::endl;
            } else if (c >= '0' && c <= '9') {
                uint32_t b = (bytes * 10) + (c - '0');
                if (b < bytes) {
                    // Overflow
                    throw std::runtime_error("Bytes field overflow");
                }
                bytes = b;
            }
            break;
        }

        case State::sLF: {
            if (c == '\n') {
                parse_complete = true;
            } else {
                std::stringstream err;
                err << "Invalid char " << (int)c << " at position " << (parsed + pos) << ", \\n expected";
                throw std::runtime_error(err.str());
            }
            break;
        }

        default:
            throw std::runtime_error("Unknown state");
        }
    }

    parsed += pos;
    return state == State::sLF;
}

// See Parse.h
std::unique_ptr<Execute::Command> Parser::Build(uint32_t &body_size) const {
    if (state != State::sLF) {
        return std::unique_ptr<Execute::Command>(nullptr);
    }

    body_size = bytes;
    if (name == "set") {
        return std::unique_ptr<Execute::Command>(new Execute::Set(keys[0], flags, exprtime));
    } else if (name == "add") {
        return std::unique_ptr<Execute::Command>(new Execute::Add(keys[0], flags, exprtime));
    } else if (name == "append") {
        return std::unique_ptr<Execute::Command>(new Execute::Append(keys[0], flags, exprtime));
    } else if (name == "get") {
        return std::unique_ptr<Execute::Command>(new Execute::Get(keys));
    } else {
        throw std::runtime_error("Unsupported command");
    }
}

// See Parse.h
void Parser::Reset() {
    state = State::sName;
    name.clear();
    keys.clear();
    flags = 0;
    bytes = 0;
    exprtime = 0;
}

} // namespace Protocol
} // namespace Afina
