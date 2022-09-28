﻿// Copyright (c) 2022 NetEase, Inc. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "meeting_prochandler.h"
#include <QUuid>
#include <codecvt>
#include <locale>
#include <regex>
#include <set>
#include "manager/auth_manager.h"
#include "manager/global_manager.h"
#include "manager/meeting/audio_manager.h"
#include "manager/meeting/members_manager.h"
#include "manager/meeting/video_manager.h"
#include "manager/meeting/whiteboard_manager.h"
#include "manager/meeting_manager.h"
#include "manager/more_item_manager.h"
#include "manager/settings_manager.h"

#define MENU_ITEM_TITLE_LIMIT 10

NEMeetingServiceProcHandlerIMP::NEMeetingServiceProcHandlerIMP(QObject* parent /*= nullptr*/)
    : QObject(parent) {
    connect(MeetingManager::getInstance(), &MeetingManager::meetingStatusChanged, this, &NEMeetingServiceProcHandlerIMP::onMeetingStatusChanged);
}

bool NEMeetingServiceProcHandlerIMP::onStartMeeting(const NS_I_NEM_SDK::NEStartMeetingParams& param,
                                                    const NS_I_NEM_SDK::NEStartMeetingOptions& opts,
                                                    const NS_I_NEM_SDK::NEMeetingService::NEStartMeetingCallback& cb) {
    YXLOG_API(Info) << "Received start meeting request, meeting ID: " << param.meetingId << ", nickname: " << param.displayName
                    << ", enable audio: " << !opts.noAudio << ", enable video: " << !opts.noVideo << ", enable chatroom: " << !opts.noChat
                    << ", enable invitation: " << !opts.noInvite << ", enable screenShare: " << !opts.noScreenShare
                    << ", enable view: " << !opts.noView << ", meeting show options: " << opts.meetingIdDisplayOption
                    << ", enable sip: " << !opts.noSip << ", enable bAudioAINSEnabled: " << opts.audioAINSEnabled
                    << ", showMemberTag: " << opts.showMemberTag << ", extraData: " << param.extraData << ", joinTimeout: " << opts.joinTimeout
                    << ", enable noMuteAllVideo: " << opts.noMuteAllVideo << ", full_more submenu size: " << opts.full_more_menu_items_.size()
                    << ", full_toolbar submenu size: " << opts.full_toolbar_menu_items_.size()
                    << ", showMeetingRemainingTip: " << opts.showMeetingRemainingTip
                    << ", enableFileMessage: " << opts.chatroomConfig.enableFileMessage
                    << ", enableImageMessage: " << opts.chatroomConfig.enableImageMessage << YXLOGEnd;

    for (auto control : param.controls) {
        YXLOG_API(Info) << "control.attendeeOff" << control.attendeeOff << YXLOGEnd;
        YXLOG_API(Info) << "control.type" << control.type << YXLOGEnd;
    }

    for (auto iter = param.roleBinds.begin(); iter != param.roleBinds.end(); iter++) {
        YXLOG_API(Info) << "roleBinds userId: " << iter->first << ", role: " << iter->second << YXLOGEnd;
    }

    if (m_startMeetingCallback != nullptr) {
        if (cb != nullptr) {
            cb(NS_I_NEM_SDK::ERROR_CODE_FAILED, tr("Frequent operation, please try again later").toStdString());
        }
        return false;
    }

    QStringList listError;
    do {
        if (opts.full_more_menu_items_.size() > 0) {
            std::vector<NEMeetingMenuItem> items(opts.full_toolbar_menu_items_.begin(), opts.full_toolbar_menu_items_.end());
            items.insert(items.end(), opts.full_more_menu_items_.begin(), opts.full_more_menu_items_.end());
            if (!checkOptionsEx(items)) {
                listError
                    << tr("the %1 of the %2 and the %3 cannot be duplicated").arg("itemId", "full_toolbar_menu_items_", "full_more_menu_items_");
            }
            if (!checkOptionsVisibility(opts.full_more_menu_items_, 10)) {
                listError << tr("%1 cannot exceed %2 items").arg("full_more_menu_items_", "10");
            }
            if (!checkOptionsId(opts.full_more_menu_items_, false)) {
                listError << tr("%1 in %2 cannot be less than %3").arg("itemId", "full_more_menu_items_", "kFirstinjectedMenuId");
            }
            if (!checkOptionsExMore(opts.full_more_menu_items_)) {
                listError << tr("%1 can not add %2, %3, %4, %5")
                                 .arg("full_more_menu_items_", "kMicMenuId", "kCameraMenuId", "kMangeParticipantsMenuId", "kChatMenuId");
            }
            if (!checkOptions(opts.full_more_menu_items_, false)) {
                listError << tr("%1 or %2 in %3 is invalid").arg("title", "image", "full_more_menu_items_");
            }
        }
        if (opts.full_more_menu_items_.empty()) {
            if (!checkOptionsEx(opts.full_toolbar_menu_items_)) {
                listError << tr("%1 in %2 cannot be duplicated").arg("itemId", "full_toolbar_menu_items_");
            }
        }
        if (!checkOptionsVisibility(opts.full_toolbar_menu_items_, 7)) {
            listError << tr("%1 cannot exceed %2 items").arg("full_toolbar_menu_items_", "7");
        }
        if (!checkOptionsId(opts.full_toolbar_menu_items_, false)) {
            listError << tr("%1 in %2 cannot be less than %3").arg("itemId", "full_toolbar_menu_items_", "kFirstinjectedMenuId");
        }
        if (!checkOptions(opts.full_toolbar_menu_items_, false)) {
            listError << tr("%1 or %2 in %3 is invalid").arg("title", "image", "full_toolbar_menu_items_");
        }

        auto isDigit = [](const std::string& str) {
            for (char const& c : str) {
                if (std::isdigit(c) == 0)
                    return false;
            }
            return true;
        };
        if (!param.password.empty() && (param.password.length() < 4 || !isDigit(param.password))) {
            listError << tr("The password must contain at least %1 digits").arg(4);
        }
    } while (0);

    if (!listError.isEmpty()) {
        if (cb) {
            QString strError;
            strError.append(tr("Invalid params:"));
            strError.append("\n");
            int listSize = listError.size();
            for (int i = 0; i < listSize; i++) {
                strError.append(listError.at(i));
                if (i != listSize - 1) {
                    strError.append("\n");
                }
            }
            cb(NS_I_NEM_SDK::MEETING_ERROR_FAILED_PARAM_ERROR, strError.toStdString());
        }
        return false;
    }

    auto meetingStatus = MeetingManager::getInstance()->getRoomStatus();
    if (NEMeeting::MEETING_IDLE != meetingStatus) {
        YXLOG(Info) << "getRoomStatus, meetingStatus: " << meetingStatus << YXLOGEnd;
        if (cb) {
            cb(NS_I_NEM_SDK::MEETING_ERROR_ALREADY_INMEETING, tr("The last meeting is not end yet.").toStdString());
        }

        return true;
    }

    if (!param.meetingId.empty() && param.meetingId != AuthManager::getInstance()->getAuthInfo().personalRoomId) {
        YXLOG_API(Info) << "Only supports personal meeting ID" << YXLOGEnd;
        if (cb) {
            cb(NS_I_NEM_SDK::MEETING_ERROR_FAILED_PARAM_ERROR, tr("Only supports personal meeting ID.").toStdString());
        }
        return false;
    }

    m_startMeetingCallback = cb;

    Invoker::getInstance()->execute([=]() {
        NERoomOptions options;
        if (!opts.noVideo && !VideoManager::getInstance()->hasCameraPermission()) {
            options.noVideo = true;
        } else {
            options.noVideo = opts.noVideo;
        }

        if (!opts.noAudio && !AudioManager::getInstance()->hasMicrophonePermission()) {
            options.noAudio = true;
        } else {
            options.noAudio = opts.noAudio;
        }

        options.noAutoOpenWhiteboard = !((opts.defaultWindowMode == WHITEBOARD_MODE) &&
                                         GlobalManager::getInstance()->getGlobalConfig()->isWhiteboardSupported() && !opts.noWhiteboard);
        options.noCloudRecord = opts.noCloudRecord;
        options.noChat = opts.noChat;
        options.noSip = opts.noSip;

        WhiteboardManager::getInstance()->setAutoOpenWhiteboard(!options.noAutoOpenWhiteboard);
        MeetingManager::getInstance()->setMeetingIdDisplayOption((NEMeeting::MeetingIdDisplayOptions)opts.meetingIdDisplayOption);
        MeetingManager::getInstance()->setShowMeetingRemainingTip(opts.showMeetingRemainingTip &&
                                                                  GlobalManager::getInstance()->getGlobalConfig()->isEndTimeTipSupported());
        MeetingManager::getInstance()->setHideChatroom(opts.noChat);
        MeetingManager::getInstance()->setHideInvitation(opts.noInvite);
        MeetingManager::getInstance()->setHideScreenShare(opts.noScreenShare);
        MeetingManager::getInstance()->setHideView(opts.noView);
        MeetingManager::getInstance()->setHideWhiteboard(opts.noWhiteboard);
        MeetingManager::getInstance()->setHideSip(opts.noSip);
        MeetingManager::getInstance()->setHideMuteAllVideo(opts.noMuteAllVideo);
        MeetingManager::getInstance()->setHideMuteAllAudio(opts.noMuteAllAudio);
        MeetingManager::getInstance()->setReName(!opts.noRename);
        MeetingManager::getInstance()->setJoinTimeout(opts.joinTimeout <= 0 ? 45 * 1000 : opts.joinTimeout);
        MeetingManager::getInstance()->setShowMemberTag(opts.showMemberTag);
        MeetingManager::getInstance()->setEnableFileMessage(opts.chatroomConfig.enableFileMessage &&
                                                            GlobalManager::getInstance()->getGlobalConfig()->isFileMessageSupported());
        MeetingManager::getInstance()->setEnableImageMessage(opts.chatroomConfig.enableImageMessage &&
                                                             GlobalManager::getInstance()->getGlobalConfig()->isImageMessageSupported());
        MoreItemManager::getInstance()->setMoreItemInjected(opts.full_more_menu_items_.empty());
        SettingsManager::getInstance()->setEnableAudioAINS(opts.audioAINSEnabled);
        if (opts.full_more_menu_items_.size() > 0) {
            QVector<MoreItem> items;
            for (auto& item : opts.full_more_menu_items_) {
                YXLOG(Info) << "Insert submenu to more button list, menu id: " << item.itemId << ", title: " << item.itemTitle
                            << ", image: " << item.itemImage << YXLOGEnd;
                MoreItem it;
                it.itemIndex = item.itemId;
                it.itemGuid = QUuid::createUuid().toString();
                it.itemTitle = QString::fromStdString(item.itemTitle);
                it.itemImagePath = MoreItemManager::getInstance()->strPathToPath(item.itemImage);
                it.itemTitle2 = QString::fromStdString(item.itemTitle2);
                it.itemImagePath2 = MoreItemManager::getInstance()->strPathToPath(item.itemImage2);
                it.itemVisibility = MoreItemManager::getInstance()->switchVisibility(item.itemVisibility);
                it.itemCheckedIndex = item.itemCheckedIndex;
                items.push_back(it);
            }
            MoreItemManager::getInstance()->initializeMore(items);
        }

        if (opts.full_more_menu_items_.empty()) {
            MoreItemManager::getInstance()->restoreMore();
        }

        if (opts.full_toolbar_menu_items_.size() != 0) {
            QVector<MoreItem> items;
            for (auto& item : opts.full_toolbar_menu_items_) {
                YXLOG(Info) << "Insert submenu to toolbar button list, menu id: " << item.itemId << ", title: " << item.itemTitle
                            << ", image: " << item.itemImage << YXLOGEnd;
                MoreItem it;
                it.itemIndex = item.itemId;
                it.itemGuid = QUuid::createUuid().toString();
                it.itemTitle = QString::fromStdString(item.itemTitle);
                it.itemImagePath = MoreItemManager::getInstance()->strPathToPath(item.itemImage);
                it.itemTitle2 = QString::fromStdString(item.itemTitle2);
                it.itemImagePath2 = MoreItemManager::getInstance()->strPathToPath(item.itemImage2);
                it.itemVisibility = MoreItemManager::getInstance()->switchVisibility(item.itemVisibility);
                it.itemCheckedIndex = item.itemCheckedIndex;
                items.push_back(it);
            }
            MoreItemManager::getInstance()->initializeToolbar(items);
        } else {
            MoreItemManager::getInstance()->restoreToolbar();
        }
        MeetingManager::getInstance()->createMeeting(param, options);
    });

    return true;
}

