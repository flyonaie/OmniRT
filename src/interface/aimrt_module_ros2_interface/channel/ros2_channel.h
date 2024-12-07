// Copyright (c) 2023, AgiBot Inc.
// All rights reserved.

#pragma once

#include <concepts>
#include <memory>

#include "aimrt_module_cpp_interface/channel/channel_handle.h"
#include "aimrt_module_ros2_interface/util/ros2_type_support.h"

#include "aimrt_module_cpp_interface/co/inline_scheduler.h"
#include "aimrt_module_cpp_interface/co/on.h"
#include "aimrt_module_cpp_interface/co/start_detached.h"
#include "aimrt_module_cpp_interface/co/task.h"
#include "aimrt_module_cpp_interface/co/then.h"

namespace aimrt::channel {

template <Ros2MsgType MsgType>
inline bool RegisterPublishType(PublisherRef publisher) {
  return publisher.RegisterPublishType(GetRos2MessageTypeSupport<MsgType>());
}

template <Ros2MsgType MsgType>
inline void Publish(PublisherRef publisher, ContextRef ctx_ref, const MsgType& msg) {
  static const std::string kMsgTypeName =
      std::string("ros2:") + rosidl_generator_traits::name<MsgType>();

  if (ctx_ref) {
    if (ctx_ref.GetSerializationType().empty()) ctx_ref.SetSerializationType("ros2");
    publisher.Publish(kMsgTypeName, ctx_ref, static_cast<const void*>(&msg));
    return;
  }

  Context ctx;
  ctx.SetSerializationType("ros2");
  publisher.Publish(kMsgTypeName, ctx, static_cast<const void*>(&msg));
}

template <Ros2MsgType MsgType>
inline void Publish(PublisherRef publisher, const MsgType& msg) {
  Publish(publisher, ContextRef(), msg);
}

template <Ros2MsgType MsgType>
inline bool Subscribe(
    SubscriberRef subscriber,
    std::function<void(ContextRef, const std::shared_ptr<const MsgType>&)>&& callback) {
  return subscriber.Subscribe(
      GetRos2MessageTypeSupport<MsgType>(),
      [callback{std::move(callback)}](
          const aimrt_channel_context_base_t* ctx_ptr,
          const void* msg_ptr,
          aimrt_function_base_t* release_callback_base) {
        SubscriberReleaseCallback release_callback(release_callback_base);
        callback(ContextRef(ctx_ptr),
                 std::shared_ptr<const MsgType>(
                     static_cast<const MsgType*>(msg_ptr),
                     [release_callback{std::move(release_callback)}](const MsgType*) { release_callback(); }));
      });
}

template <Ros2MsgType MsgType>
inline bool Subscribe(
    SubscriberRef subscriber,
    std::function<void(const std::shared_ptr<const MsgType>&)>&& callback) {
  return subscriber.Subscribe(
      GetRos2MessageTypeSupport<MsgType>(),
      [callback{std::move(callback)}](
          const aimrt_channel_context_base_t* ctx_ptr,
          const void* msg_ptr,
          aimrt_function_base_t* release_callback_base) {
        SubscriberReleaseCallback release_callback(release_callback_base);
        callback(std::shared_ptr<const MsgType>(
            static_cast<const MsgType*>(msg_ptr),
            [release_callback{std::move(release_callback)}](const MsgType*) { release_callback(); }));
      });
}

template <Ros2MsgType MsgType>
inline bool SubscribeCo(
    SubscriberRef subscriber,
    std::function<co::Task<void>(ContextRef, const MsgType&)>&& callback) {
  return subscriber.Subscribe(
      GetRos2MessageTypeSupport<MsgType>(),
      [callback{std::move(callback)}](
          const aimrt_channel_context_base_t* ctx_ptr,
          const void* msg_ptr,
          aimrt_function_base_t* release_callback_base) {
        aimrt::co::StartDetached(
            aimrt::co::On(
                aimrt::co::InlineScheduler(),
                callback(ContextRef(ctx_ptr), *(static_cast<const MsgType*>(msg_ptr)))) |
            aimrt::co::Then(
                SubscriberReleaseCallback(release_callback_base)));
      });
}

template <Ros2MsgType MsgType>
inline bool SubscribeCo(
    SubscriberRef subscriber,
    std::function<co::Task<void>(const MsgType&)>&& callback) {
  return subscriber.Subscribe(
      GetRos2MessageTypeSupport<MsgType>(),
      [callback{std::move(callback)}](
          const aimrt_channel_context_base_t* ctx_ptr,
          const void* msg_ptr,
          aimrt_function_base_t* release_callback_base) {
        aimrt::co::StartDetached(
            aimrt::co::On(
                aimrt::co::InlineScheduler(),
                callback(*(static_cast<const MsgType*>(msg_ptr)))) |
            aimrt::co::Then(
                SubscriberReleaseCallback(release_callback_base)));
      });
}

template <Ros2MsgType MsgType>
class PublisherProxy<MsgType> : public PublisherProxyBase {
 public:
  explicit PublisherProxy(PublisherRef publisher)
      : PublisherProxyBase(publisher, std::string("ros2:") + rosidl_generator_traits::name<MsgType>()) {}
  ~PublisherProxy() = default;

