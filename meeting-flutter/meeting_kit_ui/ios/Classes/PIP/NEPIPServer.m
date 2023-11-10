// Copyright (c) 2022 NetEase, Inc. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#import "NEPIPServer.h"
#import <AVKit/AVKit.h>
@import NERoomKit;
#import "MeetingUILog.h"
#import "NEPIPRenderer.h"
#import "NESampleBufferDisplayView.h"
#import "NEScreenShareDisplayView.h"
@interface UIView (meeting_ui)
- (void)meeting_addConstrainedSubView:(UIView *)subview;
@end

@implementation UIView (meeting_ui)

- (void)meeting_addConstrainedSubView:(UIView *)subview {
  [self addSubview:subview];
  subview.translatesAutoresizingMaskIntoConstraints = NO;
  [NSLayoutConstraint activateConstraints:@[
    [subview.topAnchor constraintEqualToAnchor:self.topAnchor],
    [subview.leadingAnchor constraintEqualToAnchor:self.leadingAnchor],
    [subview.trailingAnchor constraintEqualToAnchor:self.trailingAnchor],
    [subview.bottomAnchor constraintEqualToAnchor:self.bottomAnchor]
  ]];
}
@end

@interface NEPIPResult : NSObject
+ (NSDictionary *)code:(NSInteger)code desc:(NSString *)desc;
@end

@implementation NEPIPResult
+ (NSDictionary *)code:(NSInteger)code desc:(NSString *)desc {
  NSMutableDictionary *tempDic = @{}.mutableCopy;
  tempDic[@"code"] = @(code);
  tempDic[@"description"] = desc;
  return tempDic.copy;
}
@end

#define kScreenWidth UIScreen.mainScreen.bounds.size.width
#define kScreenHeight UIScreen.mainScreen.bounds.size.height

static NSString *kEndPIPNotificationName = @"com.netease.yunxin.kit.pip.notification.ended";
static NSString *kPIPServerClassName = @"NEPIPServer";
API_AVAILABLE(ios(15.0))
@interface NEPIPServer () <AVPictureInPictureControllerDelegate, NERoomListener>
@property(nonatomic, strong) AVPictureInPictureVideoCallViewController *videoCallViewController;
@property(nonatomic, strong) AVPictureInPictureController *pipController;
@property(nonatomic, strong) NESampleBufferDisplayView *displayView;
@property(nonatomic, strong) NEScreenShareDisplayView *shareDisplayView;
@property(nonatomic, strong) NEPIPRenderer *shareRenderer;
@property(nonatomic, copy) NSString *currentUuid;
@property(nonatomic, copy) NSString *shareUuid;
@property(nonatomic, strong) NERoomContext *roomContext;
@property(nonatomic, strong) NSMutableDictionary<NSString *, NEPIPRenderer *> *renderers;
// 停止后，2s恢复
@property(nonatomic, assign) BOOL isStopPIP;
@end

@implementation NEPIPServer
- (void)pipAction:(FlutterMethodCall *)call result:(FlutterResult)result {
  NSString *method = call.method;
  if ([method isEqualToString:@"setupPIP"]) {
    [self setupPIP:call result:result];
  } else if ([method isEqualToString:@"disposePIP"]) {
    [self disposePIP:call result:result];
  } else if ([method isEqualToString:@"changeVideo"]) {
    [self changeVideo:call result:result];
  } else if ([method isEqualToString:@"isPIPActive"]) {
    [self isPIPActive:result];
  } else if ([method isEqualToString:@"memberVideoChange"]) {
    [self memberVideoChange:call result:result];
  } else if ([method isEqualToString:@"memberAudioChange"]) {
    [self memberAudioChange:call result:result];
  } else if ([method isEqualToString:@"pipAvailable"]) {
    [self isPIPActive:result];
  } else if ([method isEqualToString:@"memberInCall"]) {
    [self memberInCall:call result:result];
  } else if ([method isEqualToString:@"updatePIPParams"]) {
    result(@YES);
  } else {
    result(FlutterMethodNotImplemented);
  }
}