bool NEMeetingServiceProcHandlerIMP::onJoinMeeting(const NS_I_NEM_SDK::NEJoinMeetingParams& param,
                                                   const NS_I_NEM_SDK::NEJoinMeetingOptions& opts,
                                                   const NS_I_NEM_SDK::NEMeetingService::NEJoinMeetingCallback& cb) {
    YXLOG_API(Info) << "Received join meeting request" << YXLOGEnd;
    return joinMeeting(param, opts, cb);
}

bool NEMeetingServiceProcHandlerIMP::onAnonymousJoinMeeting(const NEJoinMeetingParams& param,
                                                            const NEJoinMeetingOptions& opts,
                                                            const NEMeetingService::NEJoinMeetingCallback& cb) {
    YXLOG_API(Info) << "Received anonymous join meeting request" << YXLOGEnd;

    Invoker::getInstance()->execute([=]() {
        AuthManager::getInstance()->doAnonymousLogin([=](int code, const std::string& msg) {
            if (code == 0) {
                joinMeeting(param, opts, cb);
            } else {
                if (cb) {
                    cb((nem_sdk_interface::NEErrorCode)code, msg);
                }
            }
        });
    });
    return true;
}

bool NEMeetingServiceProcHandlerIMP::onLeaveMeeting(bool finish, const nem_sdk_interface::NEMeetingService::NELeaveMeetingCallback& cb) {
    YXLOG_API(Info) << "Received onLeaveMeeting, finish: " << finish << YXLOGEnd;
    if (m_leaveMeetingCallback != nullptr) {
        if (cb != nullptr) {
            cb(NS_I_NEM_SDK::ERROR_CODE_FAILED, tr("Frequent operation, please try again later").toStdString());
        }

        return false;
    }

    m_leaveMeetingCallback = cb;
    Invoker::getInstance()->execute([=]() {
        if (finish) {
            if (kAuthLoginSuccessed != AuthManager::getInstance()->getAuthStatus()) {
                cb(NS_I_NEM_SDK::ERROR_CODE_FAILED, tr("Did not logged in.").toStdString());
                m_leaveMeetingCallback = nullptr;
                return;
            }

            auto meetingStatus = MeetingManager::getInstance()->getRoomStatus();
            if (meetingStatus != NEMeeting::MEETING_CONNECTED && meetingStatus != NEMeeting::MEETING_RECONNECTED) {
                cb(NS_I_NEM_SDK::ERROR_CODE_FAILED, tr("The meeting has not yet started.").toStdString());
                m_leaveMeetingCallback = nullptr;
                return;
            }

            if (MeetingManager::getInstance()->getMeetingInfo().hostAccountId != AuthManager::getInstance()->getAuthInfo().accountId) {
                cb(NS_I_NEM_SDK::ERROR_CODE_FAILED, tr("You have no permission").toStdString());
                m_leaveMeetingCallback = nullptr;
                return;
            }

            MeetingManager::getInstance()->leaveMeeting(true);
        } else {
            auto meetingStatus = MeetingManager::getInstance()->getRoomStatus();
            if (meetingStatus != NEMeeting::MEETING_CONNECTED && meetingStatus != NEMeeting::MEETING_RECONNECTED) {
                cb(NS_I_NEM_SDK::ERROR_CODE_FAILED, "The meeting has not yet started.");
                m_leaveMeetingCallback = nullptr;
                return;
            }
            MeetingManager::getInstance()->leaveMeeting(false);
        }
    });

    return true;
}

