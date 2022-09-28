﻿// Copyright (c) 2022 NetEase, Inc. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "nem_hosting_module_protocol/protocol/meeting_protocol.h"
NNEM_SDK_HOSTING_MODULE_PROTOCOL_BEGIN_DECLS

void StartRequest::OnPack(Json::Value& root) const {
    root["param_"]["displayName"] = param_.displayName;
    root["param_"]["meetingId"] = param_.meetingId;
    root["param_"]["sceneCode"] = param_.scene.code;
    root["param_"]["tag"] = param_.tag;
    root["param_"]["password"] = param_.password;
    root["param_"]["extraData"] = param_.extraData;

    Json::Value roleBinds;
    for (auto iter = param_.roleBinds.begin(); iter != param_.roleBinds.end(); iter++) {
        roleBinds[iter->first] = static_cast<int>(iter->second);
    }
    root["param_"]["roleBinds"] = roleBinds;

    for (auto it : param_.scene.roleTypes) {
        Json::Value valueItem;
        valueItem["roleType"] = it.roleType;
        valueItem["maxCount"] = it.maxCount;

        Json::Value userItems;
        for (auto user : it.userList) {
            Json::Value userItem;
            userItem["userId"] = user;
            userItems.append(userItem);
        }

        valueItem["userList"] = userItems;
        root["param_"]["roleTypes"].append(valueItem);
    }

    for (auto it : param_.controls) {
        Json::Value control;
        control["attendeeOff"] = it.attendeeOff;
        control["type"] = it.type;
        root["param_"]["controls"].append(control);
    }

    root["options_"]["noAudio"] = options_.noAudio;
    root["options_"]["noVideo"] = options_.noVideo;
    root["options_"]["noChat"] = options_.noChat;
    root["options_"]["noInvite"] = options_.noInvite;
    root["options_"]["noScreenShare"] = options_.noScreenShare;
    root["options_"]["noView"] = options_.noView;
    root["options_"]["noWhiteboard"] = options_.noWhiteboard;
    root["options_"]["noRename"] = options_.noRename;
    root["options_"]["noSip"] = options_.noSip;
    root["options_"]["noMuteAllVideo"] = options_.noMuteAllVideo;
    root["options_"]["noMuteAllAudio"] = options_.noMuteAllAudio;
    root["options_"]["showMemberTag"] = options_.showMemberTag;
    root["options_"]["showMeetingRemainingTip"] = options_.showMeetingRemainingTip;
    root["options_"]["audioAINSEnabled"] = options_.audioAINSEnabled;
    root["options_"]["noCloudRecord"] = options_.noCloudRecord;
    root["options_"]["defaultWindowMode"] = options_.defaultWindowMode;
    root["options_"]["meetingIdDisplayOption"] = options_.meetingIdDisplayOption;
    root["options_"]["joinTimeout"] = options_.joinTimeout;

    Json::Value chatroomConfig;
    chatroomConfig["enableFileMessage"] = options_.chatroomConfig.enableFileMessage;
    chatroomConfig["enableImageMessage"] = options_.chatroomConfig.enableImageMessage;
    root["options_"]["chatroomConfig"] = chatroomConfig;

    if (options_.full_more_menu_items_.empty() == false) {
        for (auto& item : options_.full_more_menu_items_) {
            Json::Value json_item;
            json_item["item_guid"] = item.itemGuid;
            json_item["item_id"] = item.itemId;
            json_item["item_title"] = item.itemTitle;
            json_item["item_image"] = item.itemImage;
            json_item["item_title2"] = item.itemTitle2;
            json_item["item_image2"] = item.itemImage2;
            json_item["item_visibility"] = item.itemVisibility;
            json_item["item_checkedIndex"] = item.itemCheckedIndex;
            root["options_"]["fullMoreMenuItem"].append(json_item);
        }
    }

    if (options_.full_toolbar_menu_items_.empty() == false) {
        for (auto& item : options_.full_toolbar_menu_items_) {
            Json::Value json_item;
            json_item["item_guid"] = item.itemGuid;
            json_item["item_id"] = item.itemId;
            json_item["item_title"] = item.itemTitle;
            json_item["item_image"] = item.itemImage;
            json_item["item_title2"] = item.itemTitle2;
            json_item["item_image2"] = item.itemImage2;
            json_item["item_visibility"] = item.itemVisibility;
            json_item["item_checkedIndex"] = item.itemCheckedIndex;
            root["options_"]["fullToolbarMenuItem"].append(json_item);
        }
    }
}

