﻿// Copyright (c) 2022 NetEase, Inc. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "auth_manager.h"
#include <QDirIterator>
#include "base/http_request.h"
#include "base/nem_auth_requests.h"
#include "feedback_manager.h"

AuthManager::AuthManager(QObject* parent)
    : QObject(parent)
    , m_httpManager(new HttpManager(parent)) {
    m_phoneNumber = ConfigManager::getInstance()->getValue("localPhoneNumber", "").toString();
}

AuthManager::~AuthManager() {}

void AuthManager::getAuthCode(const QString& phonePrefix, const QString& phoneNumber, int scene) {
    nem_auth::VerifyCode request(phoneNumber, (nem_auth::VerifyScene)scene);
    m_httpManager->getRequest(request, [=](int code, const QJsonObject& response) {
        if (0 == code) {
            emit gotAuthCode(phoneNumber);
        } else {
            emit error(code, response);
        }
    });
}

void AuthManager::verifyAuthCode(const QString& phonePrefix, const QString& phoneNumber, const QString& code, int scene) {
    nem_auth::CheckVerifyCode request(phoneNumber, code.toUInt(), (nem_auth::VerifyScene)scene);
    m_httpManager->postRequest(request, [=](int code, const QJsonObject& response) {
        if (0 == code) {
            setAuthCodeInternal(response["authCodeInternal"].toString());
            emit verifiedAuthCode(response["authCodeInternal"].toString());
        } else {
            emit error(code, response);
        }
    });
}

void AuthManager::registerAccount(const QString& phonePrefix, const QString& phoneNumber, const QString& nickname, const QString& password) {
    AppRegisterRequest request(phonePrefix, phoneNumber, authCodeInternal(), nickname, password);
    m_httpManager->postRequest(request, [this, nickname, phonePrefix, phoneNumber](int code, const QJsonObject& response) {
        if (code == 200) {
            setAppUserNick(nickname);
            setPhonePrefix(phonePrefix);
            setPhoneNumber(phoneNumber);
            setAppUserId(response["userId"].toString());
            setAppUserToken(response["meetingToken"].toString());
            setAPaasAccountId(response["accountId"].toString());
            setAPaasAccountToken(response["accountToken"].toString());
            setAPaasAppKey(response["appKey"].toString());

            ConfigManager::getInstance()->setValue("localUserId", response["userId"].toString());
            ConfigManager::getInstance()->setValue("localUserToken", response["meetingToken"].toString());

            emit registeredAccount();
        } else {
            emit error(code, response);
        }
    });
}

void AuthManager::registerNEAccount(const QString& phoneNumber, const QString& verifyCode, const QString& nickname, const QString& password) {
    auto defaultKey = ConfigManager::getInstance()->getValue("localAnonAppKey", LOCAL_DEFAULT_APPKEY);
    nem_auth::RegisterAccount request(phoneNumber, verifyCode, password, nickname, defaultKey.toString());
    m_httpManager->postRequest(request, std::bind(&AuthManager::onLoginCallback, this, std::placeholders::_1, std::placeholders::_2, true));
}

void AuthManager::loginToHttp(int loginType,
                              const QString& phonePrefix,
                              const QString& phoneNumber,
                              const QString& authValue,
                              const QString& cacheUserId) {
    AppLoginRequest request(static_cast<LoginType>(loginType), phonePrefix, phoneNumber, authValue, cacheUserId);
    m_httpManager->postRequest(request, [this, phonePrefix, phoneNumber, loginType](int code, const QJsonObject& response) {
        if (code == 200) {
            setAppUserNick(response["nickName"].toString());
            setPhonePrefix(response["countryCode"].toString());
            setPhoneNumber(response["mobilePhone"].toString());
            setAppUserOpenId(response["userOpenId"].toString());
            setAppUserId(response["userId"].toString());
            setAppUserToken(response["meetingToken"].toString());
            setAPaasAccountId(response["accountId"].toString());
            setAPaasAccountToken(response["accountToken"].toString());
            setAPaasAppKey(response["appKey"].toString());

            ConfigManager::getInstance()->setValue("localUserId", response["userId"].toString());
            ConfigManager::getInstance()->setValue("localUserToken", response["meetingToken"].toString());

            emit loggedIn(response["userId"].toString());
        } else {
            if (loginType == kLoginTypeByToken) {
                ConfigManager::getInstance()->setValue("localUserId", "");
                ConfigManager::getInstance()->setValue("localUserToken", "");
            }
            emit error(code, response);
        }
    });
}

