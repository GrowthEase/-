﻿// Copyright (c) 2022 NetEase, Inc. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

//
// Author: Ruan Liang <ruanliang@corp.netease.com>
// Date: 2011/6/9
//
// Lock implemetation on windows

#include "lock.h"

#if defined(OS_WIN)

namespace nbase {

NLock::NLock() {
    // The second parameter is the spin count, for short-held locks it avoid the
    // contending thread from going to sleep which helps performance greatly.
    ::InitializeCriticalSectionAndSpinCount(&os_lock_, 2000);
}

NLock::~NLock() {
    ::DeleteCriticalSection(&os_lock_);
}

bool NLock::Try() {
    if (::TryEnterCriticalSection(&os_lock_)) {
        return true;
    }
    return false;
}

void NLock::Lock() {
    ::EnterCriticalSection(&os_lock_);
}

void NLock::Unlock() {
    ::LeaveCriticalSection(&os_lock_);
}

}  // namespace nbase

#endif  // OS_WIN
