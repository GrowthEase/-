// Copyright (c) 2022 NetEase, Inc. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

import 'dart:async';

import '../util/sp_util.dart';

class GlobalPreferences extends Preferences {
  static const String _keyLoginInfo = "loginInfo";
  static const String _keyMeetingDebug = "meetingDebug";
  static const String _keyMeetingEnv = "meetingEnv";
  static const String _keyNERtcLogLevel = "nertcLogLevel";
  static const String _keyUserProtocolPrivacy = "userProtocolPrivacy";
  static const String _keySecurityNotice = "securityNotice";
  static const String _keyMeetingInfo = "meetingInfo";
  static const String _keyPrivacyDialogShowed = 'privacyDialogShowed';
  static const String _keyMeetingEvaluation = 'meetingEvaluation';
  static const String _keyCorpCode = 'corpCode';

  GlobalPreferences._internal();

  static GlobalPreferences _singleton = GlobalPreferences._internal();

  factory GlobalPreferences() => _singleton;

  /// save
  Future<void> setLoginInfo(String value) async {
    setSp(_keyLoginInfo, value);
  }

  Future<String?> get loginInfo async {
    return getSp(_keyLoginInfo);
  }

  Future<bool?> get meetingDebug async {
    return getBoolSp(_keyMeetingDebug);
  }

  Future<void> setMeetingDebug(bool meetingDebug) async {
    setBoolSp(_keyMeetingDebug, meetingDebug);
  }

  final _privacyAgreeCompleter = Completer();
  Future ensurePrivacyAgree() async {
    if (!_privacyAgreeCompleter.isCompleted) {
      hasPrivacyDialogShowed.then((value) {
        if (value && !_privacyAgreeCompleter.isCompleted) {
          _privacyAgreeCompleter.complete();
        }
      });
    }
    return _privacyAgreeCompleter.future;
  }

  Future<bool> get hasPrivacyDialogShowed async {
    return await getBoolSp(_keyPrivacyDialogShowed) ?? false;
  }

  Future<bool> setPrivacyDialogShowed(bool enabled) async {
    if (enabled && !_privacyAgreeCompleter.isCompleted) {
      _privacyAgreeCompleter.complete();
    }
    return setBoolSp(_keyPrivacyDialogShowed, enabled);
  }

  Future<String?> get meetingEnv async {
    return getSp(_keyMeetingEnv);
  }

  Future<bool> setMeetingEnv(String? env) async {
    if (env == null || env.isEmpty) {
      return remove(_keyMeetingEnv);
    } else {
      return setSp(_keyMeetingEnv, env);
    }
  }

  Future<String?> get nertcLogLevel async {
    return getSp(_keyNERtcLogLevel);
  }

  Future<void> setNertcLogLevel(String level) async {
    setSp(_keyNERtcLogLevel, level);
  }

  Future<bool?> get userProtocolAndPrivacy async {
    return getBoolSp(_keyUserProtocolPrivacy);
  }

  Future<void> setUserProtocolAndPrivacy(bool isShow) async {
    setBoolSp(_keyUserProtocolPrivacy, isShow);
  }

  Future<String?> get securityNotice async {
    return getSp(_keySecurityNotice);
  }

  Future<void> setSecurityNotice(String notice) async {
    setSp(_keySecurityNotice, notice);
  }

  Future<void> setMeetingInfo(String value) async {
    setSp(_keyMeetingInfo, value);
  }

  Future<String?> get meetingInfo async {
    return getSp(_keyMeetingInfo);
  }

  Future<void> setMeetingEvaluation(String value) async {
    setSp(_keyMeetingEvaluation, value);
  }

  Future<String?> get meetingEvaluation async {
    return getSp(_keyMeetingEvaluation);
  }

  Future<String?> get savedCorpCode async {
    return getSp(_keyCorpCode);
  }

  void saveCorpCode(String? value) {
    value != null ? setSp(_keyCorpCode, value) : remove(_keyCorpCode);
  }
}
