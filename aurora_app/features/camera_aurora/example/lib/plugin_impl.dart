// SPDX-FileCopyrightText: Copyright 2024 Open Mobile Platform LLC <community@omp.ru>
// SPDX-License-Identifier: BSD-3-Clause

import 'dart:async';
import 'dart:io';
import 'dart:typed_data';

import 'package:camera/camera.dart';
import 'package:camera_aurora/camera_aurora.dart';
import 'package:path/path.dart' as p;
import 'package:path_provider/path_provider.dart';

/// Main features of the plugin camera
class PluginImpl {
  CameraController? _controller;

  /// Get list available cameras
  Future<List<CameraDescription>?> getListCameras() {
    try {
      return availableCameras();
    } on CameraException {
      return Future.value(null);
    }
  }

  /// Check is init controller
  bool isInit() {
    if (_controller == null) {
      return false;
    }
    if (!_controller!.value.isInitialized) {
      return false;
    }
    return true;
  }

  /// Get active camera
  CameraDescription? getActiveCamera() {
    return _controller?.description;
  }

  /// Get controller
  CameraController? getCameraController() {
    return _controller;
  }

  /// Init camera controller
  Future<bool> changeCamera(CameraDescription camera) async {
    try {
      if (_controller != null) {
        // Check and stop if need image stream
        if (_controller!.value.isStreamingImages) {
          await _controller!.stopImageStream();
        }
        // Change camera
        await _controller!.setDescription(camera);
      } else {
        _controller = CameraController(
          camera,
          ResolutionPreset.medium,
          enableAudio: false,
          imageFormatGroup: ImageFormatGroup.jpeg,
        );
        await _controller!.initialize();
        await Future<void>.delayed(const Duration(milliseconds: 500));
      }
      return true;
    } on CameraException {
      return false;
    }
  }

  /// Enable search QR and get first data
  Future<String?> loadQR() async {
    return cameraSearchQr?.firstWhere((String item) => item.isNotEmpty);
  }

  /// Take and save photo
  Future<String?> takePhoto() async {
    if (_controller == null) {
      return null;
    }
    try {
      // Get image
      final picture = await _controller!.takePicture();
      // Get bytes
      final bytes = await picture.readAsBytes();
      // Get path
      final directory = await getExternalStorageDirectories(
        type: StorageDirectory.pictures,
      );
      // Save to file
      final file = await bytes.writeToFile(directory![0], picture);
      // Return saved file
      return file.path;
    } on CameraException {
      return null;
    }
  }
}

/// Save file form Uint8List
extension ExtUint8List on Uint8List {
  Future<File> writeToFile(Directory directory, XFile file) {
    if (file.name.isEmpty) {
      return File(p.join(
        directory.path,
        '${DateTime.now().millisecondsSinceEpoch}.jpg',
      )).writeAsBytes(this);
    } else {
      return File(p.join(directory.path, file.name)).writeAsBytes(this);
    }
  }
}