bool NEMeetingServiceProcHandlerIMP::onGetCurrentMeetingInfo(const nem_sdk_interface::NEMeetingService::NEGetMeetingInfoCallback& cb) {
    YXLOG_API(Info) << "Received get current meeting info request." << YXLOGEnd;
    Invoker::getInstance()->execute([=]() {
        auto authInfo = AuthManager::getInstance()->getAuthInfo();

        if (kAuthLoginSuccessed != AuthManager::getInstance()->getAuthStatus()) {
            cb(NS_I_NEM_SDK::ERROR_CODE_FAILED, "", NS_I_NEM_SDK::NEMeetingInfo());
            return;
        }

        auto meetingStatus = MeetingManager::getInstance()->getRoomStatus();
        if (meetingStatus != NEMeeting::MEETING_CONNECTED && meetingStatus != NEMeeting::MEETING_RECONNECTED) {
            cb(NS_I_NEM_SDK::ERROR_CODE_FAILED, "", NS_I_NEM_SDK::NEMeetingInfo());
            return;
        }

        auto meetingInfo = MeetingManager::getInstance()->getMeetingInfo();
        NS_I_NEM_SDK::NEMeetingInfo info;
        info.isHost = authInfo.accountId == meetingInfo.hostAccountId;
        info.isLocked = meetingInfo.lock;
        info.meetingId = meetingInfo.roomId;
        info.shortMeetingId = meetingInfo.shortRoomId;
        info.duration = MeetingManager::getInstance()->meetingDuration() * 1000;
        info.hostUserId = meetingInfo.hostAccountId;
        info.meetingUniqueId = std::atoll(meetingInfo.roomUniqueId.c_str());
        info.password = meetingInfo.password;
        info.subject = meetingInfo.subject;
        info.startTime = meetingInfo.startTime;
        info.sipId = meetingInfo.sipId;
        info.extraData = meetingInfo.extraData;
        info.meetingCreatorId = meetingInfo.creatorUserId;
        info.meetingCreatorName = meetingInfo.creatorUserName;
        if (NEMeetingType::schduleType == meetingInfo.type) {
            info.scheduleStartTime = meetingInfo.scheduleTimeBegin;
            info.scheduleEndTime = meetingInfo.scheduleTimeEnd;
        } else {
            info.scheduleStartTime = -1;
            info.scheduleEndTime = -1;
        }

        QVector<MemberInfo> userList = MembersManager::getInstance()->items();
        for (auto item : userList) {
            NS_I_NEM_SDK::NEInMeetingUserInfo user;
            user.userId = item.accountId.toStdString();
            user.userName = item.nickname.toStdString();
            user.tag = item.tag.toStdString();
            info.userList.push_back(user);
        }
        YXLOG(Info) << "Got current meeting info, meeting ID: " << info.meetingId << ", shortId: " << info.shortMeetingId
                    << ", is host: " << info.isHost << ", is locked: " << info.isLocked << ", duration: " << info.duration
                    << ", sipId: " << info.sipId << YXLOGEnd;
        cb(NS_I_NEM_SDK::ERROR_CODE_SUCCESS, "", info);
    });

    return true;
}