- (void)setupPIP:(FlutterMethodCall *)call result:(FlutterResult)result {
  if (!AVPictureInPictureController.isPictureInPictureSupported) {
    result([NEPIPResult code:-1 desc:@"Not supported PIP."]);
    return;
  }
  UIView *view = UIApplication.sharedApplication.keyWindow.rootViewController.view;
  if (!view) {
    result([NEPIPResult code:-1 desc:@"No root view exists."]);
    return;
  }
  NSDictionary *arguments = call.arguments;
  NSString *roomUuid = arguments[@"roomUuid"];
  if (!roomUuid) {
    result([NEPIPResult code:-1 desc:@"The parameter roomUuid is empty."]);
    return;
  }
  NERoomContext *context = [NERoomKit.shared.roomService getRoomContextWithRoomUuid:roomUuid];
  if (!context) {
    result([NEPIPResult code:-1 desc:@"The room does not exist."]);
    return;
  }
  self.roomContext = context;

  AVPictureInPictureVideoCallViewController *videoCallVC =
      [[AVPictureInPictureVideoCallViewController alloc] init];
  videoCallVC.preferredContentSize =
      CGSizeMake(view.frame.size.height / 16.0 * 9.0, view.frame.size.height);
  NESampleBufferDisplayView *displayView =
      [[NESampleBufferDisplayView alloc] initWithFrame:CGRectZero];
  displayView.backgroundColor = UIColor.blackColor;
  [videoCallVC.view meeting_addConstrainedSubView:displayView];
  videoCallVC.view.backgroundColor = UIColor.blackColor;
  // 初始化默认为自己
  [displayView updateStateWithMember:self.roomContext.localMember isSelf:YES];
  self.displayView = displayView;

  NEScreenShareDisplayView *shareView = [[NEScreenShareDisplayView alloc] initWithFrame:CGRectZero];
  shareView.backgroundColor = UIColor.blackColor;
  shareView.hidden = YES;
  [videoCallVC.view meeting_addConstrainedSubView:shareView];
  self.shareDisplayView = shareView;

  self.videoCallViewController = videoCallVC;
  AVPictureInPictureControllerContentSource *contentSource =
      [[AVPictureInPictureControllerContentSource alloc]
          initWithActiveVideoCallSourceView:view
                      contentViewController:videoCallVC];
  self.pipController = [[AVPictureInPictureController alloc] initWithContentSource:contentSource];
  self.pipController.delegate = self;

  NSNumber *autoEnterPIP = arguments[@"auto_enter_pip"];
  self.pipController.canStartPictureInPictureAutomaticallyFromInline = autoEnterPIP.boolValue;
  result([NEPIPResult code:0 desc:@"Successfully set up pip."]);
}
- (void)changeVideo:(FlutterMethodCall *)call result:(FlutterResult)result {
  if (!self.pipController.isPictureInPictureActive || self.isStopPIP) {
    result([NEPIPResult code:-1 desc:@"Not pip active or stopping."]);
    return;
  }
  NSDictionary *arguments = call.arguments;
  NSString *roomUuid = arguments[@"roomUuid"];
  NSString *userUuid = arguments[@"userUuid"];
  NSString *shareUuid = arguments[@"shareUuid"];
  NSNumber *isInCall = arguments[@"isInCall"];
  if (!roomUuid || !userUuid || !shareUuid || !isInCall || !self.roomContext) {  // 房间不存在
    result([NEPIPResult code:-1 desc:@"The room does not exist."]);
    return;
  }

  self.shareDisplayView.hidden = !shareUuid.length;
  if (shareUuid.length) {
    if ([shareUuid isEqualToString:self.roomContext.localMember.uuid]) {  // 自己共享
      self.shareDisplayView.hidden = YES;
      self.shareUuid = nil;
      self.shareRenderer = nil;
    } else if (![self.shareUuid isEqualToString:shareUuid]) {
      self.shareUuid = shareUuid;
      NEPIPRenderer *renderer = [NEPIPRenderer renderWithUserUuid:shareUuid];
      __weak typeof(self) weakSelf = self;
      renderer.renderResult =
          ^(NSString *uuid, uint32_t width, uint32_t height, CMSampleBufferRef bufferRef) {
            [weakSelf onShareFrame:uuid width:width height:height buffer:bufferRef];
          };
      self.shareRenderer = renderer;
      [self.roomContext.rtcController setupRemoteVideoSubStreamCanvasWithVideoView:nil
                                                                          userUuid:shareUuid];
      NERoomVideoView *canvas = [[NERoomVideoView alloc] init];
      canvas.useExternalRender = YES;
      canvas.externalVideoRender = renderer;
      [self.roomContext.rtcController setupRemoteVideoSubStreamCanvasWithVideoView:canvas
                                                                          userUuid:shareUuid];
      [self.roomContext.rtcController subscribeRemoteVideoSubStreamWithUserUuid:shareUuid];
    }
    [self.displayView showInfo:NO];
  } else {
    self.shareUuid = nil;
    self.shareRenderer = nil;
    [self.displayView showInfo:YES];
  }

  if (userUuid.length) {
    if ([userUuid isEqualToString:self.roomContext.localMember.uuid]) {  // 自己
      self.currentUuid = nil;
      [self.displayView updateStateWithMember:self.roomContext.localMember isSelf:YES];
      [self.displayView showPhone:isInCall.boolValue];
    } else if (![self.currentUuid isEqualToString:userUuid]) {
      self.currentUuid = userUuid;
      NERoomMember *member = [self.roomContext getMemberWithUuid:userUuid];
      [self.displayView updateStateWithMember:member ?: self.roomContext.localMember
                                       isSelf:member ? NO : YES];
      [self.displayView showPhone:isInCall.boolValue];
      if (member != nil) {
        NEPIPRenderer *renderer = [NEPIPRenderer renderWithUserUuid:userUuid];
        __weak typeof(self) weakSelf = self;
        renderer.renderResult =
            ^(NSString *userUuid, uint32_t width, uint32_t height, CMSampleBufferRef bufferRef) {
              [weakSelf onVideoFrame:userUuid width:width height:height buffer:bufferRef];
            };
        self.renderers[userUuid] = renderer;

        NERoomVideoView *canvas = [[NERoomVideoView alloc] init];
        canvas.useExternalRender = YES;
        canvas.externalVideoRender = renderer;

        [self.roomContext.rtcController setupRemoteVideoCanvasWithVideoView:canvas
                                                                   userUuid:userUuid];
        [self.roomContext.rtcController
            subscribeRemoteVideoStreamWithUserUuid:userUuid
                                        streamType:NEVideoStreamTypeHigh];
      }
    }
  }
  result([NEPIPResult code:0 desc:@"Successfully change video."]);
}
- (void)isPIPActive:(FlutterResult)result {
  if (self.pipController && self.pipController.isPictureInPictureActive && !self.isStopPIP) {
    result(@(YES));
  } else {
    result(@(NO));
  }
}
- (void)disposePIP:(FlutterMethodCall *)call result:(FlutterResult)result {
  if (self.pipController.isPictureInPictureActive) {
    [NSNotificationCenter.defaultCenter postNotificationName:kEndPIPNotificationName object:nil];
  }

  if (self.shareUuid.length) {
    [self.roomContext.rtcController setupRemoteVideoSubStreamCanvasWithVideoView:nil
                                                                        userUuid:self.shareUuid];
  }
  if (self.currentUuid.length) {
    [self.roomContext.rtcController setupRemoteVideoCanvasWithVideoView:nil
                                                               userUuid:self.currentUuid];
  }
  self.currentUuid = nil;
  self.shareUuid = nil;
  self.shareRenderer = nil;
  self.roomContext = nil;
  [self.renderers removeAllObjects];
  self.isStopPIP = NO;
  if (!self.pipController.isPictureInPictureActive) {
    self.videoCallViewController = nil;
    self.pipController = nil;
    self.shareDisplayView = nil;
    self.displayView = nil;
    result(@(NO));
    return;
  }
  self.videoCallViewController = nil;
  self.pipController = nil;
  self.shareDisplayView = nil;
  self.displayView = nil;
  result(@(YES));
}

