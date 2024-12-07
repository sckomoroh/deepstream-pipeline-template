#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "IBinFactory.h"

namespace yz {

std::unordered_map<std::string, std::shared_ptr<IBinFactory>> getFactories();

}