  static bool RegisterPublishType(PublisherRef publisher) {
    return publisher.RegisterPublishType(GetRos2MessageTypeSupport<MsgType>());
  }

  bool RegisterPublishType() {
    return publisher_.RegisterPublishType(GetRos2MessageTypeSupport<MsgType>());
  }

  void Publish(ContextRef ctx_ref, const MsgType& msg) {
    if (ctx_ref) {
      if (ctx_ref.GetSerializationType().empty()) ctx_ref.SetSerializationType("ros2");
      PublishImpl(ctx_ref, static_cast<const void*>(&msg));
      return;
    }

    auto ctx_ptr = NewContextSharedPtr();
    ctx_ptr->SetSerializationType("ros2");
    PublishImpl(ctx_ptr, static_cast<const void*>(&msg));
  }

  void Publish(const MsgType& msg) {
    Publish(ContextRef(), msg);
  }
};

template <Ros2MsgType MsgType>
class SubscriberProxy<MsgType> : public SubscriberProxyBase {
 public:
  explicit SubscriberProxy(SubscriberRef subscriber)
      : SubscriberProxyBase(subscriber, std::string("ros2:") + rosidl_generator_traits::name<MsgType>()) {}
  ~SubscriberProxy() = default;

  bool Subscribe(
      std::function<void(ContextRef, const std::shared_ptr<const MsgType>&)>&& callback) {
    return subscriber_.Subscribe(
        GetRos2MessageTypeSupport<MsgType>(),
        [callback{std::move(callback)}](
            const aimrt_channel_context_base_t* ctx_ptr,
            const void* msg_ptr,
            aimrt_function_base_t* release_callback_base) {
          ContextRef ctx_ref(ctx_ptr);

          SubscriberReleaseCallback release_callback(release_callback_base);
          callback(ctx_ref,
                   std::shared_ptr<const MsgType>(
                       static_cast<const MsgType*>(msg_ptr),
                       [release_callback{std::move(release_callback)}](const MsgType*) { release_callback(); }));
        });
  }

  bool Subscribe(
      std::function<void(const std::shared_ptr<const MsgType>&)>&& callback) {
    return subscriber_.Subscribe(
        GetRos2MessageTypeSupport<MsgType>(),
        [callback{std::move(callback)}](
            const aimrt_channel_context_base_t* ctx_ptr,
            const void* msg_ptr,
            aimrt_function_base_t* release_callback_base) {
          ContextRef ctx_ref(ctx_ptr);

          SubscriberReleaseCallback release_callback(release_callback_base);
          callback(std::shared_ptr<const MsgType>(
              static_cast<const MsgType*>(msg_ptr),
              [release_callback{std::move(release_callback)}](const MsgType*) { release_callback(); }));
        });
  }

  bool SubscribeCo(
      std::function<co::Task<void>(ContextRef, const MsgType&)>&& callback) {
    return subscriber_.Subscribe(
        GetRos2MessageTypeSupport<MsgType>(),
        [callback{std::move(callback)}](
            const aimrt_channel_context_base_t* ctx_ptr,
            const void* msg_ptr,
            aimrt_function_base_t* release_callback_base) {
          ContextRef ctx_ref(ctx_ptr);

          aimrt::co::StartDetached(
              aimrt::co::On(
                  aimrt::co::InlineScheduler(),
                  callback(ctx_ref, *(static_cast<const MsgType*>(msg_ptr)))) |
              aimrt::co::Then(
                  SubscriberReleaseCallback(release_callback_base)));
        });
  }

  bool SubscribeCo(std::function<co::Task<void>(const MsgType&)>&& callback) {
    return subscriber_.Subscribe(
        GetRos2MessageTypeSupport<MsgType>(),
        [callback{std::move(callback)}](
            const aimrt_channel_context_base_t* ctx_ptr,
            const void* msg_ptr,
            aimrt_function_base_t* release_callback_base) {
          ContextRef ctx_ref(ctx_ptr);

          aimrt::co::StartDetached(
              aimrt::co::On(
                  aimrt::co::InlineScheduler(),
                  callback(*(static_cast<const MsgType*>(msg_ptr)))) |
              aimrt::co::Then(
                  SubscriberReleaseCallback(release_callback_base)));
        });
  }
};

}  // namespace aimrt::channel