void StartRequest::OnParse(const Json::Value& root) {
    param_.displayName = root["param_"]["displayName"].asString();
    param_.meetingId = root["param_"]["meetingId"].asString();
    param_.tag = root["param_"]["tag"].asString();
    param_.password = root["param_"]["password"].asString();
    param_.extraData = root["param_"]["extraData"].asString();
    param_.scene.code = root["param_"]["sceneCode"].asString();
    param_.scene.roleTypes.clear();
    for (auto it : root["param_"]["roleTypes"]) {
        NEMeetingRoleConfiguration config;
        config.roleType = (NEMeetingRoleType)it["roleType"].asUInt();
        config.maxCount = it["maxCount"].asInt();

        for (auto user : it["userList"]) {
            config.userList.push_back(user["userId"].asString());
        }

        param_.scene.roleTypes.push_back(config);
    }

    for (auto it : root["param_"]["controls"]) {
        NEMeetingControl control;
        control.attendeeOff = static_cast<NEAttendeeOffType>(it["attendeeOff"].asInt());
        control.type = static_cast<NEControlType>(it["type"].asInt());
        param_.controls.push_back(control);
    }

    auto roleBindsObj = root["param_"]["roleBinds"];
    auto keys = roleBindsObj.getMemberNames();
    for (auto it : keys) {
        param_.roleBinds[it] = static_cast<NEMeetingRoleType>(roleBindsObj[it].asInt());
    }

    options_.noAudio = root["options_"]["noAudio"].asBool();
    options_.noVideo = root["options_"]["noVideo"].asBool();
    options_.noChat = root["options_"]["noChat"].asBool();
    options_.noInvite = root["options_"]["noInvite"].asBool();
    options_.noScreenShare = root["options_"]["noScreenShare"].asBool();
    options_.noView = root["options_"]["noView"].asBool();
    options_.noWhiteboard = root["options_"]["noWhiteboard"].asBool();
    options_.noRename = root["options_"]["noRename"].asBool();
    options_.noCloudRecord = root["options_"]["noCloudRecord"].asBool();
    options_.noSip = root["options_"]["noSip"].asBool();
    options_.noMuteAllVideo = root["options_"]["noMuteAllVideo"].asBool();
    options_.noMuteAllAudio = root["options_"]["noMuteAllAudio"].asBool();
    options_.showMemberTag = root["options_"]["showMemberTag"].asBool();
    options_.showMeetingRemainingTip = root["options_"]["showMeetingRemainingTip"].asBool();
    options_.audioAINSEnabled = root["options_"]["audioAINSEnabled"].asBool();
    options_.meetingIdDisplayOption = (NEShowMeetingIdOption)root["options_"]["meetingIdDisplayOption"].asInt();
    options_.defaultWindowMode = (NEMettingWindowMode)root["options_"]["defaultWindowMode"].asInt();
    options_.joinTimeout = root["options_"]["joinTimeout"].asInt();

    auto chatroomObj = root["options_"]["chatroomConfig"];
    options_.chatroomConfig.enableFileMessage = chatroomObj["enableFileMessage"].asBool();
    options_.chatroomConfig.enableImageMessage = chatroomObj["enableImageMessage"].asBool();

    if (root.isMember("options_") && root["options_"].isMember("fullMoreMenuItem")) {
        for (auto& item : root["options_"]["fullMoreMenuItem"]) {
            NEMeetingMenuItem menu_item;
            menu_item.itemGuid = item["item_guid"].asString();
            menu_item.itemId = item["item_id"].asInt();
            menu_item.itemTitle = item["item_title"].asString();
            menu_item.itemImage = item["item_image"].asString();
            menu_item.itemTitle2 = item["item_title2"].asString();
            menu_item.itemImage2 = item["item_image2"].asString();
            menu_item.itemVisibility = (NEMenuVisibility)item["item_visibility"].asInt();
            menu_item.itemCheckedIndex = item["item_checkedIndex"].asInt();
            options_.full_more_menu_items_.push_back(menu_item);
        }
    }

    if (root.isMember("options_") && root["options_"].isMember("fullToolbarMenuItem")) {
        for (auto& item : root["options_"]["fullToolbarMenuItem"]) {
            NEMeetingMenuItem menu_item;
            menu_item.itemGuid = item["item_guid"].asString();
            menu_item.itemId = item["item_id"].asInt();
            menu_item.itemTitle = item["item_title"].asString();
            menu_item.itemImage = item["item_image"].asString();
            menu_item.itemTitle2 = item["item_title2"].asString();
            menu_item.itemImage2 = item["item_image2"].asString();
            menu_item.itemVisibility = (NEMenuVisibility)item["item_visibility"].asInt();
            menu_item.itemCheckedIndex = item["item_checkedIndex"].asInt();
            options_.full_toolbar_menu_items_.push_back(menu_item);
        }
    }
}