void NEMeetingServiceProcHandlerIMP::onGetPresetMenuItems(const std::vector<int>& menuItemsId,
                                                          const NS_I_NEM_SDK::NEMeetingService::NEGetPresetMenuItemsCallback& cb) {
    YXLOG_API(Info) << "Received onGetPresetMenuItems." << YXLOGEnd;
    if (!cb) {
        return;
    }

    Invoker::getInstance()->execute([=]() {
        std::vector<NS_I_NEM_SDK::NEMeetingMenuItem> items;
        std::vector<MoreItem> vItems;
        MoreItemManager::getInstance()->getPresetItems(menuItemsId, vItems);
        for (auto& it : vItems) {
            NS_I_NEM_SDK::NEMeetingMenuItem item;
            item.itemId = it.itemIndex;
            item.itemGuid = it.itemGuid.toStdString();
            item.itemTitle = it.itemTitle.toStdString();
            item.itemImage = it.itemImagePath.toStdString();
            item.itemTitle2 = it.itemTitle2.toStdString();
            item.itemImage2 = it.itemImagePath2.toStdString();
            item.itemVisibility = MoreItemManager::getInstance()->switchVisibility(it.itemVisibility);
            item.itemCheckedIndex = it.itemCheckedIndex;
            items.push_back(item);
        }

        cb(NS_I_NEM_SDK::ERROR_CODE_SUCCESS, "", items);
    });
}

void NEMeetingServiceProcHandlerIMP::onInjectedMenuItemClickExReturn(int itemId, const std::string& itemGuid, int itemCheckedIndex) {
    YXLOG(Info) << "Received itemId: " << itemId << " ,itemGuid: " << itemGuid << ", itemCheckedIndex: " << itemCheckedIndex << YXLOGEnd;
    MoreItemManager::getInstance()->modifyItem(itemId, QString::fromStdString(itemGuid), itemCheckedIndex);
}

void NEMeetingServiceProcHandlerIMP::onSubscribeRemoteAudioStream(const std::string& accountId,
                                                                  bool subscribe,
                                                                  const NS_I_NEM_SDK::NEEmptyCallback& cb) {
    //    YXLOG_API(Info) << "Received onSubscribeRemoteAudioStream, accountId: " << accountId << ", subscribe: " << subscribe << YXLOGEnd;
    //    auto meetingInfo = MeetingManager::getInstance()->getMeetingInfo();
    //    if (!meetingInfo) {
    //        if (cb) {
    //            cb((nem_sdk_interface::NEErrorCode)2200, tr("The meeting is not in progress").toStdString());
    //        }
    //        return;
    //    }

    //    Invoker::getInstance()->execute([=]() {
    //        auto membersController = MeetingManager::getInstance()->getUserController();
    //        auto audioController = MeetingManager::getInstance()->getInRoomAudioController();
    //        if (!membersController || !audioController) {
    //            if (cb) {
    //                cb((nem_sdk_interface::NEErrorCode)2200, tr("The meeting is not in progress").toStdString());
    //            }
    //            return;
    //        }

    //        if (QString::fromStdString(accountId).trimmed().isEmpty()) {
    //            if (cb) {
    //                cb((nem_sdk_interface::NEErrorCode)300, tr("%1 is null").arg("accountId").toStdString());
    //            }
    //            return;
    //        } else if (accountId == AuthManager::getInstance()->getAuthInfo()->getAccountId()) {
    //            if (cb) {
    //                cb((nem_sdk_interface::NEErrorCode)300, tr("You can't subscribe to your own audio").toStdString());
    //            }
    //            return;
    //        }

    //        SharedUserPtr memberPtr = GlobalManager::getInstance()->getInRoomService()->getUserInfoById(accountId);
    //        if (!memberPtr) {
    //            if (cb) {
    //                cb((nem_sdk_interface::NEErrorCode)2101, tr("Member is not in the meeting").toStdString());
    //            }
    //            return;
    //        }

    //        if (subscribe) {
    //            audioController->subscribeRemoteAudioStream(std::vector<std::string>{accountId});
    //        } else {
    //            audioController->unsubscribeRemoteAudioStream(std::vector<std::string>{accountId});
    //        }

    //        if (cb) {
    //            cb(NS_I_NEM_SDK::ERROR_CODE_SUCCESS, "");
    //        }
    //    });
}