- (void)memberAudioChange:(FlutterMethodCall *)call result:(FlutterResult)result {
  if (!self.pipController.isPictureInPictureActive || self.isStopPIP) {
    result([NEPIPResult code:-1 desc:@"Not pip active or stopping."]);
    return;
  }

  NSDictionary *arguments = call.arguments;
  NSString *userUuid = arguments[@"userUuid"];
  NSNumber *isAudioOn = arguments[@"isAudioOn"];
  if (!userUuid || !isAudioOn || !self.roomContext) {  // 房间不存在
    result([NEPIPResult code:-1 desc:@"The room does not exist."]);
    return;
  }
  NERoomMember *member = [self.roomContext getMemberWithUuid:userUuid];
  if (!member) {
    result([NEPIPResult code:-1 desc:@"Member does not exist."]);
    return;
  }
  [self.displayView updateStateWithMember:member isSelf:NO];
  result([NEPIPResult code:0 desc:@"Successfully member audio change."]);
}
- (void)memberVideoChange:(FlutterMethodCall *)call result:(FlutterResult)result {
  if (!self.pipController.isPictureInPictureActive || self.isStopPIP) {
    result([NEPIPResult code:-1 desc:@"Not pip active or stopping."]);
    return;
  }

  NSDictionary *arguments = call.arguments;
  NSString *userUuid = arguments[@"userUuid"];
  NSNumber *isVideoOn = arguments[@"isVideoOn"];
  if (!userUuid || !isVideoOn || !self.roomContext) {  // 房间不存在
    result([NEPIPResult code:-1 desc:@"The room does not exist."]);
    return;
  }
  if ([self.roomContext.localMember.uuid isEqualToString:userUuid]) {
    result([NEPIPResult code:-1 desc:@"Do not process your own changes."]);
    return;
  }
  NERoomMember *member = [self.roomContext getMemberWithUuid:userUuid];
  if (!member) {
    result([NEPIPResult code:-1 desc:@"Member does not exist."]);
    return;
  }
  NEPIPRenderer *renderer = self.renderers[userUuid];
  if (isVideoOn.boolValue) {
    if ([userUuid isEqualToString:self.currentUuid]) {
      [self.displayView updateStateWithMember:member isSelf:NO];
      __weak typeof(self) weakSelf = self;
      NEPIPRenderer *renderer = [NEPIPRenderer renderWithUserUuid:userUuid];
      renderer.renderResult =
          ^(NSString *userUuid, uint32_t width, uint32_t height, CMSampleBufferRef bufferRef) {
            [weakSelf onVideoFrame:userUuid width:width height:height buffer:bufferRef];
          };
      self.renderers[userUuid] = renderer;
      [self.roomContext.rtcController setupRemoteVideoCanvasWithVideoView:nil userUuid:userUuid];
      NERoomVideoView *canvas = [[NERoomVideoView alloc] init];
      canvas.useExternalRender = YES;
      canvas.externalVideoRender = renderer;
      [self.roomContext.rtcController setupRemoteVideoCanvasWithVideoView:canvas userUuid:userUuid];
      [self.roomContext.rtcController subscribeRemoteVideoStreamWithUserUuid:userUuid
                                                                  streamType:NEVideoStreamTypeHigh];
    }
  } else {
    if (renderer && [userUuid isEqualToString:self.currentUuid]) {
      [self.roomContext.rtcController setupRemoteVideoCanvasWithVideoView:nil userUuid:userUuid];
      [self.renderers removeObjectForKey:userUuid];
      [self.displayView updateStateWithMember:member isSelf:NO];
      self.currentUuid = nil;
    }
  }
  result([NEPIPResult code:0 desc:@"Successfully member video change."]);
}
- (void)memberInCall:(FlutterMethodCall *)call result:(FlutterResult)result {
  if (!self.pipController.isPictureInPictureActive || self.isStopPIP) {
    result([NEPIPResult code:-1 desc:@"Not pip active or stopping."]);
    return;
  }
  NSDictionary *arguments = call.arguments;
  NSString *userUuid = arguments[@"userUuid"];
  NSNumber *isInCall = arguments[@"isInCall"];
  if (!userUuid || !isInCall || !self.roomContext) {  // 房间不存在 / 参数错误
    result([NEPIPResult code:-1 desc:@"The room does not exist."]);
    return;
  }
  if ([userUuid isEqualToString:self.roomContext.localMember.uuid]) {  // 自己
    if (!self.currentUuid) {
      [self.displayView showPhone:isInCall.boolValue];
    }
  } else {
    if ([self.currentUuid isEqualToString:userUuid]) {
      [self.displayView showPhone:isInCall.boolValue];
    }
  }
  result([NEPIPResult code:0 desc:@"Successfully member incall."]);
}