///////////////////////////////
void JoinRequest::OnPack(Json::Value& root) const {
    root["param_"]["displayName"] = param_.displayName;
    root["param_"]["meetingId"] = param_.meetingId;
    root["param_"]["password"] = param_.password;
    root["param_"]["tag"] = param_.tag;
    root["options_"]["noAudio"] = options_.noAudio;
    root["options_"]["noVideo"] = options_.noVideo;
    root["options_"]["noChat"] = options_.noChat;
    root["options_"]["noInvite"] = options_.noInvite;
    root["options_"]["noScreenShare"] = options_.noScreenShare;
    root["options_"]["noView"] = options_.noView;
    root["options_"]["noWhiteboard"] = options_.noWhiteboard;
    root["options_"]["noRename"] = options_.noRename;
    root["options_"]["noSip"] = options_.noSip;
    root["options_"]["noMuteAllVideo"] = options_.noMuteAllVideo;
    root["options_"]["noMuteAllAudio"] = options_.noMuteAllAudio;
    root["options_"]["showMemberTag"] = options_.showMemberTag;
    root["options_"]["showMeetingRemainingTip"] = options_.showMeetingRemainingTip;
    root["options_"]["audioAINSEnabled"] = options_.audioAINSEnabled;
    root["options_"]["meetingIdDisplayOption"] = options_.meetingIdDisplayOption;
    root["options_"]["defaultWindowMode"] = options_.defaultWindowMode;
    root["options_"]["joinTimeout"] = options_.joinTimeout;

    Json::Value chatroomConfig;
    chatroomConfig["enableFileMessage"] = options_.chatroomConfig.enableFileMessage;
    chatroomConfig["enableImageMessage"] = options_.chatroomConfig.enableImageMessage;
    root["options_"]["chatroomConfig"] = chatroomConfig;

    if (options_.full_more_menu_items_.empty() == false) {
        for (auto& item : options_.full_more_menu_items_) {
            Json::Value json_item;
            json_item["item_guid"] = item.itemGuid;
            json_item["item_id"] = item.itemId;
            json_item["item_title"] = item.itemTitle;
            json_item["item_image"] = item.itemImage;
            json_item["item_title2"] = item.itemTitle2;
            json_item["item_image2"] = item.itemImage2;
            json_item["item_visibility"] = item.itemVisibility;
            json_item["item_checkedIndex"] = item.itemCheckedIndex;
            root["options_"]["fullMoreMenuItem"].append(json_item);
        }
    }

    if (options_.full_toolbar_menu_items_.empty() == false) {
        for (auto& item : options_.full_toolbar_menu_items_) {
            Json::Value json_item;
            json_item["item_guid"] = item.itemGuid;
            json_item["item_id"] = item.itemId;
            json_item["item_title"] = item.itemTitle;
            json_item["item_image"] = item.itemImage;
            json_item["item_title2"] = item.itemTitle2;
            json_item["item_image2"] = item.itemImage2;
            json_item["item_visibility"] = item.itemVisibility;
            json_item["item_checkedIndex"] = item.itemCheckedIndex;
            root["options_"]["fullToolbarMenuItem"].append(json_item);
        }
    }
}

