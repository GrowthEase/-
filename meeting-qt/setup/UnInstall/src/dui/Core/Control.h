﻿// Copyright (c) 2022 NetEase, Inc. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef UI_CORE_CONTROL_H_
#define UI_CORE_CONTROL_H_

#pragma once

namespace ui {

/////////////////////////////////////////////////////////////////////////////////////
//

typedef Control*(CALLBACK* FINDCONTROLPROC)(Control*, LPVOID);

class UILIB_API UserDataBase {
public:
    virtual ~UserDataBase() {}
};

class UILIB_API Control : public PlaceHolder {
public:
    Control();
    virtual ~Control();

public:
    virtual CursorType GetCursorType() const;
    void SetCursorType(CursorType flag);

    virtual void Activate();
    virtual bool IsActivatable() const;

    // ͼ
    std::wstring GetBkColor() const;
    void SetBkColor(const std::wstring& dwColor);
    std::wstring GetStateColor(ControlStateType stateType);
    void SetStateColor(ControlStateType stateType, const std::wstring& dwColor);

    std::wstring GetBkImage() const;
    std::string GetUTF8BkImage() const;
    void SetBkImage(const std::wstring& pStrImage);
    void SetUTF8BkImage(const std::string& pStrImage);

    std::wstring GetStateImage(ControlStateType stateType);
    void SetStateImage(ControlStateType stateType, const std::wstring& pStrImage);

    std::wstring GetForeStateImage(ControlStateType stateType);
    void SetForeStateImage(ControlStateType stateType, const std::wstring& pStrImage);

    ControlStateType GetState() const;
    void SetState(ControlStateType pStrState);
    virtual Image* GetEstimateImage();

    CSize GetBorderRound() const;
    void SetBorderRound(CSize cxyRound);
    void GetImage(Image& duiImage) const;
    bool DrawImage(HDC hDC, Image& duiImage, const std::wstring& pStrModify = L"", int fade = DUI_NOSET_VALUE);

    //߿
    int GetBorderSize() const;
    void SetBorderSize(int nSize);
    std::wstring GetBorderColor() const;
    void SetBorderColor(const std::wstring& dwBorderColor);

    void SetBorderSize(UiRect rc);
    int GetLeftBorderSize() const;
    void SetLeftBorderSize(int nSize);
    int GetTopBorderSize() const;
    void SetTopBorderSize(int nSize);
    int GetRightBorderSize() const;
    void SetRightBorderSize(int nSize);
    int GetBottomBorderSize() const;
    void SetBottomBorderSize(int nSize);

    // λ
    virtual UiRect GetPos(bool bContainShadow = true) const override;
    virtual void SetPos(UiRect rc) override;
    virtual UiRect GetMargin() const;
    virtual void SetMargin(UiRect rcMargin);

    // ʾ
    virtual std::wstring GetToolTip() const;
    virtual std::string GetUTF8ToolTip() const;
    virtual void SetToolTip(const std::wstring& pstrText);
    virtual void SetUTF8ToolTip(const std::string& pstrText);
    virtual void SetToolTipWidth(int nWidth);
    virtual int GetToolTipWidth(void) const;  // ToolTip

    // ˵
    virtual bool IsContextMenuUsed() const;
    virtual void SetContextMenuUsed(bool bMenuUsed);

    // û
    virtual std::wstring GetDataID() const;  //ûʹ
    virtual std::string GetUTF8DataID() const;
    virtual void SetDataID(const std::wstring& pstrText);  //ûʹ
    virtual void SetUTF8DataID(const std::string& pstrText);

    virtual UserDataBase* GetUserDataBase() const;             //ûʹ
    virtual void SetUserDataBase(UserDataBase* userDataBase);  //ûʹ

    // һЩҪ
    virtual void SetVisible(bool bVisible = true);
    virtual void SetInternVisible(bool bVisible = true);  // ڲãЩuiӵдھҪд˺
    virtual bool IsEnabled() const;
    virtual void SetEnabled(bool bEnable = true);
    virtual bool IsMouseEnabled() const;
    virtual void SetMouseEnabled(bool bEnable = true);
    virtual bool IsKeyboardEnabled() const;
    virtual void SetKeyboardEnabled(bool bEnable = true);
    virtual bool IsFocused() const;
    virtual void SetFocus();
    virtual bool IsMouseFocused() const { return m_bMouseFocused; }
    virtual void SetMouseFocused(bool mouseFocused) { m_bMouseFocused = mouseFocused; }

    virtual Control* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags, CPoint scrollPos = CPoint());

    void HandleMessageTemplate(EventType eventType, WPARAM wParam = 0, LPARAM lParam = 0, TCHAR tChar = 0, CPoint mousePos = CPoint());
    virtual void HandleMessageTemplate(EventArgs& msg);
    virtual void HandleMessage(EventArgs& msg);
    virtual bool MouseEnter(EventArgs& msg);
    virtual bool MouseLeave(EventArgs& msg);
    virtual bool ButtonDown(EventArgs& msg);
    virtual bool ButtonUp(EventArgs& msg);
    virtual void SetAttribute(const std::wstring& pstrName, const std::wstring& pstrValue);
    void SetClass(const std::wstring& pstrClass);
    void ApplyAttributeList(const std::wstring& strList);
    bool OnApplyAttributeList(const std::wstring& receiver, const std::wstring& strList, EventArgs* eventArgs);