void AuthManager::loginByPassword(const QString& username, const QString& password) {
    nem_auth::LoginWithPassword request(username, password);
    m_httpManager->postRequest(request, std::bind(&AuthManager::onLoginCallback, this, std::placeholders::_1, std::placeholders::_2, false));
}

void AuthManager::loginByVerifyCode(const QString& phoneNumber, const QString& verifyCode) {
    nem_auth::LoginWithVerifyCode request(phoneNumber, verifyCode);
    setPhoneNumber(phoneNumber);
    m_httpManager->postRequest(request, std::bind(&AuthManager::onLoginCallback, this, std::placeholders::_1, std::placeholders::_2, false));
}

void AuthManager::getAccountApps(const QString& appKey, const QString& accountId, const QString& accountToken) {
    return;
    nem_auth::GetApps request(appKey, accountId, accountToken);
    m_httpManager->postRequest(request, [=](int code, const QJsonObject& response) {
        if (code == 200) {
            if (response.contains("apps")) {
                for (auto app : response["apps"].toArray()) {
                    m_accountApps.push_back(app);
                }
                Q_EMIT gotAccountApps(m_accountApps);
            }
        } else {
            Q_EMIT error(code, response);
        }
    });
}

void AuthManager::getAccountAppInfo(const QString& appKey, const QString& accountId, const QString& accountToken) {
    return;
    nem_auth::GetAppInfo request(appKey, accountId, accountToken);
    m_httpManager->postRequest(request, [=](int code, const QJsonObject& response) {
        if (code == 200) {
            if (response.contains("appName"))
                setCurDisplayCompany(response["appName"].toString());
            if (response.contains("edition")) {
                auto edition = response["edition"].toObject();
                if (edition.contains("name"))
                    setCurDisplayVersion(edition["name"].toString());
                if (edition.contains("extra"))
                    setExtraInfo(edition["extra"].toString());
                if (edition.contains("functionList")) {
                    for (auto function : edition["functionList"].toArray()) {
                        auto object = function.toObject();
                        if (object["code"].toString() == "maxRoomDuration") {
                            YXLOG(Info) << "Set current max room duration: " << object["description"].toString().toStdString() << YXLOGEnd;
                            setMaxDuration(object["description"].toString());
                        }
                        if (object["code"].toString() == "maxRoomMemberCount") {
                            YXLOG(Info) << "Set current max member count: " << object["description"].toString().toStdString() << YXLOGEnd;
                            setMaxMemberCount(object["description"].toString());
                        }
                    }
                }
            }
        } else {
            Q_EMIT error(code, response);
        }
    });
}

void AuthManager::switchApp(const QString& appKey, const QString& accountId, const QString& accountToken) {
    nem_auth::SwitchApp request(appKey, accountId, accountToken);
    m_httpManager->postRequest(request, [=](int code, const QJsonObject& response) {
        if (code == 200) {
            Q_EMIT switchedApp(response);
        } else {
            Q_EMIT error(code, response);
        }
    });
}

void AuthManager::logout(bool needsLogout /* = true*/, bool cleanup /* = false*/) {
    YXLOG(Info) << "logout, needsLogout: " << needsLogout << ", cleanup: " << cleanup << YXLOGEnd;
    ConfigManager::getInstance()->setValue("localUserId", "");
    ConfigManager::getInstance()->setValue("localUserToken", "");
    ConfigManager::getInstance()->setValue("localPaasAppKey", "");
    ConfigManager::getInstance()->setValue("localPaasAccountId", "");
    ConfigManager::getInstance()->setValue("localPaasAccountToken", "");
    setCurDisplayCompany("");
    setCurDisplayVersion("");
    setMaxDuration("");
    setMaxMemberCount("");
    setAuthCodeInternal("");
    setPhonePrefix("");
    setPhoneNumber("");
    setAppUserId("");
    setAppUserOpenId("");
    setAppUserToken("");
    setAppUserNick("");
    setAPaasAccountId("");
    setAPaasAccountToken("");
    setAPaasAppKey("");
    setPersonalShortMeetingId("");
    setPersonalMeetingId("");
    m_accountApps = QJsonArray();
    emit loggedOut(needsLogout, cleanup);
}