void JoinRequest::OnParse(const Json::Value& root) {
    param_.displayName = root["param_"]["displayName"].asString();
    param_.meetingId = root["param_"]["meetingId"].asString();
    param_.password = root["param_"]["password"].asString();
    param_.tag = root["param_"]["tag"].asString();
    options_.noAudio = root["options_"]["noAudio"].asBool();
    options_.noVideo = root["options_"]["noVideo"].asBool();
    options_.noChat = root["options_"]["noChat"].asBool();
    options_.noInvite = root["options_"]["noInvite"].asBool();
    options_.noScreenShare = root["options_"]["noScreenShare"].asBool();
    options_.noView = root["options_"]["noView"].asBool();
    options_.noWhiteboard = root["options_"]["noWhiteboard"].asBool();
    options_.noRename = root["options_"]["noRename"].asBool();
    options_.noSip = root["options_"]["noSip"].asBool();
    options_.noMuteAllVideo = root["options_"]["noMuteAllVideo"].asBool();
    options_.noMuteAllAudio = root["options_"]["noMuteAllAudio"].asBool();
    options_.showMemberTag = root["options_"]["showMemberTag"].asBool();
    options_.showMeetingRemainingTip = root["options_"]["showMeetingRemainingTip"].asBool();
    options_.audioAINSEnabled = root["options_"]["audioAINSEnabled"].asBool();
    options_.meetingIdDisplayOption = (NEShowMeetingIdOption)root["options_"]["meetingIdDisplayOption"].asInt();
    options_.defaultWindowMode = (NEMettingWindowMode)root["options_"]["defaultWindowMode"].asInt();
    options_.joinTimeout = root["options_"]["joinTimeout"].asInt();

    auto chatroomObj = root["options_"]["chatroomConfig"];
    options_.chatroomConfig.enableFileMessage = chatroomObj["enableFileMessage"].asBool();
    options_.chatroomConfig.enableImageMessage = chatroomObj["enableImageMessage"].asBool();

    if (root.isMember("options_") && root["options_"].isMember("fullMoreMenuItem")) {
        for (auto& item : root["options_"]["fullMoreMenuItem"]) {
            NEMeetingMenuItem menu_item;
            menu_item.itemGuid = item["item_guid"].asString();
            menu_item.itemId = item["item_id"].asInt();
            menu_item.itemTitle = item["item_title"].asString();
            menu_item.itemImage = item["item_image"].asString();
            menu_item.itemTitle2 = item["item_title2"].asString();
            menu_item.itemImage2 = item["item_image2"].asString();
            menu_item.itemVisibility = (NEMenuVisibility)item["item_visibility"].asInt();
            menu_item.itemCheckedIndex = item["item_checkedIndex"].asInt();
            options_.full_more_menu_items_.push_back(menu_item);
        }
    }

    if (root.isMember("options_") && root["options_"].isMember("fullToolbarMenuItem")) {
        for (auto& item : root["options_"]["fullToolbarMenuItem"]) {
            NEMeetingMenuItem menu_item;
            menu_item.itemGuid = item["item_guid"].asString();
            menu_item.itemId = item["item_id"].asInt();
            menu_item.itemTitle = item["item_title"].asString();
            menu_item.itemImage = item["item_image"].asString();
            menu_item.itemTitle2 = item["item_title2"].asString();
            menu_item.itemImage2 = item["item_image2"].asString();
            menu_item.itemVisibility = (NEMenuVisibility)item["item_visibility"].asInt();
            menu_item.itemCheckedIndex = item["item_checkedIndex"].asInt();
            options_.full_toolbar_menu_items_.push_back(menu_item);
        }
    }
}
///////////////////////////////
void LeaveMeetingRequest::OnPack(Json::Value& root) const {
    root["finish"] = finish_;
}

void LeaveMeetingRequest::OnParse(const Json::Value& root) {
    finish_ = root["finish"].asBool();
}

void MeetingStatusChangePack::OnPack(Json::Value& root) const {
    root["status_"] = status_;
    root["code_"] = code_;
}
void MeetingStatusChangePack::OnParse(const Json::Value& root) {
    status_ = root["status_"].asInt();
    code_ = root["code_"].asInt();
}

void GetMeetingInfoRequest::OnPack(Json::Value& root) const {}
void GetMeetingInfoRequest::OnParse(const Json::Value& root) {}

void GetMeetingInfoResponse::OnOtherPack(Json::Value& root) const {
    root["isHost"] = meeting_info_.isHost;
    root["isLocked"] = meeting_info_.isLocked;
    root["meetingId"] = meeting_info_.meetingId;
    root["duration"] = meeting_info_.duration;
    root["shortMeetingId"] = meeting_info_.shortMeetingId;
    root["sipId"] = meeting_info_.sipId;
    root["meetingUniqueId"] = meeting_info_.meetingUniqueId;
    root["subject"] = meeting_info_.subject;
    root["password"] = meeting_info_.password;
    root["hostUserId"] = meeting_info_.hostUserId;
    root["scheduleStartTime"] = meeting_info_.scheduleStartTime;
    root["scheduleEndTime"] = meeting_info_.scheduleEndTime;
    root["startTime"] = meeting_info_.startTime;
    root["extraData"] = meeting_info_.extraData;
    root["meetingCreatorId"] = meeting_info_.meetingCreatorId;
    root["meetingCreatorName"] = meeting_info_.meetingCreatorName;

    for (auto& item : meeting_info_.userList) {
        Json::Value json_item;
        json_item["userId"] = item.userId;
        json_item["userName"] = item.userName;
        json_item["tag"] = item.tag;
        root["userList"].append(json_item);
    }
}

void GetMeetingInfoResponse::OnOtherParse(const Json::Value& root) {
    meeting_info_.isHost = root["isHost"].asBool();
    meeting_info_.isLocked = root["isLocked"].asBool();
    meeting_info_.meetingId = root["meetingId"].asString();
    meeting_info_.duration = root["duration"].asInt64();
    meeting_info_.shortMeetingId = root["shortMeetingId"].asString();
    meeting_info_.sipId = root["sipId"].asString();
    meeting_info_.meetingUniqueId = root["meetingUniqueId"].asInt64();
    meeting_info_.subject = root["subject"].asString();
    meeting_info_.password = root["password"].asString();
    meeting_info_.hostUserId = root["hostUserId"].asString();
    meeting_info_.scheduleStartTime = root["scheduleStartTime"].asInt64();
    meeting_info_.scheduleEndTime = root["scheduleEndTime"].asInt64();
    meeting_info_.startTime = root["startTime"].asInt64();
    meeting_info_.extraData = root["extraData"].asString();
    meeting_info_.meetingCreatorId = root["meetingCreatorId"].asString();
    meeting_info_.meetingCreatorName = root["meetingCreatorName"].asString();

    for (auto& item : root["userList"]) {
        NEInMeetingUserInfo user;
        user.userId = item["userId"].asString();
        user.userName = item["userName"].asString();
        user.tag = item["tag"].asString();
        meeting_info_.userList.push_back(user);
    }
}