- (void)onVideoFrame:(NSString *)userUuid
               width:(uint32_t)width
              height:(uint32_t)height
              buffer:(CMSampleBufferRef)buffer {
  if (!buffer || ![self.currentUuid isEqualToString:userUuid] || self.shareUuid) return;

  AVSampleBufferDisplayLayer *layer = (AVSampleBufferDisplayLayer *)self.displayView.layer;
  if (layer.status == AVQueuedSampleBufferRenderingStatusFailed) {
    [layer flush];
  }
  [layer enqueueSampleBuffer:buffer];

  if (self.shareUuid.length) return;
  CGSize size = self.videoCallViewController.view.frame.size;
  CGFloat oldWidth = size.width;
  CGFloat oldHeight = size.height;

  CGSize originalSize =
      UIApplication.sharedApplication.keyWindow.rootViewController.view.frame.size;
  CGFloat originalWidth = originalSize.width;
  CGFloat originalHeight = originalSize.height;

  if (width < height) {  // 宽 < 高
    if (oldWidth > oldHeight) {
      if (originalWidth < originalHeight) {
        size = CGSizeMake(originalHeight / 16.0 * 9.0, originalHeight);
      } else {
        size = CGSizeMake(originalWidth / 16.0 * 9.0, originalWidth);
      }
    } else {
      size = CGSizeMake(originalHeight / 16.0 * 9.0, originalHeight);
    }
  } else {  // 宽 > 高
    if (oldWidth < oldHeight) {
      if (originalWidth < originalHeight) {
        size = CGSizeMake(originalHeight, originalHeight / 16.0 * 9.0);
      } else {
        size = CGSizeMake(originalWidth, originalWidth / 16.0 * 9.0);
      }
    } else {
      size = CGSizeMake(originalWidth, originalWidth / 16.0 * 9.0);
    }
  }
  self.videoCallViewController.preferredContentSize = size;
}
- (void)onShareFrame:(NSString *)userUuid
               width:(uint32_t)width
              height:(uint32_t)height
              buffer:(CMSampleBufferRef)buffer {
  if (!buffer || ![self.shareUuid isEqualToString:userUuid]) return;
  AVSampleBufferDisplayLayer *layer = (AVSampleBufferDisplayLayer *)self.shareDisplayView.layer;
  if (layer.status == AVQueuedSampleBufferRenderingStatusFailed) {
    [layer flush];
  }
  [layer enqueueSampleBuffer:buffer];

  CGSize size = CGSizeMake(kScreenWidth, kScreenHeight);
  if (width < height) {
    if (kScreenWidth < kScreenHeight) {
      size = CGSizeMake(kScreenHeight / 16.0 * 9.0, kScreenHeight);
    } else {
      size = CGSizeMake(kScreenWidth / 16.0 * 9.0, kScreenWidth);
    }
    //    size = CGSizeMake(kScreenWidth, kScreenHeight);
  } else {
    if (kScreenWidth < kScreenHeight) {
      size = CGSizeMake(kScreenHeight, kScreenHeight / 16.0 * 9.0);
    } else {
      size = CGSizeMake(kScreenHeight, kScreenHeight / 16.0 * 9.0);
    }
    //    size = CGSizeMake(kScreenHeight, kScreenWidth);
  }
  self.videoCallViewController.preferredContentSize = size;
}

