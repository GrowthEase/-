﻿// Copyright (c) 2022 NetEase, Inc. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

/*
 *	Author		Wang Rongtao <rtwang@corp.netease.com>
 *	Date		2011-06-14
 *	Copyright	Hangzhou, Netease Inc.
 *	Brief		Common utilities for Windows applications
 */

#ifndef BASE_WIN32_WIN_UTIL_H_
#define BASE_WIN32_WIN_UTIL_H_

#include "base/base_config.h"

#if defined(OS_WIN)

#include <string>
#include "base/base_export.h"

namespace nbase {
namespace win32 {

BASE_EXPORT bool IsAddressInCurrentModule(void* address);
BASE_EXPORT bool IsModuleHandleValid(HMODULE module_handle);
BASE_EXPORT HMODULE GetModuleHandleFromAddress(void* address);
BASE_EXPORT HMODULE GetCurrentModuleHandle();

BASE_EXPORT std::wstring GetHostName();

BASE_EXPORT bool RunApp(const wchar_t* application, HANDLE* process = NULL);
BASE_EXPORT bool RunAppWithCommand(const wchar_t* application, const wchar_t* command, HANDLE* process = NULL);
BASE_EXPORT bool RunAppWithRedirection(const wchar_t* application,
                                       const wchar_t* command,
                                       HANDLE input,
                                       HANDLE output,
                                       HANDLE error,
                                       HANDLE* process = NULL);

BASE_EXPORT bool MinimizeProcessWorkingSize();
BASE_EXPORT bool SingletonRun(const wchar_t* application);

BASE_EXPORT bool IsRunningOnVistaOrHigher();

BASE_EXPORT bool OpenResource(const wchar_t* resource, const wchar_t* type, void*& data, unsigned long& size, HMODULE module = NULL);

}  // namespace win32
}  // namespace nbase

#endif  // OS_WIN
#endif  // BASE_WIN32_WIN_UTIL_H_
