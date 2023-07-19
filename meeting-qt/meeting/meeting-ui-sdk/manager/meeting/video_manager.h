﻿// Copyright (c) 2022 NetEase, Inc. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

#include <QObject>
#include <QVideoFrame>
#include "controller/video_controller.h"
#include "manager/auth_manager.h"
#include "manager/meeting_manager.h"

using namespace neroom;

typedef struct tagCanvas {
    QObject* view = nullptr;
    bool sub = false;
    uint64_t timestamp = 0;
} Canvas;

Q_DECLARE_METATYPE(NEMeeting::DeviceStatus)

class VideoFrameDelegate : public QObject {
    Q_OBJECT
public:
    VideoFrameDelegate(QObject* parent = nullptr);
    ~VideoFrameDelegate() = default;

signals:
    void receivedVideoFrame(const QString& accountId, const QVideoFrame& videoFrame, const QSize& videoSize, bool sub);
};

using DelegatePtr = std::unique_ptr<VideoFrameDelegate>;

class VideoManager : public QObject {
    Q_OBJECT
private:
    explicit VideoManager(QObject* parent = nullptr);

public:
    SINGLETONG(VideoManager)

    Q_PROPERTY(int localVideoStatus READ localVideoStatus WRITE setLocalVideoStatus NOTIFY localVideoStatusChanged)
    Q_PROPERTY(QString focusAccountId READ focusAccountId WRITE setFocusAccountId NOTIFY focusAccountIdChanged)
    Q_PROPERTY(bool displayVideoStats READ displayVideoStats WRITE setDisplayVideoStats NOTIFY displayVideoStatsChanged)

    Q_INVOKABLE void openSystemCameraSettings();

    void onReceivedUserVideoFrame(const std::string& accountId, const VideoFrame& frame, bool bSub);
    void onUserVideoStatusChanged(const std::string& accountId, NEMeeting::DeviceStatus deviceStatus);
    void onFocusVideoChanged(const std::string& accountId, bool isFocus);
    void onRemoteUserVideoStats(const std::vector<NEVideoStats>& videoStats);
    void onLocalUserVideoStats(const std::vector<NEVideoStats>& videoStats);
    void onError(uint32_t errorCode, const std::string& errorMessage);
    std::shared_ptr<NEMeetingVideoController> getVideoController();

public:
    int localVideoStatus() const;
    void setLocalVideoStatus(const int& localVideoStatus);

    QString focusAccountId() const;
    void setFocusAccountId(const QString& focusAccountId);

    bool displayVideoStats() const;
    void setDisplayVideoStats(bool displayVideoStats);

    bool hasCameraPermission();

public slots:
    bool setupVideoCanvas(const QString& accountId, QObject* view, bool highQuality, const QString& uuid);
    bool removeVideoCanvas(const QString& accountId, QObject* view);
    void disableLocalVideo(bool disable);
    void disableRemoteVideo(const QString& accountId, bool disable, bool bAllowOpenByself = true);
    void startLocalVideoPreview(QObject* pProvider);
    void stopLocalVideoPreview(QObject* pProvider = nullptr);

    bool subscribeRemoteVideoStream(const QString& accountId, bool highQuality, const QString& uuid);
    bool unSubscribeRemoteVideoStream(const QString& accountId, const QString& uuid);
    void onUserVideoStatusChangedUI(const QString& changedAccountId, NEMeeting::DeviceStatus deviceStatus);
    void onFocusVideoChangedUI(const QString& focusAccountId, bool isFocus);

signals:
    void localVideoStatusChanged();
    void displayVideoStatsChanged();
    void userVideoStatusChanged(const QString& changedAccountId, NEMeeting::DeviceStatus deviceStatus);
    void remoteUserVideoStats(const QJsonArray& userStats);
    void localUserVideoStats(const QJsonArray& userStats);
    void error(int errorCode, const QString& errorMessage);
    void showPermissionWnd();

    // Binding values
    void focusAccountIdChanged(const QString& oldSpeaker, const QString& newSpeaker);

private:
    std::shared_ptr<NEMeetingVideoController> m_videoController = nullptr;
    NEMeeting::DeviceStatus m_localVideoStatus = NEMeeting::DEVICE_DISABLED_BY_DELF;
    static DelegatePtr m_videoFrameDelegate;

    QString m_focusAccountId;
    bool m_displayVideoStats = false;
    std::map<QString, Canvas> m_videoCanvas;
};

#endif  // VIDEOMANAGER_H
