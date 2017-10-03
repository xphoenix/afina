#include "MapBasedGlobalLockImpl.h"
using namespace std;
namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Put(const std::string &key, const std::string &value) {
	if(data.find(key)!=data.end()){
		Delete(key);
		return PutIfAbsent(key,value);
	}
	else
		return PutIfAbsent(key,value);
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::PutIfAbsent(const std::string &key, const std::string &value){
	return data.insert( pair<string,string>(key,value)).second;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Set(const std::string &key, const std::string &value) {
	map<string,string>::iterator pos=data.find(key);
	if (pos==data.end())
		return false;
	else
		pos->second=value;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Delete(const std::string &key){
	if(data.erase(key)>0)
		return true;
	else
		return false;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Get(const std::string &key, std::string &value) const {
	if (data.find(key)==data.end())
		return false;
	else{
		value=(data.find(key)->second);
		return true;
	}
}

} // namespace Backend
} // namespace Afina
