#include <afina/Storage.h>
#include <afina/execute/IncrDecr.h>

#include <iostream>

namespace Afina {
namespace Execute {

void IncrDecr::Execute(Storage &storage, const std::string &args, std::string &out) const {
	if (_type == true) { std::cout << "Incr(" << _key << ")" << args << std::endl; }
	else               { std::cout << "Decr(" << _key << ")" << args << std::endl; }

    std::string value;
    if (!storage.Get(_key, value)) {
		if (!_no_reply) { out = "NOT_FOUND"; }
        return;
    }

	int64_t curr_val = -1;
	try {
		curr_val = std::stoll(value);
	}
	catch (std::exception& exc) {
		if (!_no_reply) {
			out = "CLIENT_ERROR Cannot convert cached value to int64_t: ";
			out += exc.what();
		}
		else { out.clear(); }
		return;
	}

	int64_t result = (_type ? (curr_val + _value) : (curr_val - _value)); //true - incr, false - decr
	if (result < 0) { result = 0; }

	if (storage.Put(_key, std::to_string(result))) { out = std::to_string(result); }
	else { out = "SERVER_ERROR Cannot store new value: Put() function returned false. Probably no enough memory"; }

	if (_no_reply) { out.clear(); }
}

} // namespace Execute
} // namespace Afina