void MeetingMenuItemClickedPack::OnPack(Json::Value& root) const {
    root["item_id"] = menu_item_.itemId;
    root["item_guid"] = menu_item_.itemGuid;
    root["item_title"] = menu_item_.itemTitle;
    root["item_image"] = menu_item_.itemImage;
    root["item_title2"] = menu_item_.itemTitle2;
    root["item_image2"] = menu_item_.itemImage2;
    root["item_visibility"] = menu_item_.itemVisibility;
    root["item_checkedIndex"] = menu_item_.itemCheckedIndex;
}

void MeetingMenuItemClickedPack::OnParse(const Json::Value& root) {
    menu_item_.itemId = root["item_id"].asInt();
    menu_item_.itemGuid = root["item_guid"].asString();
    menu_item_.itemTitle = root["item_title"].asString();
    menu_item_.itemImage = root["item_image"].asString();
    menu_item_.itemTitle2 = root["item_title2"].asString();
    menu_item_.itemImage2 = root["item_image2"].asString();
    menu_item_.itemVisibility = (NEMenuVisibility)root["item_visibility"].asInt();
    menu_item_.itemCheckedIndex = root["item_checkedIndex"].asInt();
}

void GetPresetMenuItemsResponse::OnOtherPack(Json::Value& root) const {
    if (menu_items_.empty() == false) {
        for (auto& item : menu_items_) {
            Json::Value json_item;
            json_item["item_guid"] = item.itemGuid;
            json_item["item_id"] = item.itemId;
            json_item["item_title"] = item.itemTitle;
            json_item["item_image"] = item.itemImage;
            json_item["item_title2"] = item.itemTitle2;
            json_item["item_image2"] = item.itemImage2;
            json_item["item_visibility"] = item.itemVisibility;
            json_item["item_checkedIndex"] = item.itemCheckedIndex;
            root["MenuItems"].append(json_item);
        }
    }
}
void GetPresetMenuItemsResponse::OnOtherParse(const Json::Value& root) {
    for (auto& item : root["MenuItems"]) {
        NEMeetingMenuItem menuItem;
        menuItem.itemGuid = item["item_guid"].asString();
        menuItem.itemId = item["item_id"].asInt();
        menuItem.itemTitle = item["item_title"].asString();
        menuItem.itemImage = item["item_image"].asString();
        menuItem.itemTitle2 = item["item_title2"].asString();
        menuItem.itemImage2 = item["item_image2"].asString();
        menuItem.itemVisibility = (NEMenuVisibility)item["item_visibility"].asInt();
        menuItem.itemCheckedIndex = item["item_checkedIndex"].asInt();
        menu_items_.push_back(menuItem);
    }
}

void GetPresetMenuItemsRequest::OnPack(Json::Value& root) const {
    if (menu_items_id_.empty() == false) {
        for (auto& item : menu_items_id_) {
            root.append(item);
        }
    }
}
void GetPresetMenuItemsRequest::OnParse(const Json::Value& root) {
    if (root.isArray()) {
        for (auto& it : root) {
            menu_items_id_.push_back(it.asInt());
        }
    }
}

