// Copyright (c) 2014-2015 Martijn Walraven
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#import <Foundation/Foundation.h>

#import "METSubscription.h"

@class METDDPConnection;
@class METDatabase;
@class METMethodInvocation;
@protocol METDDPClientDelegate;
@class METAccount;

extern NSString * const METDDPErrorDomain;
typedef NS_ENUM(NSInteger, METDDPErrorType) {
  METDDPServerError = 0,
  METDDPVersionError,
};

typedef NS_ENUM(NSInteger, METDDPConnectionStatus) {
  METDDPConnectionStatusOffline = 0,
  METDDPConnectionStatusConnecting,
  METDDPConnectionStatusFailed,
  METDDPConnectionStatusWaiting,
  METDDPConnectionStatusConnected
};

extern NSString * const METDDPClientDidChangeConnectionStatusNotification;
extern NSString * const METDDPClientDidChangeAccountNotification;

typedef id (^METMethodStub)(NSArray *parameters);
typedef void (^METMethodCompletionHandler)(id result, NSError *error);

typedef void (^METLogInCompletionHandler)(NSError *error);
typedef void (^METLogOutCompletionHandler)(NSError *error);

@interface METDDPClient : NSObject

- (instancetype)initWithConnection:(METDDPConnection *)connection account:(METAccount *)account;
- (instancetype)initWithConnection:(METDDPConnection *)connection;
- (instancetype)initWithServerURL:(NSURL *)serverURL;

@property(weak, nonatomic) id<METDDPClientDelegate> delegate;

- (void)connect;
- (void)disconnect;

@property (assign, nonatomic, readonly, getter=isConnected) BOOL connected;
@property (assign, nonatomic, readonly) METDDPConnectionStatus connectionStatus;
@property (copy, nonatomic) METAccount *account;
@property (strong, nonatomic, readonly) METDatabase *database;

- (METSubscription *)addSubscriptionWithName:(NSString *)name;
- (METSubscription *)addSubscriptionWithName:(NSString *)name completionHandler:(METSubscriptionCompletionHandler)completionHandler;
- (METSubscription *)addSubscriptionWithName:(NSString *)name parameters:(NSArray *)parameters;
- (METSubscription *)addSubscriptionWithName:(NSString *)name parameters:(NSArray *)parameters completionHandler:(METSubscriptionCompletionHandler)completionHandler;
- (void)removeSubscription:(METSubscription *)subscription;

- (void)defineStubForMethodWithName:(NSString *)methodName usingBlock:(METMethodStub)block;
- (id)callMethodWithName:(NSString *)methodName parameters:(NSArray *)parameters completionHandler:(METMethodCompletionHandler)completionHandler;
- (id)callMethodWithName:(NSString *)methodName parameters:(NSArray *)parameters;

@property (assign, nonatomic, readonly, getter=isLoggingIn) BOOL loggingIn;
@property (copy, nonatomic, readonly) NSString *userID;

- (void)loginWithMethodName:(NSString *)methodName parameters:(NSArray *)parameters completionHandler:(METLogInCompletionHandler)completionHandler;
- (void)logoutWithCompletionHandler:(METLogOutCompletionHandler)completionHandler;

@end

@protocol METDDPClientDelegate <NSObject>

@optional

// General info and connection status
- (void)clientWillConnect:(METDDPClient *)client;
- (void)clientWillDisconnect:(METDDPClient *)client;
- (void)clientDidEstablishConnection:(METDDPClient *)client;
- (void)client:(METDDPClient *)client didFailWithError:(NSError *)error;

// Login
- (void)client:(METDDPClient *)client willLoginWithMethodName:(NSString *)methodName parameters:(NSArray *)parameters;
- (void)client:(METDDPClient *)client didSucceedLoginToAccount:(METAccount *)account;
- (void)client:(METDDPClient *)client didFailLoginWithWithError:(NSError *)error;

// Logout
- (void)clientWillLogout:(METDDPClient *)client;
- (void)clientDidLogout:(METDDPClient *)client;

// Subscriptions
- (void)client:(METDDPClient *)client willSubscribe:(METSubscription *)subscription;
- (void)client:(METDDPClient *)client willUnsubscribe:(METSubscription *)subscription;
- (void)client:(METDDPClient *)client didReceiveError:(NSError *)error forSubscription:(METSubscription *)subscription;
- (void)client:(METDDPClient *)client didReceiveReadyForSubscription:(METSubscription *)subscription;

// RPC Calls
- (void)client:(METDDPClient *)client willCallMethod:(METMethodInvocation *)methodInvocation;
- (void)client:(METDDPClient *)client didReceiveResult:(id)result error:(NSError *)error forMethod:(METMethodInvocation *)methodInvocation;
- (void)client:(METDDPClient *)client didReceiveUpdatesForMethod:(METMethodInvocation *)methodInvocation;

// More Advanced: Raw access to DDP messages on the wire
// Shouldn't have to really ever use this for anything more than logging and debugging
- (void)client:(METDDPClient *)client willSendDDPMessage:(NSDictionary *)message;
- (void)client:(METDDPClient *)client didReceiveDDPMessage:(NSDictionary *)message;

@end
