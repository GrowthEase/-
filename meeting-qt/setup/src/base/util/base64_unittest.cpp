﻿// Copyright (c) 2022 NetEase, Inc. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

//
// Author: Ruan Liang <ruanliang@corp.netease.com>
// Date: 2011/6/14
//
// Base64 Unittest

#if defined(WITH_UNITTEST)

#include "base/util/base64.h"
#include "gtest/gtest.h"

TEST(Base64, Basic) {
    const std::string kText = "hello world";
    const std::string kBase64Text = "aGVsbG8gd29ybGQ=";

    std::string encoded, decoded;
    bool ok;

    ok = nbase::Base64Encode(kText, &encoded);
    EXPECT_TRUE(ok);
    EXPECT_EQ(kBase64Text, encoded);

    ok = nbase::Base64Decode(encoded, &decoded);
    EXPECT_TRUE(ok);
    EXPECT_EQ(kText, decoded);
}

#endif  // WITH_UNITTEST