void NEMeetingServiceProcHandlerIMP::onSubscribeRemoteAudioStreams(const std::vector<std::string>& accountIdList,
                                                                   bool subscribe,
                                                                   const NS_I_NEM_SDK::NEEmptyCallback& cb) {
    std::string strTmp;
    for (auto& it : accountIdList) {
        strTmp += it;
    }
    //    YXLOG_API(Info) << "Received onSubscribeRemoteAudioStreams, accountIdList: " << strTmp << ", subscribe: " << subscribe << YXLOGEnd;
    //    auto meetingInfo = MeetingManager::getInstance()->getMeetingInfo();
    //    if (!meetingInfo) {
    //        if (cb) {
    //            cb((nem_sdk_interface::NEErrorCode)2200, tr("The meeting is not in progress").toStdString());
    //        }
    //        return;
    //    }

    //    Invoker::getInstance()->execute([=]() {
    //        auto membersController = MeetingManager::getInstance()->getUserController();
    //        auto audioController = MeetingManager::getInstance()->getInRoomAudioController();
    //        if (!membersController || !audioController) {
    //            if (cb) {
    //                cb((nem_sdk_interface::NEErrorCode)2200, tr("The meeting is not in progress").toStdString());
    //            }
    //            return;
    //        }

    //        if (accountIdList.empty()) {
    //            if (cb) {
    //                cb((nem_sdk_interface::NEErrorCode)300, tr("%1 list is null").arg("accountId").toStdString());
    //            }
    //            return;
    //        } else {
    //            auto authAccountId = AuthManager::getInstance()->getAuthInfo()->getAccountId();
    //            auto it =
    //                std::find_if(accountIdList.begin(), accountIdList.end(), [authAccountId](const std::string& id) { return id == authAccountId;
    //                });
    //            if (accountIdList.end() != it) {
    //                if (cb) {
    //                    cb((nem_sdk_interface::NEErrorCode)300, tr("You can't subscribe to your own audio").toStdString());
    //                }
    //                return;
    //            }

    //            std::vector<std::string> accountIdListTmp;
    //            accountIdListTmp.reserve(accountIdList.size());
    //            std::list<std::string> strInvaildIdList;
    //            std::vector<SharedUserPtr> members;
    //            GlobalManager::getInstance()->getInRoomService()->getAllUsers(members);
    //            for (auto& id : accountIdList) {
    //                auto it = std::find_if(members.begin(), members.end(), [id](const SharedUserPtr& accountId) { return id ==
    //                accountId->getUserId(); }); if (members.end() == it) {
    //                    if (strInvaildIdList.empty()) {
    //                        strInvaildIdList.push_back(id);
    //                    } else {
    //                        strInvaildIdList.push_back(",");
    //                        strInvaildIdList.push_back(id);
    //                    }
    //                } else {
    //                    accountIdListTmp.push_back(id);
    //                }
    //            }

    //            if (!accountIdList.empty()) {
    //                if (subscribe) {
    //                    audioController->subscribeRemoteAudioStream(accountIdListTmp);
    //                } else {
    //                    audioController->unsubscribeRemoteAudioStream(accountIdListTmp);
    //                }
    //            }

    //            if (!strInvaildIdList.empty()) {
    //                if (cb) {
    //                    std::string strTmp = std::accumulate(strInvaildIdList.begin(), strInvaildIdList.end(), std::string(""),
    //                                                         [](const std::string& str1, const std::string& str2) -> std::string { return str1 +
    //                                                         str2; });
    //                    cb((nem_sdk_interface::NEErrorCode)2101,
    //                       tr("Some members were absent from the meeting,(%1)").arg(QString::fromStdString(strTmp)).toStdString());
    //                }
    //            } else {
    //                cb(NS_I_NEM_SDK::ERROR_CODE_SUCCESS, "");
    //            }
    //            return;
    //        }
    //    });
}

void NEMeetingServiceProcHandlerIMP::onSubscribeAllRemoteAudioStreams(bool subscribe, const NS_I_NEM_SDK::NEEmptyCallback& cb) {
    YXLOG_API(Info) << "Received onSubscribeAllRemoteAudioStreams, subscribe: " << subscribe << YXLOGEnd;
    /*    auto meetingInfo = MeetingManager::getInstance()->getMeetingInfo();
        if (!meetingInfo) {
            if (cb) {
                cb((nem_sdk_interface::NEErrorCode)2200, tr("The meeting is not in progress").toStdString());
            }
            return;
        }

        Invoker::getInstance()->execute([=]() {
            auto audioController = MeetingManager::getInstance()->getInRoomAudioController();
            if (!audioController) {
                if (cb) {
                    cb((nem_sdk_interface::NEErrorCode)2200, tr("The meeting is not in progress").toStdString());
                }
                return;
            }

            if (subscribe) {
                audioController->subscribeAllRemoteAudioStream();
            } else {
                audioController->unsubscribeAllRemoteAudioStream();
            }

            if (cb) {
                cb(NS_I_NEM_SDK::ERROR_CODE_SUCCESS, "");
            }
        })*/
    ;
}

NS_I_NEM_SDK::NEErrorCode NEMeetingServiceProcHandlerIMP::convertExtentedCode(int extentedCode) {
    NS_I_NEM_SDK::NEErrorCode resultCode = (NS_I_NEM_SDK::NEErrorCode)extentedCode;
    switch (extentedCode) {
        case 0:
        case 200:
            resultCode = NS_I_NEM_SDK::ERROR_CODE_SUCCESS;
            break;
        case 3100:
            resultCode = NS_I_NEM_SDK::MEETING_ERROR_FAILED_ALREADY_IN_MEETING;
            break;
        default:
            break;
    }
    return resultCode;
}

