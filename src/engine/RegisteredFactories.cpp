#include "RegisteredFactories.h"

#include <memory>
#include <string>
#include <unordered_map>

#include "InferBinFactory.h"
#include "SourceBinFactory.h"
#include "SinkBinFactory.h"
#include "RenderBinFactory.h"
#include "BrokerBinFactory.h"

namespace yz {

std::unordered_map<std::string, std::shared_ptr<IBinFactory>> getFactories() {
    return {{"yz-infer", std::make_shared<InferBinFactory>()},
            {"yz-source", std::make_shared<SourceBinFactory>()},
            {"yz-render", std::make_shared<RenderBinFactory>()},
            {"yz-broker", std::make_shared<BrokerBinFactory>()},
            {"yz-sink", std::make_shared<SinkBinFactory>()}};
}

}  // namespace yz