void PreMeetingRequest::OnPack(Json::Value& root) const {
    root["param_"]["meetingId"] = param_.meetingId;
    root["param_"]["starttime"] = param_.startTime;
    root["param_"]["endtime"] = param_.endTime;
    root["param_"]["meetingSubject"] = param_.subject;
    root["param_"]["password"] = param_.password;
    root["param_"]["AttendeeAudioOff"] = param_.setting.attendeeAudioOff;
    root["param_"]["cloudRecordOn "] = param_.setting.cloudRecordOn;
    root["param_"]["meetingUniqueId"] = param_.meetingUniqueId;
    root["param_"]["meetingStatus"] = param_.status;
    root["param_"]["createTime"] = param_.createTime;
    root["param_"]["updateTime"] = param_.updateTime;
    root["param_"]["enableLive"] = param_.enableLive;
    root["param_"]["noSip"] = param_.noSip;
    root["param_"]["liveWebAccessControlLevel"] = param_.liveWebAccessControlLevel;
    root["param_"]["liveUrl"] = param_.liveUrl;
    root["param_"]["sceneCode"] = param_.setting.scene.code;
    root["param_"]["extraData"] = param_.extraData;

    Json::Value roleBinds;
    for (auto iter = param_.roleBinds.begin(); iter != param_.roleBinds.end(); iter++) {
        roleBinds[iter->first] = static_cast<int>(iter->second);
    }
    root["param_"]["roleBinds"] = roleBinds;

    for (auto it : param_.setting.scene.roleTypes) {
        Json::Value valueItem;
        valueItem["roleType"] = it.roleType;
        valueItem["maxCount"] = it.maxCount;

        Json::Value userItems;
        for (auto user : it.userList) {
            Json::Value userItem;
            userItem["userId"] = user;
            userItems.append(userItem);
        }

        valueItem["userList"] = userItems;
        root["param_"]["roleTypes"].append(valueItem);
    }

    for (auto it : param_.setting.controls) {
        Json::Value control;
        control["attendeeOff"] = it.attendeeOff;
        control["type"] = it.type;
        root["param_"]["controls"].append(control);
    }
}
void PreMeetingRequest::OnParse(const Json::Value& root) {
    param_.meetingId = root["param_"]["meetingId"].asString();
    param_.startTime = root["param_"]["starttime"].asInt64();
    param_.endTime = root["param_"]["endtime"].asInt64();
    param_.subject = root["param_"]["meetingSubject"].asString();
    param_.password = root["param_"]["password"].asString();
    param_.setting.attendeeAudioOff = root["param_"]["AttendeeAudioOff"].asBool();
    param_.setting.cloudRecordOn = root["param_"]["cloudRecordOn "].asBool();
    param_.meetingUniqueId = root["param_"]["meetingUniqueId"].asInt64();
    param_.status = (NEMeetingItemStatus)(root["param_"]["meetingStatus"].asInt());
    param_.createTime = root["param_"]["createTime"].asInt64();
    param_.updateTime = root["param_"]["updateTime"].asInt64();
    param_.enableLive = root["param_"]["enableLive"].asBool();
    param_.noSip = root["param_"]["noSip"].asBool();
    param_.liveWebAccessControlLevel = (NEMettingLiveAuthLevel)(root["param_"]["liveWebAccessControlLevel"].asInt());
    param_.liveUrl = root["param_"]["liveUrl"].asString();
    param_.extraData = root["param_"]["extraData"].asString();

    param_.setting.scene.roleTypes.clear();
    param_.setting.scene.code = root["param_"]["sceneCode"].asString();
    for (auto it : root["param_"]["roleTypes"]) {
        NEMeetingRoleConfiguration config;
        config.roleType = (NEMeetingRoleType)it["roleType"].asUInt();
        config.maxCount = it["maxCount"].asUInt();

        for (auto user : it["userList"]) {
            config.userList.push_back(user["userId"].asString());
        }

        param_.setting.scene.roleTypes.push_back(config);
    }

    for (auto it : root["param_"]["controls"]) {
        NEMeetingControl control;
        control.attendeeOff = static_cast<NEAttendeeOffType>(it["attendeeOff"].asInt());
        control.type = static_cast<NEControlType>(it["type"].asInt());
        param_.setting.controls.push_back(control);
    }

    auto roleBindsObj = root["param_"]["roleBinds"];
    auto keys = roleBindsObj.getMemberNames();
    for (auto it : keys) {
        param_.roleBinds[it] = static_cast<NEMeetingRoleType>(roleBindsObj[it].asInt());
    }
}

void PreMeetingResponse::OnOtherPack(Json::Value& root) const {
    root["param_"]["meetingId"] = param_.meetingId;
    root["param_"]["starttime"] = param_.startTime;
    root["param_"]["endtime"] = param_.endTime;
    root["param_"]["meetingSubject"] = param_.subject;
    root["param_"]["password"] = param_.password;
    root["param_"]["AttendeeAudioOff"] = param_.setting.attendeeAudioOff;
    root["param_"]["cloudRecordOn "] = param_.setting.cloudRecordOn;
    root["param_"]["meetingUniqueId"] = param_.meetingUniqueId;
    root["param_"]["meetingStatus"] = param_.status;
    root["param_"]["createTime"] = param_.createTime;
    root["param_"]["updateTime"] = param_.updateTime;
    root["param_"]["enableLive"] = param_.enableLive;
    root["param_"]["noSip"] = param_.noSip;
    root["param_"]["liveWebAccessControlLevel"] = param_.liveWebAccessControlLevel;
    root["param_"]["liveUrl"] = param_.liveUrl;
    root["param_"]["sceneCode"] = param_.setting.scene.code;
    root["param_"]["extraData"] = param_.extraData;

    Json::Value roleBinds;
    for (auto iter = param_.roleBinds.begin(); iter != param_.roleBinds.end(); iter++) {
        roleBinds[iter->first] = static_cast<int>(iter->second);
    }
    root["param_"]["roleBinds"] = roleBinds;

    for (auto it : param_.setting.scene.roleTypes) {
        Json::Value valueItem;
        valueItem["roleType"] = it.roleType;
        valueItem["maxCount"] = it.maxCount;

        Json::Value userItems;
        for (auto user : it.userList) {
            Json::Value userItem;
            userItem["userId"] = user;
            userItems.append(userItem);
        }

        valueItem["userList"] = userItems;

        root["param_"]["roleTypes"].append(valueItem);
    }

    for (auto it : param_.setting.controls) {
        Json::Value control;
        control["attendeeOff"] = it.attendeeOff;
        control["type"] = it.type;
        root["param_"]["controls"].append(control);
    }
}