void NEMeetingServiceProcHandlerIMP::onMeetingStatusChanged(NEMeeting::Status status, int errorCode, const QString& errorMessage) {
    YXLOG_API(Info) << "Meeting process handler, meeting status changed: " << status << ", extended code: " << errorCode
                    << ", errorMessage: " << errorMessage.toStdString() << YXLOGEnd;

    NS_I_NEM_SDK::NEErrorCode responseCode = NS_I_NEM_SDK::ERROR_CODE_FAILED;
    if (NEMeeting::MEETING_CONNECT_FAILED == status &&
        ("joinTimeoutTimer" == errorMessage || "kReasonRoomNotExist" == errorMessage || "kReasonSyncDataError" == errorMessage ||
         "kReasonRtcInitError" == errorMessage || "kReasonJoinChannelError" == errorMessage)) {
        responseCode = NS_I_NEM_SDK::ERROR_CODE_SUCCESS;
        if (m_startMeetingCallback != nullptr) {
            YXLOG(Info) << "Invoke start meeting callback, error code: " << errorCode << ", error message: " << errorMessage.toStdString()
                        << YXLOGEnd;
            m_startMeetingCallback(responseCode, "");
            m_startMeetingCallback = nullptr;
        }

        if (m_joinMeetingCallback != nullptr) {
            YXLOG(Info) << "Invoke join meeting callback, error code: " << errorCode << ", error message: " << errorMessage.toStdString() << YXLOGEnd;
            m_joinMeetingCallback(responseCode, "");
            m_joinMeetingCallback = nullptr;
        }
        return;
    }
    switch (status) {
        case NEMeeting::MEETING_IDLE:        // 初始状态
        case NEMeeting::MEETING_CONNECTING:  // 连接中
        case NEMeeting::MEETING_PREPARING:   // 准备数据
            return;
        case NEMeeting::MEETING_CONNECTED:  // 连接成功
            responseCode = convertExtentedCode(errorCode);
            break;
        case NEMeeting::MEETING_ENDED:  // 会议正常结束
            responseCode = convertExtentedCode(errorCode);
            break;
        case NEMeeting::MEETING_CONNECT_FAILED:  // 连接失败
            responseCode = convertExtentedCode(errorCode);
            break;
        default:
            return;
    }

    if (m_startMeetingCallback != nullptr) {
        YXLOG(Info) << "Invoke start meeting callback, error code: " << errorCode << ", error message: " << errorMessage.toStdString() << YXLOGEnd;
        m_startMeetingCallback(responseCode, errorMessage.toStdString());
        m_startMeetingCallback = nullptr;
    }

    if (m_joinMeetingCallback != nullptr) {
        YXLOG(Info) << "Invoke join meeting callback, error code: " << errorCode << ", error message: " << errorMessage.toStdString() << YXLOGEnd;
        m_joinMeetingCallback(responseCode, errorMessage.toStdString());
        m_joinMeetingCallback = nullptr;
    }

    if (m_leaveMeetingCallback != nullptr) {
        YXLOG(Info) << "Invoke leave meeting callback, error code: " << errorCode << ", error message: " << errorMessage.toStdString() << YXLOGEnd;
        m_leaveMeetingCallback(responseCode, errorMessage.toStdString());
        m_leaveMeetingCallback = nullptr;
    }
}

bool NEMeetingServiceProcHandlerIMP::checkOptionsEx(const std::vector<NEMeetingMenuItem>& items) {
    if (items.empty()) {
        return true;
    }

    return std::set<NEMeetingMenuItem>(items.begin(), items.end()).size() == items.size();
}

bool NEMeetingServiceProcHandlerIMP::checkOptionsId(const std::vector<NS_I_NEM_SDK::NEMeetingMenuItem>& items, bool bInjected) {
    std::vector<int> vExclude = {kMicMenuId,    kCameraMenuId, kScreenShareMenuId, kParticipantsMenuId, kMangeParticipantsMenuId,
                                 kInviteMenuId, kChatMenuId,   kViewMenuId,        kWhiteboardMenuId};
    for (auto& it : items) {
        // ID 索引校验
        if (!bInjected && vExclude.end() != std::find(vExclude.begin(), vExclude.end(), it.itemId)) {
            continue;
        }
        if (it.itemId < kFirstinjectedMenuId) {
            return false;
        }
    }

    return true;
}

