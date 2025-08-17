#include "ble.hh"

namespace luz::ble::detail
{
void ServerCallbacks::onConnect(NimBLEServer* server_, NimBLEConnInfo& conn_info)
{
  ESP_LOGI(detail::tag, "Client address: %s\n", conn_info.getAddress().toString().c_str());
  // restart_advertising=true;
  NimBLEDevice::startAdvertising();
}

void ServerCallbacks::onDisconnect(NimBLEServer* server_, NimBLEConnInfo& conn_info, int reason)
{
  ESP_LOGI(detail::tag, "Client disconnected - start advertising");
  // restart_advertising=true;
  NimBLEDevice::startAdvertising();
}

void DescriptorCallbacks::onRead(NimBLEDescriptor* descriptor, NimBLEConnInfo& conn_info)
{
  ESP_LOGI(detail::tag, "%s Descriptor read\n", descriptor->getUUID().toString().c_str());
}
} // namespace luz::ble::detail