void AuthManager::verifyPassword(const QString& oldPassword, const QString& userId, const QString& token) {
    AppVerifyPasswordRequest request(oldPassword, userId, token);
    m_httpManager->postRequest(request, [this](int code, const QJsonObject& response) {
        if (code == 200) {
            setAuthCodeInternal(response["authCodeInternal"].toString());
            emit verifiedPassword();
        } else {
            emit error(code, response);
        }
    });
}

void AuthManager::resetPassword(const QString& appKey, const QString& accountId, const QString& accountToken, const QString& newPassword) {
#if 0
    AppResetPasswordRequest request(phonePrefix, phoneNumber, newPassword, authCodeInternal());
    m_httpManager->postRequest(request, [this](int code, const QJsonObject& response) {
        if (code == 200) {
            emit resetPasswordSig();
        } else {
            emit error(code, response);
        }
    });
#else
    nem_auth::ChangePassword request(appKey, accountId, accountToken, newPassword);
    m_httpManager->postRequest(request, [=](int code, const QJsonObject& response) {
        if (code == 200) {
            Q_EMIT resetPasswordSig();
        } else {
            Q_EMIT error(code, response);
        }
    });
#endif
}

void AuthManager::resetPasswordByVerifyCode(const QString& phoneNumber, const QString& verifyCode, const QString& newPassword) {
    nem_auth::ResetPassword request(phoneNumber, verifyCode, newPassword);
    m_httpManager->postRequest(request, [=](int code, const QJsonObject& response) {
        if (code == 200) {
            Q_EMIT resetPasswordSig();
        } else {
            Q_EMIT error(code, response);
        }
    });
}

void AuthManager::updateProfile(const QString& nickname, const QString& appKey, const QString& userId, const QString& token) {
    nem_auth::ModifyNickname request(appKey, userId, token, nickname);
    m_httpManager->postRequest(request, [=](int code, const QJsonObject& response) {
        if (0 == code) {
            setAppUserNick(nickname);
            emit updatedProfile();
        } else {
            Q_EMIT error(code, response);
        }
    });
}

void AuthManager::resetAuthInfo() {
    setAppUserId("");
    setAPaasAppKey("");
    setAppUserNick("");
    setPhoneNumber("");
    setPhonePrefix("");
    setAppUserToken("");
    setAppUserOpenId("");
    setAPaasAccountId("");
    setAuthCodeInternal("");
    setAPaasAccountToken("");
}

void AuthManager::onLoginCallback(int code, const QJsonObject& response, bool registerMode) {
    if (0 == code) {
        setAPaasAccountId(response["userUuid"].toString());
        setAPaasAccountToken(response["userToken"].toString());
        setAppUserNick(response["nickname"].toString());
        setPersonalMeetingId(response["privateMeetingNum"].toString());
        setPersonalShortMeetingId(response["shortMeetingNum"].toString());
        // setAPaasAppKey(response["appKey"].toString());
        // ConfigManager::getInstance()->setValue("localPaasAppKey", response["appKey"].toString());
        if (registerMode)
            Q_EMIT registeredAccount();
        else
            Q_EMIT loggedIn(response["userUuid"].toString());
    } else {
        setPhoneNumber("");
        emit error(code, response);
    }
}

QString AuthManager::extraInfo() const {
    return m_extraInfo;
}

void AuthManager::setExtraInfo(const QString& extraInfo) {
    m_extraInfo = extraInfo;
    Q_EMIT extraInfoChanged();
}

bool AuthManager::autoRegistered() const {
    return m_autoRegistered;
}

void AuthManager::setAutoRegistered(bool autoRegistered) {
    m_autoRegistered = autoRegistered;
    Q_EMIT autoRegisteredChanged();
}

QString AuthManager::personalShortMeetingId() const {
    return m_shortPersonalMeetingId;
}

void AuthManager::setPersonalShortMeetingId(const QString& shortMeetingId) {
    m_shortPersonalMeetingId = shortMeetingId;
    emit personalShortMeetingIdChanged();
}

QString AuthManager::personalMeetingId() const {
    return m_personalMeetingId;
}

void AuthManager::setPersonalMeetingId(const QString& personalMeetingId) {
    m_personalMeetingId = personalMeetingId;
    emit personalMeetingIdChanged();

    auto prettyMeetingId = personalMeetingId.mid(0, 3).append("-").append(personalMeetingId.mid(3, 3)).append("-").append(personalMeetingId.mid(6));
    setPrettyMeetingId(prettyMeetingId);
}

QString AuthManager::prettyMeetingId() const {
    return m_prettyMeetingId;
}

