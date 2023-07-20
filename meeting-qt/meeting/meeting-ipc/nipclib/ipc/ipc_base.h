﻿// Copyright (c) 2022 NetEase, Inc. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef NIPCLIB_IPC_IPC_BASE_H_
#define NIPCLIB_IPC_IPC_BASE_H_

#include <iostream>
#include "nipclib/base/ipc_thread.h"
#include "nipclib/config/build_config.h"
#include "nipclib/ipc/ipc_def.h"
#include "nipclib/ipc/package/socket_data_warpper.h"
#include "nipclib/nipclib_export.h"
#include "nipclib/socket/server_socket_wrapper.h"
#include "tnet.h"

NIPCLIB_BEGIN_DECLS
template <typename TcpImpl>
class NIPCLIB_EXPORT IPCBase : public IIPC,
                               protected NS_NIPCLIB::IPCThread,
                               public NS_NIPCLIB::TcpClientHandler,
                               public NS_NIPCLIB::SupportWeakCallback {
public:
    IPCBase()
        : NS_NIPCLIB::IPCThread("ipc_base")
        , init_(false)
        , ready_(false)
        , init_callback_(nullptr)
        , ready_callback_(nullptr)
        , close_callback_(nullptr)
        , keep_alive_callback_(nullptr)
        , receive_callback_(nullptr)
        , socket_data_warpper_(nullptr) {}
    virtual ~IPCBase() {}

public:
    virtual bool Init(int port = 0) override {
        if (init_)
            return true;
        ipc_port_ = port;
        socket_data_warpper_ = std::make_unique<SocketDataWarpper>();
        AttachInternalBegin([this]() {
            tnet_startup();
            OnInternalBegin();
        });
        AttachInternalEnd([this]() {
            OnInternalEnd();
            tnet_cleanup();
        });
        init_ = true;
        return true;
    }
    virtual void Start() override { IPCThread::Start(); }
    virtual void Stop() override {
        IPCThread::Stop();
        IPCThread::Join();
    }
    virtual void AttachInit(const IPCInitCallback& init_callback) override { init_callback_ = init_callback; }
    virtual void AttachReady(const IPCReadyCallback& ready_callback) override { ready_callback_ = ready_callback; }
    virtual void AttachClose(const IPCCloseCallback& close_callback) override { close_callback_ = close_callback; }
    virtual void AttachKeepAliveTimeOut(const IPCKeepAliveTimeOutCallback& keep_alive_callback) override {
        keep_alive_callback_ = keep_alive_callback;
    }
    virtual void AttachReceiveData(const IPCReceiveDataCallback& receive_callback) override { receive_callback_ = receive_callback; }
    virtual int SendData(const IPCData& data) override {
        Log(1, "SendData: ");
        if (!TaskLoop()) {
            Log(1, "!TaskLoop(): ");
            return 0;
        }
        TaskLoop()->PostTask(ToWeakCallback([this, data]() {
            Log(1, "PostTask SendData: ");
            IPCData data_send = std::make_shared<IPCData::element_type>();
            if (socket_data_warpper_)
                socket_data_warpper_->PackSendData(*data, *data_send);
            Log(1, "socket_data_warpper_ PackSendData.");
            InvokeSendData(data_send);
        }));
        return 0;
    }
    virtual void Close() override {
        Log(1, "Close.");
        TaskLoop()->PostTask(ToWeakCallback([this]() {
            Log(1, "PostTask Close.");
            DoClose();
            InvokeCloseCallback();
        }));

        auto keep_alive_latest_ = std::chrono::steady_clock::now();

        while (!keep_alive_exit_ &&
               (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - keep_alive_latest_).count() < 3)) {
            std::this_thread::yield();
        };
    }
    virtual bool Ready() override { return ready_; }

protected:
    virtual bool OnAcceptClient(void* client_fd) override { return OnAccept(client_fd); }
    virtual void OnReceiveSocketData(int error_code, const void* data, size_t size) override {
        Log(1, "OnReceiveSocketData: ");
        std::shared_ptr<std::string> received_data = std::make_shared<std::string>();
        // received_data->resize(size);
        received_data->append((char*)data, size);
        TaskLoop()->PostTask(ToWeakCallback([this, received_data]() {
            Log(1, "PostTask OnReceiveSocketData: ");
            if (socket_data_warpper_->OnReceiveData(received_data->data(), received_data->size())) {
                Log(1, "socket_data_warpper_ OnReceiveData: ");
                std::string raw_data_buf;
                socket_data_warpper_->GetReceivedPack(raw_data_buf);
                do {
                    IPCData ipc_data = std::make_shared<IPCData::element_type>();
                    ipc_data->append(raw_data_buf);
                    Log(1, "do OnReceive: ");
                    OnReceive(ipc_data);
                } while (socket_data_warpper_->GetReceivedPack(raw_data_buf));
            } else {
                Log(1, "socket_data_warpper_ OnReceiveData failed.");
            }
        }));
    }
    virtual void OnSendSocketData(int error_code) override {}
    virtual void OnSocketClose(int error_code) override { OnClose(error_code); }
    virtual void OnSocketConnect(int error_code) override { OnConnect(error_code); }
    virtual void OnSocketError(int error_code, const void* data, size_t size) override {}

protected:
    virtual void SetReady(bool value = true) {
        ready_ = value;
        std::string d = ready_ ? "true" : "false";
        Log(1, "ready_:" + d);
        if (ready_) {
            TaskLoop()->PostTask(ToWeakCallback([this]() { InvokeReadyCallback(); }));
            keep_alive_exit_ = false;
        } else {
        }
    }
    virtual void OnInternalBegin() = 0;
    virtual void OnInternalEnd() = 0;
    virtual void DoClose() = 0;
    virtual bool OnAccept(void*) { return false; }
    virtual int InvokeSendData(const IPCData& data) = 0;
    virtual void InvokeInitCallback(bool ret, const std::string& host, int port) {
        if (init_callback_ != nullptr) {
            TaskLoop()->PostTask(ToWeakCallback([this, ret, host, port]() { init_callback_(ret, host, port); }));
        }
    }
    virtual void InvokeCloseCallback() {
        if (close_callback_ != nullptr)
            close_callback_();
    }
    virtual void InvokeKeepAliveTimeOutCallback() {
        if (keep_alive_callback_ != nullptr)
            keep_alive_callback_();
    }
    virtual void InvokeReceiveDataCallback(const IPCData& data) {
        if (receive_callback_ != nullptr)
            receive_callback_(data);
    }
    virtual void InvokeReadyCallback() {
        if (ready_callback_ != nullptr)
            ready_callback_();
    }
    virtual void OnReceive(const IPCData& data) = 0;
    virtual void OnClose(int error) = 0;
    virtual void OnConnect(int error){};

protected:
    std::shared_ptr<TcpImpl> tcp_impl_;
    int ipc_port_;
    std::atomic_bool keep_alive_exit_{true};

private:
    std::atomic_bool init_;
    std::atomic_bool ready_;
    IPCCloseCallback close_callback_;
    IPCKeepAliveTimeOutCallback keep_alive_callback_;
    IPCReceiveDataCallback receive_callback_;
    IPCReadyCallback ready_callback_;
    IPCInitCallback init_callback_;
    std::unique_ptr<SocketDataWarpper> socket_data_warpper_;
};
NIPCLIB_END_DECLS
#endif  // NIPCLIB_IPC_IPC_BASE_H_