bool NEMeetingServiceProcHandlerIMP::joinMeeting(const NEJoinMeetingParams& param,
                                                 const NEJoinMeetingOptions& opts,
                                                 const NEMeetingService::NEJoinMeetingCallback& cb) {
    QString strPassword = param.password.empty() ? "null" : "not null";
    YXLOG_API(Info) << param.meetingId << ", nickname: " << param.displayName << ", password: " << strPassword.toStdString()
                    << ", enable audio: " << !opts.noAudio << ", enable video: " << !opts.noVideo << ", enable chatroom: " << !opts.noChat
                    << ", enable invitation: " << !opts.noInvite << ", enable ScreenShare: " << !opts.noScreenShare
                    << ", enable View: " << !opts.noView << ", enable sip: " << !opts.noSip << ", enable bAudioAINSEnabled: " << opts.audioAINSEnabled
                    << ", meeting show options: " << opts.meetingIdDisplayOption << ", joinTimeout: " << opts.joinTimeout
                    << ", enable noMuteAllVideo: " << opts.noMuteAllVideo << ", full_more submenu size: " << opts.full_more_menu_items_.size()
                    << ", full_toolbar submenu size: " << opts.full_toolbar_menu_items_.size()
                    << ", showMeetingRemainingTip: " << opts.showMeetingRemainingTip
                    << ", enableFileMessage: " << opts.chatroomConfig.enableFileMessage
                    << ", enableImageMessage: " << opts.chatroomConfig.enableImageMessage << YXLOGEnd;

    if (m_joinMeetingCallback != nullptr) {
        if (cb != nullptr) {
            cb(NS_I_NEM_SDK::ERROR_CODE_FAILED, tr("Frequent operation, please try again later").toStdString());
        }
        return false;
    }

    QStringList listError;
    do {
        if (opts.full_more_menu_items_.size() > 0) {
            std::vector<NEMeetingMenuItem> items(opts.full_toolbar_menu_items_.begin(), opts.full_toolbar_menu_items_.end());
            items.insert(items.end(), opts.full_more_menu_items_.begin(), opts.full_more_menu_items_.end());
            if (!checkOptionsEx(items)) {
                listError
                    << tr("the %1 of the %2 and the %3 cannot be duplicated").arg("itemId", "full_toolbar_menu_items_", "full_more_menu_items_");
            }
            if (!checkOptionsVisibility(opts.full_more_menu_items_, 10)) {
                listError << tr("%1 cannot exceed %2 items").arg("full_more_menu_items_", "10");
            }
            if (!checkOptionsId(opts.full_more_menu_items_, false)) {
                listError << tr("%1 in %2 cannot be less than %3").arg("itemId", "full_more_menu_items_", "kFirstinjectedMenuId");
            }
            if (!checkOptionsExMore(opts.full_more_menu_items_)) {
                listError << tr("%1 can not add %2, %3, %4, %5")
                                 .arg("full_more_menu_items_", "kMicMenuId", "kCameraMenuId", "kMangeParticipantsMenuId", "kChatMenuId");
            }
            if (!checkOptions(opts.full_more_menu_items_, false)) {
                listError << tr("%1 or %2 in %3 is invalid").arg("title", "image", "full_more_menu_items_");
            }
        }
        if (opts.full_more_menu_items_.empty()) {
            if (!checkOptionsEx(opts.full_toolbar_menu_items_)) {
                listError << tr("%1 in %2 cannot be duplicated").arg("itemId", "full_toolbar_menu_items_");
            }
        }
        if (!checkOptionsVisibility(opts.full_toolbar_menu_items_, 7)) {
            listError << tr("%1 cannot exceed %2 items").arg("full_toolbar_menu_items_", "7");
        }
        if (!checkOptionsId(opts.full_toolbar_menu_items_, false)) {
            listError << tr("%1 in %2 cannot be less than %3").arg("itemId", "full_toolbar_menu_items_", "kFirstinjectedMenuId");
        }
        if (!checkOptions(opts.full_toolbar_menu_items_, false)) {
            listError << tr("%1 or %2 in %3 is invalid").arg("title", "image", "full_toolbar_menu_items_");
        }
    } while (0);

    if (!listError.isEmpty()) {
        if (cb) {
            QString strError;
            strError.append(tr("Invalid params:"));
            strError.append("\n");
            int listSize = listError.size();
            for (int i = 0; i < listSize; i++) {
                strError.append(listError.at(i));
                if (i != listSize - 1) {
                    strError.append("\n");
                }
            }
            cb(NS_I_NEM_SDK::MEETING_ERROR_FAILED_PARAM_ERROR, strError.toStdString());
        }
        return false;
    }

    auto meetingStatus = MeetingManager::getInstance()->getRoomStatus();
    if (NEMeeting::MEETING_IDLE != meetingStatus) {
        if (cb) {
            cb(NS_I_NEM_SDK::MEETING_ERROR_ALREADY_INMEETING, tr("The last meeting is not end yet.").toStdString());
        }

        return true;
    }

    m_joinMeetingCallback = cb;

    Invoker::getInstance()->execute([=]() {
        MeetingManager::getInstance()->setMeetingIdDisplayOption((NEMeeting::MeetingIdDisplayOptions)opts.meetingIdDisplayOption);
        MeetingManager::getInstance()->setShowMeetingRemainingTip(opts.showMeetingRemainingTip &&
                                                                  GlobalManager::getInstance()->getGlobalConfig()->isEndTimeTipSupported());
        MeetingManager::getInstance()->setHideChatroom(opts.noChat);
        MeetingManager::getInstance()->setHideInvitation(opts.noInvite);
        MeetingManager::getInstance()->setHideScreenShare(opts.noScreenShare);
        MeetingManager::getInstance()->setHideView(opts.noView);
        MeetingManager::getInstance()->setHideWhiteboard(opts.noWhiteboard);
        MeetingManager::getInstance()->setHideSip(opts.noSip);
        MeetingManager::getInstance()->setHideMuteAllVideo(opts.noMuteAllVideo);
        MeetingManager::getInstance()->setHideMuteAllAudio(opts.noMuteAllAudio);
        MeetingManager::getInstance()->setReName(!opts.noRename);
        MeetingManager::getInstance()->setJoinTimeout(opts.joinTimeout <= 0 ? 45 * 1000 : opts.joinTimeout);
        MeetingManager::getInstance()->setShowMemberTag(opts.showMemberTag);
        MeetingManager::getInstance()->setEnableFileMessage(opts.chatroomConfig.enableFileMessage);
        MeetingManager::getInstance()->setEnableImageMessage(opts.chatroomConfig.enableImageMessage);
        MoreItemManager::getInstance()->setMoreItemInjected(opts.full_more_menu_items_.empty());
        SettingsManager::getInstance()->setEnableAudioAINS(opts.audioAINSEnabled);
        if (opts.full_more_menu_items_.size() > 0) {
            QVector<MoreItem> items;
            for (auto& item : opts.full_more_menu_items_) {
                YXLOG(Info) << "Insert submenu to more button list, menu id: " << item.itemId << ", title: " << item.itemTitle
                            << ", image: " << item.itemImage << YXLOGEnd;
                MoreItem it;
                it.itemIndex = item.itemId;
                it.itemGuid = QUuid::createUuid().toString();
                it.itemTitle = QString::fromStdString(item.itemTitle);
                it.itemImagePath = MoreItemManager::getInstance()->strPathToPath(item.itemImage);
                it.itemTitle2 = QString::fromStdString(item.itemTitle2);
                it.itemImagePath2 = MoreItemManager::getInstance()->strPathToPath(item.itemImage2);
                it.itemVisibility = MoreItemManager::getInstance()->switchVisibility(item.itemVisibility);
                it.itemCheckedIndex = item.itemCheckedIndex;
                items.push_back(it);
            }
            MoreItemManager::getInstance()->initializeMore(items);
        }

        if (opts.full_more_menu_items_.empty()) {
            MoreItemManager::getInstance()->restoreMore();
        }

        if (opts.full_toolbar_menu_items_.size() != 0) {
            QVector<MoreItem> items;
            for (auto& item : opts.full_toolbar_menu_items_) {
                YXLOG(Info) << "Insert submenu to toolbar button list, menu id: " << item.itemId << ", title: " << item.itemTitle
                            << ", image: " << item.itemImage << YXLOGEnd;
                MoreItem it;
                it.itemIndex = item.itemId;
                it.itemGuid = QUuid::createUuid().toString();
                it.itemTitle = QString::fromStdString(item.itemTitle);
                it.itemImagePath = MoreItemManager::getInstance()->strPathToPath(item.itemImage);
                it.itemTitle2 = QString::fromStdString(item.itemTitle2);
                it.itemImagePath2 = MoreItemManager::getInstance()->strPathToPath(item.itemImage2);
                it.itemVisibility = MoreItemManager::getInstance()->switchVisibility(item.itemVisibility);
                it.itemCheckedIndex = item.itemCheckedIndex;
                items.push_back(it);
            }
            MoreItemManager::getInstance()->initializeToolbar(items);
        } else {
            MoreItemManager::getInstance()->restoreToolbar();
        }

        if (AuthManager::getInstance()->getAuthStatus() == kAuthLoginSuccessed) {
            YXLOG(Info) << "Invoke join meeting." << YXLOGEnd;
        } else {
            YXLOG(Info) << "Invoke anonjoin meeting." << YXLOGEnd;
        }

        NERoomOptions option_;
        option_.noVideo = opts.noVideo;
        option_.noAudio = opts.noAudio;
        option_.noAutoOpenWhiteboard = !((opts.defaultWindowMode == WHITEBOARD_MODE) &&
                                         GlobalManager::getInstance()->getGlobalConfig()->isWhiteboardSupported() && !opts.noWhiteboard);

        WhiteboardManager::getInstance()->setAutoOpenWhiteboard(!option_.noAutoOpenWhiteboard);
        MeetingManager::getInstance()->joinMeeting(param, option_);
    });

    return true;
}

