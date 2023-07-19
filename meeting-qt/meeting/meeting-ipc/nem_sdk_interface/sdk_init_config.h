﻿// Copyright (c) 2022 NetEase, Inc. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

/**
 * @file sdk_init_config.h
 * @brief SDK初始化配置头文件
 * @copyright (c) 2014-2021, NetEase Inc. All rights reserved
 * @author
 * @date 2021/04/08
 */

#ifndef NEM_SDK_INTERFACE_DEFINE_SDK_INIT_CONFIG_H__
#define NEM_SDK_INTERFACE_DEFINE_SDK_INIT_CONFIG_H__

#include <string>
#include "meeting.h"
#include "public_define.h"

NNEM_SDK_INTERFACE_BEGIN_DECLS

/**
 * @brief 应用信息
 */
class NEM_SDK_INTERFACE_EXPORT NEMAppInfo : public NEObject {
public:
    /**
     * @brief 构造函数
     */
    NEMAppInfo() {}

    /**
     * @brief 获取SDK路径
     * @return std::string
     */
    std::string SDKPath() const { return sdk_path_; }

    /**
     * @brief 设置SDK路径
     * @param path SDK路径
     * @return void
     */
    void SDKPath(const std::string& path) { sdk_path_ = path; }

    /**
     * @brief 获取组织名称
     * @return std::string
     */
    std::string OrganizationName() const { return organization_name_; }

    /**
     * @brief 设置组织名称
     * @param organization_name 组织名称
     * @return void
     */
    void OrganizationName(const std::string& organization_name) { organization_name_ = organization_name; }

    /**
     * @brief 获取应用名称
     * @return std::string
     */
    std::string ApplicationName() const { return application_name_; }

    /**
     * @brief 设置应用名称
     * @param application_name 应用名称
     * @return void
     */
    void ApplicationName(const std::string& application_name) { application_name_ = application_name; }

    /**
     * @brief 获取产品名称
     * @return std::string
     */
    std::string ProductName() const { return product_name_; }

    /**
     * @brief 设置产品名称
     * @param product_name 产品名称
     * @return void
     */
    void ProductName(const std::string& product_name) { product_name_ = product_name; }

private:
    std::string sdk_path_;          /**< SDK路径 */
    std::string organization_name_; /**< 组织名称 */
    std::string application_name_;  /**< 应用名称 */
    std::string product_name_;      /**< 产品名称 */
};

/**
 * @brief 日志等级枚举
 */
enum NELogLevel {
    NEVERBOSE,
    NEDEBUG,
    NEINFO,
    NEWARNING,
    NEERROR,
};

/**
 * @brief 日志配置
 */
class NEM_SDK_INTERFACE_EXPORT NELoggerConfig : public NEObject {
public:
    /**
     * @brief 构造函数
     */
    NELoggerConfig() {}

    /**
     * @brief 获取日志路径
     * @attention 之前如果没有设置，则获取为空，内部则保存在默认
     * @return std::string
     */
    std::string LoggerPath() const { return path_; }

    /**
     * @brief 设置日志路径
     * @param path 日志路径
     * @return void
     */
    void LoggerPath(const std::string& path) { path_ = path; }

    /**
     * @brief 获取日志等级
     * @return NELogLevel
     */
    NELogLevel LoggerLevel() const { return level_; }

    /**
     * @brief 设置日志等级
     * @param level 日志等级
     * @return void
     */
    void LoggerLevel(NELogLevel level) { level_ = level; }

private:
    std::string path_;          /**< 日志路径，不设置默认为内部路径 */
    NELogLevel level_ = NEINFO; /**< 日志等级，不设置默认为INFO */
};

/**
 * @brief SDK的配置
 */
class NEM_SDK_INTERFACE_EXPORT NEMeetingKitConfig : public NEObject {
public:
    /**
     * @brief 构造函数
     */
    NEMeetingKitConfig() = default;

    /**
     * @brief 构造函数
     * @param strDomain 域名
     */
    NEMeetingKitConfig(const std::string& strDomain)
        : domain(strDomain) {}

