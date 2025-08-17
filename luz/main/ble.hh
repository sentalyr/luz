#pragma once

#include "NimBLEDescriptor.h"
#include "NimBLEDevice.h"
#include "NimBLEHIDDevice.h"
#include "NimBLELocalValueAttribute.h"

#include <concepts>
#include <memory>
#include <span>
#include <string_view>

namespace luz::ble
{
template <std::regular_invocable<std::span<const std::byte>> OnWriteCallback>
class DecoyPeripheral;

namespace detail
{
constexpr auto tag = "BLE";

class ServerCallbacks : public NimBLEServerCallbacks
{
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override;
  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override;
};

template <std::regular_invocable<std::span<const std::byte>> OnWriteCallback>
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
public:
  template <std::regular_invocable<std::span<const std::byte>> _OnWriteCallback>
  CharacteristicCallbacks(_OnWriteCallback& on_write_callback) noexcept;

  ~CharacteristicCallbacks() noexcept = default;

  /// Copy/move constructor/assignment
  CharacteristicCallbacks(const CharacteristicCallbacks&) = delete;
  CharacteristicCallbacks& operator=(const CharacteristicCallbacks&) = delete;
  CharacteristicCallbacks(CharacteristicCallbacks&&) = delete;
  CharacteristicCallbacks& operator=(CharacteristicCallbacks&&) = delete;

private:
  void onWrite(NimBLECharacteristic* characteristic, NimBLEConnInfo& conn_info) override;
  OnWriteCallback* on_write_callback_;
};

class DescriptorCallbacks : public NimBLEDescriptorCallbacks
{
  void onRead(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) override;
};
} // namespace detail

template <std::regular_invocable<std::span<const std::byte>> OnWriteCallback> class DecoyPeripheral
{
public:
  template <std::regular_invocable<std::span<const std::byte>> _OnWriteCallback>
  DecoyPeripheral(std::string_view name, _OnWriteCallback& on_write_callback) noexcept;

  ~DecoyPeripheral() noexcept = default;

  /// Copy/move constructor/assignment
  DecoyPeripheral(const DecoyPeripheral&) = delete;
  DecoyPeripheral& operator=(const DecoyPeripheral&) = delete;
  DecoyPeripheral(DecoyPeripheral&&) = delete;
  DecoyPeripheral& operator=(DecoyPeripheral&&) = delete;

private:
  NimBLEServer* server_{};
  NimBLEService* service_{};
  NimBLECharacteristic* characteristic_{};
  NimBLEDescriptor* descriptor_{};
  NimBLEAdvertising* advertising_{};

  detail::ServerCallbacks server_callbacks_{};
  detail::CharacteristicCallbacks<OnWriteCallback> characteristic_callbacks_;
  detail::DescriptorCallbacks descriptor_callbacks_{};

  // TODO unused at the moment
  bool restart_advertising_ = false;
};

/// Deduation guide
template <std::regular_invocable<std::span<const std::byte>> OnWriteCallback>
DecoyPeripheral(std::string_view, OnWriteCallback) -> DecoyPeripheral<OnWriteCallback>;
} // namespace luz::ble

#include "ble.inl"
