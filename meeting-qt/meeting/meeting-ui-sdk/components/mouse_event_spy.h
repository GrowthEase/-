﻿// Copyright (c) 2022 NetEase, Inc. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef MOUSEEVENTSPY_H
#define MOUSEEVENTSPY_H

#include <QJSEngine>
#include <QObject>
#include <QQmlEngine>
#include <QtQml>

class MouseEventSpy : public QObject {
    Q_OBJECT
public:
    explicit MouseEventSpy(QObject* parent = 0);
    ~MouseEventSpy();

signals:
    void mousePosDetected(int mousePosX, int mousePosY);

public slots:
    void timeout();

private:
    QTimer m_mousePosTimer;
    QPoint m_lastMousePos;
};

#endif  // MOUSEEVENTSPY_H