void PreMeetingResponse::OnOtherParse(const Json::Value& root) {
    param_.meetingId = root["param_"]["meetingId"].asString();
    param_.startTime = root["param_"]["starttime"].asInt64();
    param_.endTime = root["param_"]["endtime"].asInt64();
    param_.subject = root["param_"]["meetingSubject"].asString();
    param_.password = root["param_"]["password"].asString();
    param_.setting.attendeeAudioOff = root["param_"]["AttendeeAudioOff"].asBool();
    param_.setting.cloudRecordOn = root["param_"]["cloudRecordOn "].asBool();
    param_.meetingUniqueId = root["param_"]["meetingUniqueId"].asInt64();
    param_.status = (NEMeetingItemStatus)(root["param_"]["meetingStatus"].asInt());
    param_.createTime = root["param_"]["createTime"].asInt64();
    param_.updateTime = root["param_"]["updateTime"].asInt64();
    param_.enableLive = root["param_"]["enableLive"].asBool();
    param_.noSip = root["param_"]["noSip"].asBool();
    param_.liveUrl = root["param_"]["liveUrl"].asString();
    param_.liveWebAccessControlLevel = (NEMettingLiveAuthLevel)(root["param_"]["liveWebAccessControlLevel"].asInt());
    param_.extraData = root["param_"]["extraData"].asString();

    param_.setting.scene.roleTypes.clear();
    param_.setting.scene.code = root["param_"]["sceneCode"].asString();
    for (auto it : root["param_"]["roleTypes"]) {
        NEMeetingRoleConfiguration config;
        config.roleType = (NEMeetingRoleType)it["roleType"].asUInt();
        config.maxCount = it["maxCount"].asInt();

        for (auto user : it["userList"]) {
            config.userList.push_back(user["userId"].asString());
        }

        param_.setting.scene.roleTypes.push_back(config);
    }

    for (auto it : root["param_"]["controls"]) {
        NEMeetingControl control;
        control.attendeeOff = static_cast<NEAttendeeOffType>(it["attendeeOff"].asInt());
        control.type = static_cast<NEControlType>(it["type"].asInt());
        param_.setting.controls.push_back(control);
    }

    auto roleBindsObj = root["param_"]["roleBinds"];
    auto keys = roleBindsObj.getMemberNames();
    for (auto it : keys) {
        param_.roleBinds[it] = static_cast<NEMeetingRoleType>(roleBindsObj[it].asInt());
    }
}

void GetPreMeetingListResponse::OnOtherPack(Json::Value& root) const {
    if (meeting_items.empty() == false) {
        for (auto& item : meeting_items) {
            Json::Value json_item;
            json_item["meetingId"] = item.meetingId;
            json_item["starttime"] = item.startTime;
            json_item["endtime"] = item.endTime;
            json_item["meetingSubject"] = item.subject;
            json_item["password"] = item.password;
            json_item["AttendeeAudioOff"] = item.setting.attendeeAudioOff;
            json_item["cloudRecordOn "] = item.setting.cloudRecordOn;
            json_item["meetingUniqueId"] = item.meetingUniqueId;
            json_item["meetingStatus"] = item.status;
            json_item["createTime"] = item.createTime;
            json_item["updateTime"] = item.updateTime;
            json_item["enableLive"] = item.enableLive;
            json_item["noSip"] = item.noSip;
            json_item["liveWebAccessControlLevel"] = item.liveWebAccessControlLevel;
            json_item["liveUrl"] = item.liveUrl;
            json_item["extraData"] = item.extraData;
            json_item["scene"]["sceneCode"] = item.setting.scene.code;
            for (auto it : item.setting.scene.roleTypes) {
                Json::Value valueItem;
                valueItem["roleType"] = it.roleType;
                valueItem["maxCount"] = it.maxCount;

                Json::Value userItems;
                for (auto user : it.userList) {
                    Json::Value userItem;
                    userItem["userId"] = user;
                    userItems.append(userItem);
                }

                valueItem["userList"] = userItems;
                json_item["scene"]["roleTypes"].append(valueItem);
            }
            for (auto it : item.setting.controls) {
                Json::Value control;
                control["attendeeOff"] = it.attendeeOff;
                control["type"] = it.type;
                json_item["param_"]["controls"].append(control);
            }

            Json::Value roleBinds;
            for (auto iter = item.roleBinds.begin(); iter != item.roleBinds.end(); iter++) {
                roleBinds[iter->first] = static_cast<int>(iter->second);
            }
            json_item["param_"]["roleBinds"] = roleBinds;

            root["MeetingItems"].append(json_item);
        }
    }
}