    /**
     * @brief 构造函数
     * @param strDomain 域名
     */
    NEMeetingKitConfig(std::string&& strDomain)
        : domain(std::move(strDomain)) {}

public:
    /**
     * @brief 获取应用信息
     * @return NEMAppInfo*
     */
    NEMAppInfo* getAppInfo() const { return &appInfo; }

    /**
     * @brief 获取应用appkey
     * @return std::string
     */
    std::string getAppKey() const { return appKey; }

    /**
     * @brief 设置应用appkey
     * @param value 应用appkey
     * @return void
     */
    void setAppKey(const std::string& value) { appKey = value; }

    /**
     * @brief 获取域名
     * @return std::string
     */
    std::string getDomain() const { return domain; }

    /**
     * @brief 设置域名
     * @param value 域名
     * @return void
     */
    void setDomain(const std::string& value) { domain = value; }

    /**
     * @brief 获取是否使用私有化服务配置
     * @return
     * - true: 使用
     * - false: 不使用
     */
    bool getUseAssetServerConfig() const { return useAssetServerConfig; }

    /**
     * @brief 设置是否使用私有化服务配置
     * @param bUse true使用，false不使用
     * @return void
     */
    void setUseAssetServerConfig(bool bUse) { useAssetServerConfig = bUse; }

    /**
     * @brief 获取保活间隔
     * @note 单位为秒
     * @return int
     */
    int getKeepAliveInterval() const { return keepAliveInterval; }

    /**
     * @brief 设置保活间隔
     * @param interval 间隔，单位为秒
     * @note 小于0则不进行保活，如果要保活则最小为3秒，建议设置超过5秒
     * @return void
     */
    void setKeepAliveInterval(int interval) { keepAliveInterval = interval; }

    /**
     * @brief 获取日志配置
     * @return NELoggerConfig*
     */
    NELoggerConfig* getLoggerConfig() const { return &loggerConfig; }

    /**
     * @brief 获取运行权限
     * @return bool
     */
    bool getRunAdmin() const { return runAdmin; }

    /**
     * @brief 设置运行权限
     * @param admin 管理员权限
     * @attention 仅Windows下有效
     * @return void
     */
    void setRunAdmin(bool admin) { runAdmin = admin; }

    /**
     * @brief 设置显示语言
     * @param uiLanguage 显示语言 {@link NEMeetingLanguage}
     * @return void
     */
    void setLanguage(NEMeetingLanguage uiLanguage) { language = uiLanguage; }

    /**
     * @brief 获取显示语言
     * @return NEMeetingLanguage 显示语言
     */
    NEMeetingLanguage getLanguage() const { return language; }

    /**
     * @brief 设置私有化服务器地址
     * @param url 私有化服务器地址
     * @return void
     */
    void setServerUrl(const std::string& url) { serverUrl = url; }

    /**
     * @brief 获取私有化服务器地址
     * @return std::string 私有化服务器地址
     */
    std::string getServerUrl() const { return serverUrl; }

private:
    mutable NEMAppInfo appInfo;                                   /**< 应用信息 */
    std::string appKey;                                           /**< 应用appkey */
    std::string domain;                                           /**< 应用域名 */
    bool useAssetServerConfig = false;                            /**< 使用私有化配置使能 */
    int keepAliveInterval = 10;                                   /**< 保活间隔，单位为秒，小于0则不进行保活 */
    bool runAdmin = false;                                        /**< 是否使用管理员权限启动，仅Windows下有效 */
    mutable NELoggerConfig loggerConfig;                          /**< 日志配置 */
    NEMeetingLanguage language = NEMeetingLanguage::kNEAutomatic; /**< 显示语言 {@link NEMeetingLanguage} */
    std::string serverUrl;                                        /**< 私有化服务器地址 */
};

NNEM_SDK_INTERFACE_END_DECLS

#endif  // NEM_SDK_INTERFACE_DEFINE_SDK_INIT_CONFIG_H__
