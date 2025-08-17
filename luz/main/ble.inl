#pragma once

#include "ble.hh"

#include <array>
#include <format>
#include <span>
#include <string_view>

// TODO constexpr?
#define ADVERTISING_SERVICE_UUID "4488B571-7806-4DF6-BCFF-A2897E4953FF"
#define DATA_TRANSFER_SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define DATA_TRANSFER_CHARACTERISTIC "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define DESCRIPTOR_UUID "00002902-0000-1000-8000-00805f9b34fb"

namespace luz::ble
{
namespace detail
{
constexpr uint8_t api_level = 3U;

template <std::regular_invocable<std::span<const std::byte>> OnWriteCallback>
template <std::regular_invocable<std::span<const std::byte>> _OnWriteCallback>
CharacteristicCallbacks<OnWriteCallback>::CharacteristicCallbacks(
    _OnWriteCallback& on_write_callback) noexcept
    : NimBLECharacteristicCallbacks(),
      on_write_callback_{ std::pointer_traits<OnWriteCallback*>::pointer_to(on_write_callback) }
{
}

template <std::regular_invocable<std::span<const std::byte>> OnWriteCallback>
void CharacteristicCallbacks<OnWriteCallback>::onWrite(NimBLECharacteristic* characteristic,
                                                       NimBLEConnInfo& conn_info)
{
  std::array<char, 256> print_buf{ '\0' };
  ESP_LOGI(detail::tag, "%s : onWrite(), value:", characteristic->getUUID().toString().c_str());

  // TODO get value has a templated overload that casts to target type -- maybe use to create
  // std::byte span?
  auto value = characteristic->getValue();
  constexpr std::string_view fmt_string = "0x%.2X, ";
  std::span<char> print_buf_view(print_buf);
  for (const uint8_t datum : value)
  {
    snprintf(print_buf_view.data(), print_buf_view.size(), fmt_string.data(), datum);
    print_buf_view = print_buf_view.subspan(6U);
  }
  ESP_LOGI(detail::tag, "\t%s", print_buf.data());

  // TODO std::as_bytes(span)
  std::invoke(*on_write_callback_, std::as_bytes(std::span{ value.data(), value.size() }));
}
} // namespace detail

template <std::regular_invocable<std::span<const std::byte>> OnWriteCallback>
template <std::regular_invocable<std::span<const std::byte>> _OnWriteCallback>
DecoyPeripheral<OnWriteCallback>::DecoyPeripheral(std::string_view name,
                                                  _OnWriteCallback& on_write_callback) noexcept
    : characteristic_callbacks_{ on_write_callback }
{
  const auto board_name = std::format("{}@{}", name, detail::api_level);
  NimBLEDevice::init(board_name);

  server_ = NimBLEDevice::createServer();
  server_->setCallbacks(&server_callbacks_);

  service_ = server_->createService(DATA_TRANSFER_SERVICE_UUID);

  characteristic_
      = service_->createCharacteristic(DATA_TRANSFER_CHARACTERISTIC, NIMBLE_PROPERTY::WRITE);
  characteristic_->setCallbacks(&characteristic_callbacks_);

  descriptor_ = characteristic_->createDescriptor(DESCRIPTOR_UUID, NIMBLE_PROPERTY::READ);
  descriptor_->setCallbacks(&descriptor_callbacks_);
  descriptor_->setValue("Hold placements");

  service_->start();

  advertising_ = NimBLEDevice::getAdvertising();
  advertising_->addServiceUUID(ADVERTISING_SERVICE_UUID);
  advertising_->enableScanResponse(true);
  advertising_->setName(board_name);
  advertising_->setConnectableMode(2);
  advertising_->setAdvertisingInterval(0x780);
  NimBLEDevice::startAdvertising();

  ESP_LOGI(detail::tag, "Advertising Decoy board with name: %s", board_name.c_str());
}
} // namespace luz::ble
