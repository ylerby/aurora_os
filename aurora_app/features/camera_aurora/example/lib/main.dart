// SPDX-FileCopyrightText: Copyright 2024 Open Mobile Platform LLC <community@omp.ru>
// SPDX-License-Identifier: BSD-3-Clause
import 'package:flutter/services.dart';
import 'package:camera/camera.dart';
import 'package:flutter/material.dart';
import 'package:internal_aurora/abb_bar_action.dart';
import 'package:internal_aurora/list_item_data.dart';
import 'package:internal_aurora/list_item_info.dart';
import 'package:internal_aurora/list_separated.dart';
import 'package:internal_aurora/theme/colors.dart';
import 'package:internal_aurora/theme/theme.dart';

import 'plugin_impl.dart';

void main() {
  runApp(const MyApp());
  SystemChrome.setPreferredOrientations([
    DeviceOrientation.portraitUp,
  ]);
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final PluginImpl _impl = PluginImpl();

  bool _isLoading = false;
  bool _isSearch = false;
  bool _isTake = false;

  /// Show info
  List<String>? _message;

  /// Enable QR on camera
  Future<void> _activateQR() async {
    if (!mounted) return;
    // enable search view
    setState(() => _isSearch = true);
    // Search qr
    final search = await _impl.loadQR();
    // Show result
    _showMessage('Data found using QR code.', search.toString());
    // disable search view
    setState(() => _isSearch = false);
  }

  /// Take photo and save
  Future<void> _takePhoto() async {
    if (!mounted) return;
    // enable search view
    setState(() => _isTake = true);
    // Search qr
    final path = await _impl.takePhoto();
    // Show result
    _showMessage('The photo was successfully saved.', path.toString());
    // disable search view
    setState(() => _isTake = false);
  }

  /// Show message
  void _showMessage(String description, String value) {
    if (!mounted) return;
    setState(() {
      _message = [description, value];
    });
    Future.delayed(
      const Duration(seconds: 3),
      () => setState(() => _message = null),
    );
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      theme: internalTheme,
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Camera'),
          actions: !_impl.isInit()
              ? null
              : [
                  AppBarAction(
                    icon: Icons.qr_code_scanner,
                    onPressed: _isLoading || _isSearch || _message != null
                        ? null
                        : _activateQR,
                  ),
                  AppBarAction(
                    icon: Icons.photo_camera,
                    onPressed: _isLoading || _isTake || _message != null
                        ? null
                        : _takePhoto,
                  ),
                ],
        ),
        body: ListSeparated(
          children: [
            const ListItemInfo("""
            A Flutter plugin for iOS, Android, Web and Aurora OS
            allowing access to the cameras.
            """),

            /// Show list available cameras
            ListItemData(
              'Cameras',
              InternalColors.purple,
              future: _impl.getListCameras(),
              builder: (value) {
                if (value == null) {
                  return const Padding(
                    padding: EdgeInsets.all(16),
                    child: Icon(
                      Icons.error,
                      color: Colors.red,
                    ),
                  );
                }
                return Card(
                  clipBehavior: Clip.antiAlias,
                  child: Column(
                    children: value
                        .map((e) => RadioListTile(
                              title: Text(
                                '${e.name.contains('rear') ? 'Back' : 'Front'} (${e.name})',
                              ),
                              value: e,
                              groupValue: _impl.getActiveCamera(),
                              onChanged: _isLoading
                                  ? null
                                  : (CameraDescription? camera) async {
                                      if (camera != null) {
                                        setState(() {
                                          _message = null;
                                          _isLoading = true;
                                        });
                                        if (await _impl.changeCamera(e)) {
                                          setState(() {
                                            _isLoading = false;
                                          });
                                        }
                                      }
                                    },
                            ))
                        .toList(),
                  ),
                );
              },
            ),

            /// Show viewfinder
            if (!_isLoading && _message == null)
              ListItemData(
                'Viewfinder',
                InternalColors.grey,
                value: _impl.getCameraController(),
                builder: (controller) {
                  if (controller == null) {
                    return 'Select camera...';
                  }
                  return SizedBox(
                    height: 300,
                    child: Stack(
                      children: [
                        Center(
                          child: Icon(
                            Icons.sync,
                            color: Colors.white,
                          ),
                        ),
                        Center(
                          child: controller.buildPreview(),
                        ),
                      ],
                    ),
                  );
                },
              ),

            /// Show info about success result
            if (_message != null)
              ListItemData(
                'Viewfinder',
                InternalColors.grey,
                description: _message![0],
                value: _message![1],
                replace: false,
                builder: (message) {
                  if (message == null) {
                    return const SizedBox.shrink();
                  }
                  return SizedBox(
                    width: double.infinity,
                    child: Text(
                      message,
                      style: TextStyle(color: Colors.white),
                    ),
                  );
                },
              ),

            /// Show loading viewfinder
            if (_isLoading)
              ListItemData(
                'Viewfinder',
                InternalColors.grey,
                value: 'Loading camera...',
              ),
          ],
        ),
      ),
    );
  }
}
