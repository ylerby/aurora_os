/*
 * SPDX-FileCopyrightText: Copyright 2023 Open Mobile Platform LLC <community@omp.ru>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef FLUTTER_PLUGIN_CAMERA_AURORA_PLUGIN_H
#define FLUTTER_PLUGIN_CAMERA_AURORA_PLUGIN_H

#include <camera_aurora/globals.h>
#include <camera_aurora/texture_camera.h>
#include <camera_aurora/encodable_helper.h>

#include <flutter/flutter_aurora.h>
#include <flutter/plugin_registrar.h>
#include <flutter/method_channel.h>
#include <flutter/event_channel.h>
#include <flutter/encodable_value.h>
#include <flutter/standard_message_codec.h>
#include <flutter/standard_method_codec.h>
#include <flutter/event_stream_handler_functions.h>

#include <QImage>
#include <QtCore>

typedef flutter::Plugin Plugin;
typedef flutter::PluginRegistrar PluginRegistrar;
typedef flutter::MethodChannel<EncodableValue> MethodChannel;
typedef flutter::MethodCall<EncodableValue> MethodCall;
typedef flutter::MethodResult<EncodableValue> MethodResult;
typedef flutter::EventChannel<EncodableValue> EventChannel;
typedef flutter::EventSink<EncodableValue> EventSink;

class PLUGIN_EXPORT CameraAuroraPlugin final
    : public QObject
    , public flutter::Plugin
{
    Q_OBJECT

public:
    static void RegisterWithRegistrar(PluginRegistrar* registrar);

private:
    // Creates a plugin that communicates on the given channel.
    CameraAuroraPlugin(
        PluginRegistrar* registrar,
        std::unique_ptr<MethodChannel> methodChannel,
        std::unique_ptr<EventChannel> eventChannelChange,
        std::unique_ptr<EventChannel> eventChannelQr
    );

    // Methods register handlers channels
    void RegisterMethodHandler();
    void RegisterStreamHandler();

    // Methods MethodCall
    EncodableValue onResizeFrame(const MethodCall &call);
    EncodableValue onAvailableCameras(const MethodCall &call);
    EncodableValue onCreateCamera(const MethodCall &call);
    EncodableValue onStartCapture(const MethodCall &call);
    EncodableValue onStopCapture(const MethodCall &call);
    EncodableValue onDispose(const MethodCall &call);

    std::unique_ptr<TextureCamera> m_textureCamera;
    
    std::unique_ptr<MethodChannel> m_methodChannel;
    std::unique_ptr<EventChannel> m_eventChannelChange;
    std::unique_ptr<EventChannel> m_eventChannelQr;

    std::unique_ptr<EventSink> m_sinkChange;
    std::unique_ptr<EventSink> m_sinkQr;

    bool m_stateEventChannelChange = false;
    bool m_stateEventChannelQr = false;
};

#endif /* FLUTTER_PLUGIN_CAMERA_AURORA_PLUGIN_H */
