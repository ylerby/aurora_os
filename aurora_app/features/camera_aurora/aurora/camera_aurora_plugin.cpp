/*
 * SPDX-FileCopyrightText: Copyright 2023 Open Mobile Platform LLC <community@omp.ru>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <camera_aurora/camera_aurora_plugin.h>

#include <QBuffer>
#include <QCamera>
#include <QCameraImageCapture>
#include <QCameraInfo>
#include <QMediaRecorder>
#include <QtCore>

namespace Channels {
    constexpr auto Methods = "camera_aurora";
    constexpr auto StateChanged = "cameraAuroraStateChanged";
    constexpr auto QrChanged = "cameraAuroraQrChanged";
} // namespace Channels

namespace Methods {
    constexpr auto AvailableCameras = "availableCameras";
    constexpr auto CreateCamera = "createCamera";
    constexpr auto ResizeFrame = "resizeFrame";
    constexpr auto Dispose = "dispose";
    constexpr auto StartCapture = "startCapture";
    constexpr auto StopCapture = "stopCapture";
    constexpr auto TakePicture = "takePicture";
} // namespace Methods

void CameraAuroraPlugin::RegisterWithRegistrar(PluginRegistrar* registrar)
{
    // Create MethodChannel with StandardMethodCodec
    auto methodChannel = std::make_unique<MethodChannel>(
        registrar->messenger(), Channels::Methods,
        &flutter::StandardMethodCodec::GetInstance());

    // Create EventChannel with StandardMethodCodec for Change
    auto eventChannelChange = std::make_unique<EventChannel>(
        registrar->messenger(), Channels::StateChanged,
        &flutter::StandardMethodCodec::GetInstance());

    // Create EventChannel with StandardMethodCodec for Qr
    auto eventChannelQr = std::make_unique<EventChannel>(
        registrar->messenger(), Channels::QrChanged,
        &flutter::StandardMethodCodec::GetInstance());

    // Create plugin
    std::unique_ptr<CameraAuroraPlugin> plugin(new CameraAuroraPlugin(
        registrar,
        std::move(methodChannel),
        std::move(eventChannelChange),
        std::move(eventChannelQr)
    ));

    // Register plugin
    registrar->AddPlugin(std::move(plugin));
}

CameraAuroraPlugin::CameraAuroraPlugin(
    PluginRegistrar* registrar,
    std::unique_ptr<MethodChannel> methodChannel,
    std::unique_ptr<EventChannel> eventChannelChange,
    std::unique_ptr<EventChannel> eventChannelQr
) : m_methodChannel(std::move(methodChannel)),
    m_eventChannelChange(std::move(eventChannelChange)),
    m_eventChannelQr(std::move(eventChannelQr))
{
    // Create camera streamcamera
    m_textureCamera = std::make_unique<TextureCamera>(registrar->texture_registrar(),
        [&]() {
            if (m_stateEventChannelChange) {
                m_sinkChange->Success(m_textureCamera->GetState());
            }
        },
        [&](std::string data) {
            if (m_stateEventChannelQr) {
                m_sinkQr->Success(data);
            }
        });

    // Listen change orientation
    aurora::SubscribeOrientationChanged([&](aurora::DisplayOrientation) {
        if (m_stateEventChannelChange) {
            m_sinkChange->Success(m_textureCamera->GetState());
        }
    });

    // Create MethodHandler
    RegisterMethodHandler();

    // Create StreamHandler
    RegisterStreamHandler();
}

void CameraAuroraPlugin::RegisterMethodHandler()
{
    m_methodChannel->SetMethodCallHandler(
        [this](const MethodCall& call, std::unique_ptr<MethodResult> result) {
            if (call.method_name().compare(Methods::ResizeFrame) == 0) {
                result->Success(onResizeFrame(call));
            }
            else if (call.method_name().compare(Methods::AvailableCameras) == 0) {
                result->Success(onAvailableCameras(call));
            }
            else if (call.method_name().compare(Methods::CreateCamera) == 0) {
                result->Success(onCreateCamera(call));
            }
            else if (call.method_name().compare(Methods::StartCapture) == 0) {
                result->Success(onStartCapture(call));
            }
            else if (call.method_name().compare(Methods::StopCapture) == 0) {
                result->Success(onStopCapture(call));
            }
            else if (call.method_name().compare(Methods::Dispose) == 0) {
                result->Success(onDispose(call));
            }
            else if (call.method_name().compare(Methods::TakePicture) == 0) {
                m_textureCamera->GetImageBase64([result = std::shared_ptr<MethodResult>(std::move(result))](std::string base64) {
                    result->Success(base64);
                });
            }
            else {
                result->Success();
            }
        });
}

void CameraAuroraPlugin::RegisterStreamHandler()
{
    // Set stream handler Change
    auto handlerChange = std::make_unique<flutter::StreamHandlerFunctions<EncodableValue>>(
        [&](const EncodableValue*,
            std::unique_ptr<flutter::EventSink<EncodableValue>>&& events
        ) -> std::unique_ptr<flutter::StreamHandlerError<EncodableValue>> {
            m_sinkChange = std::move(events);
            m_stateEventChannelChange = true;
            m_sinkChange->Success(m_textureCamera->GetState());
            return nullptr;
        },
        [&](const EncodableValue*) -> std::unique_ptr<flutter::StreamHandlerError<EncodableValue>> {
            m_stateEventChannelChange = false;
            return nullptr;
        }
    );

    m_eventChannelChange->SetStreamHandler(std::move(handlerChange));

    // Set stream handler Qr
    auto handlerQr = std::make_unique<flutter::StreamHandlerFunctions<EncodableValue>>(
        [&](const EncodableValue*,
            std::unique_ptr<flutter::EventSink<EncodableValue>>&& events
        ) -> std::unique_ptr<flutter::StreamHandlerError<EncodableValue>> {
            m_sinkQr = std::move(events);
            m_stateEventChannelQr = true;
            m_textureCamera->EnableSearchQr(true);
            return nullptr;
        },
        [&](const EncodableValue*) -> std::unique_ptr<flutter::StreamHandlerError<EncodableValue>> {
            m_stateEventChannelQr = false;
            m_textureCamera->EnableSearchQr(false);
            return nullptr;
        }
    );

    m_eventChannelQr->SetStreamHandler(std::move(handlerQr));
}

EncodableValue CameraAuroraPlugin::onResizeFrame(const MethodCall& method_call)
{
    if (Helper::TypeIs<EncodableMap>(*method_call.arguments())) {
        const EncodableMap params = Helper::GetValue<EncodableMap>(*method_call.arguments());
        auto width = Helper::GetInt(params, "width");
        auto height = Helper::GetInt(params, "height");
        auto state = m_textureCamera->ResizeFrame(width, height);
        if (m_stateEventChannelChange) {
            m_sinkChange->Success(state);
        }
    }
    return EncodableValue();
}

EncodableValue CameraAuroraPlugin::onAvailableCameras(const MethodCall&)
{
    return m_textureCamera->GetAvailableCameras();
}

EncodableValue CameraAuroraPlugin::onCreateCamera(const MethodCall& method_call)
{
    if (Helper::TypeIs<EncodableMap>(*method_call.arguments())) {
        const EncodableMap params = Helper::GetValue<EncodableMap>(*method_call.arguments());
        auto cameraName = Helper::GetString(params, "cameraName");
        auto state = m_textureCamera->Register(cameraName);
        if (m_stateEventChannelChange) {
            m_sinkChange->Success(state);
        }
        return state;
    }
    return EncodableValue();
}

EncodableValue CameraAuroraPlugin::onStartCapture(const MethodCall& method_call)
{
    if (Helper::TypeIs<EncodableMap>(*method_call.arguments())) {
        const EncodableMap params = Helper::GetValue<EncodableMap>(*method_call.arguments());
        auto width = Helper::GetInt(params, "width");
        auto height = Helper::GetInt(params, "height");
        auto state = m_textureCamera->StartCapture(width, height);
        if (m_stateEventChannelChange) {
            m_sinkChange->Success(state);
        }
    }
    return EncodableValue();
}

EncodableValue CameraAuroraPlugin::onStopCapture(const MethodCall&)
{
    m_textureCamera->StopCapture();
    return EncodableValue();
}

EncodableValue CameraAuroraPlugin::onDispose(const MethodCall&)
{
    auto state = m_textureCamera->Unregister();
    if (m_stateEventChannelChange) {
        m_sinkChange->Success(state);
    }
    return EncodableValue();
}

#include "moc_camera_aurora_plugin.cpp"