void GetPreMeetingListResponse::OnOtherParse(const Json::Value& root) {
    for (auto& item : root["MeetingItems"]) {
        NEMeetingItem meetingItem;
        meetingItem.meetingId = item["meetingId"].asString();
        meetingItem.startTime = item["starttime"].asInt64();
        meetingItem.endTime = item["endtime"].asInt64();
        meetingItem.subject = item["meetingSubject"].asString();
        meetingItem.password = item["password"].asString();
        meetingItem.setting.attendeeAudioOff = item["AttendeeAudioOff"].asBool();
        meetingItem.setting.cloudRecordOn = item["cloudRecordOn "].asBool();
        meetingItem.meetingUniqueId = item["meetingUniqueId"].asInt64();
        meetingItem.status = (NEMeetingItemStatus)(item["meetingStatus"].asInt());
        meetingItem.createTime = item["createTime"].asInt64();
        meetingItem.updateTime = item["updateTime"].asInt64();
        meetingItem.enableLive = item["enableLive"].asBool();
        meetingItem.noSip = item["noSip"].asBool();
        meetingItem.liveWebAccessControlLevel = (NEMettingLiveAuthLevel)(item["liveWebAccessControlLevel"].asInt());
        meetingItem.liveUrl = item["liveUrl"].asString();
        meetingItem.extraData = item["extraData"].asString();
        meetingItem.setting.scene.roleTypes.clear();
        meetingItem.setting.scene.code = item["scene"]["sceneCode"].asString();
        for (auto it : item["scene"]["roleTypes"]) {
            NEMeetingRoleConfiguration config;
            config.roleType = (NEMeetingRoleType)it["roleType"].asUInt();
            config.maxCount = it["maxCount"].asInt();

            for (auto user : it["userList"]) {
                config.userList.push_back(user["userId"].asString());
            }

            meetingItem.setting.scene.roleTypes.push_back(config);
        }
        for (auto it : item["param_"]["controls"]) {
            NEMeetingControl control;
            control.attendeeOff = static_cast<NEAttendeeOffType>(it["attendeeOff"].asInt());
            control.type = static_cast<NEControlType>(it["type"].asInt());
            meetingItem.setting.controls.push_back(control);
        }
        auto roleBindsObj = item["param_"]["roleBinds"];
        auto keys = roleBindsObj.getMemberNames();
        for (auto it : keys) {
            meetingItem.roleBinds[it] = static_cast<NEMeetingRoleType>(roleBindsObj[it].asInt());
        }
        meeting_items.push_back(meetingItem);
    }
}

void GetPreMeetingListRequest::OnOtherPack(Json::Value& root) const {
    if (params_.empty() == false) {
        for (auto& item : params_) {
            root["MeetingStatus"].append(item);
        }
    }
}

void GetPreMeetingListRequest::OnOtherParse(const Json::Value& root) {
    for (auto& item : root["MeetingStatus"]) {
        auto status = (NEMeetingItemStatus)(item.asInt());
        params_.push_back(status);
    }
}

void PreMeetingStatusChangePack::OnPack(Json::Value& root) const {
    root["status_"] = status_;
    root["meetingUniqueId_"] = meetingUniqueId_;
}
void PreMeetingStatusChangePack::OnParse(const Json::Value& root) {
    status_ = root["status_"].asInt();
    meetingUniqueId_ = root["meetingUniqueId_"].asInt64();
}

void SubscribeAudioStreamsRequest::OnPack(Json::Value& root) const {
    root["subscribe"] = subscribe_;
    for (auto& it : accountIdList_) {
        root["accountIdList"].append(it);
    }
}

void SubscribeAudioStreamsRequest::OnParse(const Json::Value& root) {
    subscribe_ = root["subscribe"].asBool();
    for (auto& it : root["accountIdList"]) {
        accountIdList_.push_back(it.asString());
    }
}

NNEM_SDK_HOSTING_MODULE_PROTOCOL_END_DECLS