    virtual CSize EstimateSize(CSize szAvailable);
    virtual CSize EstimateText(CSize szAvailable, bool& reEstimateSize);

    void AlphaPaint(HDC hDC, const UiRect& rcPaint);
    virtual void Paint(HDC hDC, const UiRect& rcPaint);
    virtual void PaintBkColor(HDC hDC);
    virtual void PaintBkImage(HDC hDC);
    virtual void PaintStatusColor(HDC hDC);
    virtual void PaintStatusImage(HDC hDC);
    virtual void PaintText(HDC hDC);
    virtual void PaintBorder(HDC hDC);

    void SetNoFocus();  //ؼԶҪ㣬KillFocusһ

    void SetAlpha(int alpha);
    int GetAlpha() const { return m_nAlpha; }

    void SetHotAlpha(int nHotAlpha);
    int GetHotAlpha() const { return m_nHotAlpha; }

    bool IsAlpha() const { return m_nAlpha != 255; }

    CPoint GetRenderOffset() const { return m_renderOffset; }
    void SetRenderOffset(CPoint renderOffset) {
        m_renderOffset = renderOffset;
        Invalidate();
    }
    void SetRenderOffsetX(int renderOffsetX) {
        m_renderOffset.x = renderOffsetX;
        Invalidate();
    }
    void SetRenderOffsetY(int renderOffsetY) {
        m_renderOffset.y = renderOffsetY;
        Invalidate();
    }

    bool IsPointInWithScrollOffset(const CPoint& point) const;

    void GifPlay();
    void StopGifPlay(GifStopType type = GifStopType::CUR);
    void StartGifPlayForUI(GifStopType type = GifStopType::FIRST);
    void StopGifPlayForUI(GifStopType type = GifStopType::CUR);
    virtual void SetVisible_(bool bVisible);

    AnimationManager& GetAnimationManager() { return m_animationManager; }

    void AttachAllEvents(const EventCallback& callback) { OnEvent[EventType::ALL] += callback; }

    void AttachMouseEnter(const EventCallback& callback) { OnEvent[EventType::MOUSEENTER] += callback; }

    void AttachMouseLeave(const EventCallback& callback) { OnEvent[EventType::MOUSELEAVE] += callback; }

    void AttachMouseHover(const EventCallback& callback) { OnEvent[EventType::MOUSEHOVER] += callback; }

    void AttachButtonDown(const EventCallback& callback) { OnEvent[EventType::BUTTONDOWN] += callback; }

    void AttachButtonUp(const EventCallback& callback) { OnEvent[EventType::BUTTONUP] += callback; }

    void AttachSetFocus(const EventCallback& callback) { OnEvent[EventType::SETFOCUS] += callback; }

    void AttachKillFocus(const EventCallback& callback) { OnEvent[EventType::KILLFOCUS] += callback; }

    void AttachMenu(const EventCallback& callback) { OnEvent[EventType::MENU] += callback; }

protected:
    friend WindowBuilder;

    void AttachXmlEvent(EventType eventType, const EventCallback& callback) { OnXmlEvent[eventType] += callback; }

    EventMap OnXmlEvent;

protected:
    EventMap OnEvent;
    std::unique_ptr<UserDataBase> m_sUserDataBase;
    bool m_bMenuUsed = false;
    bool m_bEnabled = true;
    bool m_bMouseEnabled = true;
    bool m_bKeyboardEnabled = true;
    bool m_bFocused = false;
    bool m_bMouseFocused = false;
    bool m_bSetPos = false;   // ֹSetPosѭ
    bool m_bNoFocus = false;  //ؼҪ
    bool m_bClip = false;
    bool m_bGifPlay = true;  // UIĿ
    CSize m_szEstimateSize;
    CPoint m_renderOffset;
    CSize m_cxyBorderRound;
    UiRect m_rcMargin;
    UiRect m_rcPaint;
    UiRect m_rcBorderSize;
    CursorType m_cursorType = CursorType::ARROW;  //Ӱؼ״
    ControlStateType m_uButtonState = ControlStateType::NORMAL;
    int m_nBorderSize = 0;
    int m_nTooltipWidth = 300;
    int m_nAlpha = 255;
    int m_nHotAlpha = 0;
    std::wstring m_sToolTip;
    std::wstring m_sUserData;
    std::wstring m_dwBkColor;
    StateColorMap m_colorMap;
    Image m_diBkImage;
    StateImageMap m_imageMap;
    StateImageMap m_foreImageMap;
    std::wstring m_dwBorderColor;
    nbase::WeakCallbackFlag m_gifWeakFlag;
    AnimationManager m_animationManager;
};

}  // namespace ui

#endif  // UI_CORE_CONTROL_H_
