﻿// Copyright (c) 2022 NetEase, Inc. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef LICENCE_BOX_H_
#define LICENCE_BOX_H_

#include "build/stdafx.h"

class LicenceForm : public ui::WindowImplBase {
public:
    LicenceForm();
    virtual ~LicenceForm();

    static LicenceForm* GetInstance();

public:
    //ӿʵ
    virtual std::wstring GetSkinFolder() override;
    virtual std::wstring GetSkinFile() override;
    virtual ui::UILIB_RESOURCETYPE GetResourceType() const;
    virtual std::wstring GetZIPFileName() const;
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

public:
    //麯
    virtual std::wstring GetWindowClassName() const override;
    virtual UINT GetClassStyle() const override;
    virtual void OnFinalMessage(HWND hWnd);
    virtual void InitWindow() override;
    virtual bool Notify(ui::EventArgs* msg);

private:
    virtual std::wstring GetWindowId() const;
    std::wstring GetLicenceText();

public:
    static const LPCTSTR kClassName;
    static const LPCTSTR kCheckBoxName;

private:
    std::wstring edit_text_;  //
    ui::RichEdit* edit_;
};

#endif  // LICENCE_BOX_H_
