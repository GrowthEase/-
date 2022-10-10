// Copyright (c) 2022 NetEase, Inc. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io';

import 'package:dio/dio.dart';
import 'package:nemeeting/service/client/app_http_client.dart';
import 'package:nemeeting/service/client/http_code.dart';
import 'package:nemeeting/service/proto/base_proto.dart';
import 'package:nemeeting/service/response/result.dart';

class DownloadFileProto extends BaseProto<void> {
  final String url;

  final File file;

  final ProgressCallback progressCallback;

  DownloadFileProto(this.url, this.file, this.progressCallback);

  @override
  Future<Result<void>> execute() async {
    var response =
        await AppHttpClient().downloadFile(url, file.path, progressCallback);
    if (response?.statusCode != HttpStatus.ok) {
      return Result(code: HttpCode.netWorkError);
    } else {
      return Result(code: HttpCode.success);
    }
  }

  @override
  String path() {
    throw UnimplementedError();
  }

  @override
  double result(Map<dynamic, dynamic> map) {
    throw UnimplementedError();
  }

  @override
  Map data() {
    throw UnimplementedError();
  }

  @override
  bool checkLoginState() {
    return false;
  }
}