void AuthManager::setPrettyMeetingId(const QString& prettyMeetingId) {
    m_prettyMeetingId = prettyMeetingId;
    emit prettyMeetingIdChanged();
}

QString AuthManager::curDisplayCompany() const {
    return m_curDisplayCompany;
}

void AuthManager::setCurDisplayCompany(const QString& curDisplayCompany) {
    m_curDisplayCompany = curDisplayCompany;
    Q_EMIT curDisplayCompanyChanged();
}

QString AuthManager::maxMemberCount() const {
    return m_maxMemberCount;
}

void AuthManager::setMaxMemberCount(const QString& maxMemberCount) {
    m_maxMemberCount = maxMemberCount;
    Q_EMIT maxMemberCountChanged();
}

QString AuthManager::maxDuration() const {
    return m_maxDuration;
}

void AuthManager::setMaxDuration(const QString& maxDuration) {
    m_maxDuration = maxDuration;
    Q_EMIT maxDurationChanged();
}

QString AuthManager::curDisplayVersion() const {
    return m_curDisplayVersion;
}

void AuthManager::setCurDisplayVersion(const QString& curDisplayVersion) {
    m_curDisplayVersion = curDisplayVersion;
    Q_EMIT curDisplayVersionChanged();
}

QString AuthManager::paasServerAddress() const {
    auto cachedServerAddress =
        ConfigManager::getInstance()->getAPaasServerAddress() + kHttpScene + ConfigManager::getInstance()->getAPaasAppKey() + "/";
    return cachedServerAddress;
}

bool AuthManager::resetPasswordFlag() const {
    return m_resetPasswordFlag;
}

void AuthManager::setResetPasswordFlag(bool resetPasswordFlag) {
    m_resetPasswordFlag = resetPasswordFlag;
}

QString AuthManager::appUserOpenId() const {
    return m_appUserOpenId;
}

void AuthManager::setAppUserOpenId(const QString& appUserOpenId) {
    m_appUserOpenId = appUserOpenId;
    emit appUserOpenIdChanged();
}

QString AuthManager::aPaasAppKey() const {
    return ConfigManager::getInstance()->getAPaasAppKey();
}

void AuthManager::setAPaasAppKey(const QString& aPaasAppKey) {
    if (ConfigManager::getInstance()->getAPaasAppKey() != aPaasAppKey) {
        emit aPaasAppKeyChanged();
    }
}

QString AuthManager::aPaasAccountToken() const {
    return m_aPaasAccountToken;
}

void AuthManager::setAPaasAccountToken(const QString& aPaasAccountToken) {
    m_aPaasAccountToken = aPaasAccountToken;
    emit aPaasAccountTokenChanged();
}

QString AuthManager::aPaasAccountId() const {
    return m_aPaasAccountId;
}

void AuthManager::setAPaasAccountId(const QString& aPaasAccountId) {
    m_aPaasAccountId = aPaasAccountId;
    emit aPaasAccountIdChanged();
}

QString AuthManager::appUserNick() const {
    return m_appUserNick;
}

void AuthManager::setAppUserNick(const QString& appUserNick) {
    m_appUserNick = appUserNick;
    emit appUserNickChanged();
}

QString AuthManager::appUserId() const {
    return m_appUserId;
}

void AuthManager::setAppUserId(const QString& appUserId) {
    m_appUserId = appUserId;
    emit appUserIdChanged();
}

QString AuthManager::appUserToken() const {
    return m_appUserToken;
}

void AuthManager::setAppUserToken(const QString& appUserToken) {
    m_appUserToken = appUserToken;
    emit appUserTokenChanged();
}

QString AuthManager::phoneNumber() const {
    return m_phoneNumber;
}

void AuthManager::setPhoneNumber(const QString& phoneNumber) {
    m_phoneNumber = phoneNumber;
    ConfigManager::getInstance()->setValue("localPhoneNumber", phoneNumber);
    emit phoneNumberChanged();
}

QString AuthManager::phonePrefix() const {
    return m_phonePrefix;
}

void AuthManager::setPhonePrefix(const QString& phonePrefix) {
    m_phonePrefix = phonePrefix;
    emit phonePrefixChanged();
}

QString AuthManager::authCodeInternal() const {
    return m_authCodeInternal;
}

void AuthManager::setAuthCodeInternal(const QString& authCodeInternal) {
    m_authCodeInternal = authCodeInternal;
    emit authCodeInternalChanged();
}
