#include "network.h"
#include "data_type.h"

Network::Network(const ModelBase* parent, const std::string& name)
  : ModuleBase(parent, name)
{}

void
Network::finalize()
{
  PacketManager::destory();
  FlitManager::destroy();
}