bool NEMeetingServiceProcHandlerIMP::checkOptionsExMore(const std::vector<NEMeetingMenuItem>& items) {
    std::vector<int> v;
    std::vector<int> vExclude = {kMicMenuId, kCameraMenuId, kMangeParticipantsMenuId, kChatMenuId};
    std::vector<int> vItems;
    std::for_each(items.begin(), items.end(), [&vItems](const NEMeetingMenuItem& item) { vItems.push_back(item.itemId); });
    sort(vExclude.begin(), vExclude.end());
    sort(vItems.begin(), vItems.end());
    set_intersection(vExclude.begin(), vExclude.end(), vItems.begin(), vItems.end(), back_inserter(v));
    return v.empty();
}

bool NEMeetingServiceProcHandlerIMP::checkOptionsVisibility(const std::vector<NEMeetingMenuItem>& items, int lengthLimit) {
    int iHost = 0;
    int iNoHost = 0;
    for (auto& it : items) {
        if (NEMenuVisibility::VISIBLE_ALWAYS == it.itemVisibility || NEMenuVisibility::VISIBLE_EXCLUDE_HOST == it.itemVisibility) {
            iNoHost++;
        }
        if (NEMenuVisibility::VISIBLE_ALWAYS == it.itemVisibility || NEMenuVisibility::VISIBLE_TO_HOST_ONLY == it.itemVisibility) {
            iHost++;
        }
    }

    return iHost <= lengthLimit && iNoHost <= lengthLimit;
}

bool NEMeetingServiceProcHandlerIMP::checkOptions(const std::vector<NEMeetingMenuItem>& items, bool bInjected) {
    if (items.size() > 0) {
        for (auto item = items.begin(); item != items.end(); item++) {
            bool correct = false;
            do {
                QString strItemTitle2 = QString::fromStdString(item->itemTitle2).trimmed();
                QString strItemImage2 = QString::fromStdString(item->itemImage2).trimmed();
                if (!bInjected && ((!strItemTitle2.isEmpty() && strItemImage2.isEmpty()) || (!strItemImage2.isEmpty() && strItemTitle2.isEmpty()))) {
                    break;
                }

                // 原始数据校验长度
                QString strItemTitle = QString::fromStdString(item->itemTitle).trimmed();
                if (strItemTitle.isEmpty())
                    break;

                // 原始数据校验长度
                QString strItemImage = QString::fromStdString(item->itemImage).trimmed();
                if (!bInjected && strItemImage.isEmpty())
                    break;

                // 内容长度校验
                std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
                std::wstring converted = myconv.from_bytes(item->itemTitle);
                if (converted.length() > MENU_ITEM_TITLE_LIMIT)
                    break;

                if (!bInjected && !strItemTitle2.isEmpty()) {
                    // 内容长度校验
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
                    std::wstring converted = myconv.from_bytes(item->itemTitle2);
                    if (converted.length() > MENU_ITEM_TITLE_LIMIT)
                        break;
                }

                correct = true;
            } while (false);

            if (!correct)
                return false;
        }
    }

    return true;
}