#pragma mark------------------------ AVPictureInPictureControllerDelegate ------------------------
- (void)pictureInPictureControllerWillStartPictureInPicture:
    (AVPictureInPictureController *)pictureInPictureController {
  [MeetingUILog infoLog:kPIPServerClassName desc:@"🖼 Picture-in-picture will start."];
}
- (void)pictureInPictureControllerDidStartPictureInPicture:
    (AVPictureInPictureController *)pictureInPictureController {
  [MeetingUILog infoLog:kPIPServerClassName desc:@"🖼 Picture-in-picture has started."];
}
- (void)pictureInPictureControllerWillStopPictureInPicture:
    (AVPictureInPictureController *)pictureInPictureController {
  [MeetingUILog infoLog:kPIPServerClassName desc:@"🖼 Picture-in-picture will stop."];
  //  [NSNotificationCenter.defaultCenter postNotificationName:kEndPIPNotificationName object:nil];
}
- (void)pictureInPictureControllerDidStopPictureInPicture:
    (AVPictureInPictureController *)pictureInPictureController {
  [MeetingUILog infoLog:kPIPServerClassName desc:@"🖼 Picture-in-picture did stop."];
}
- (void)pictureInPictureController:(AVPictureInPictureController *)pictureInPictureController
    failedToStartPictureInPictureWithError:(NSError *)error {
  [MeetingUILog infoLog:kPIPServerClassName desc:@"🖼 Picture-in-picture start to fail."];
}
- (void)pictureInPictureController:(AVPictureInPictureController *)pictureInPictureController
    restoreUserInterfaceForPictureInPictureStopWithCompletionHandler:
        (void (^)(BOOL))completionHandler {
  if (completionHandler) completionHandler(YES);
}
- (NSMutableDictionary *)renderers {
  if (!_renderers) {
    _renderers = @{}.mutableCopy;
  }
  return _renderers;
}
@end
