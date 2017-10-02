#include <afina/execute/Get.h>
#include <afina/Storage.h>

#include <sstream>
#include <iterator>
#include <iostream>

namespace Afina {
namespace Execute {

void Get::Execute(Storage &storage, const std::string &args, std::string &out) {
    std::stringstream keyStream;
    copy(_keys.begin(),_keys.end(), std::ostream_iterator<std::string>(keyStream," "));
    std::cout << "Get(" << keyStream.str() << ")" << std::endl;

    std::stringstream outStream;

    std::string value;
    for (auto& key:_keys) {
        storage.Get(key,value);
        outStream << value << " ";
    }

    out = outStream.str();
}

} // namespace Execute
} // namespace Afina
