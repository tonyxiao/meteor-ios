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

NS_ASSUME_NONNULL_BEGIN

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

typedef id __nullable (^METMethodStub)(NSArray *parameters);
typedef void (^METMethodCompletionHandler)(id __nullable result, NSError * __nullable error);

typedef void (^METLogInCompletionHandler)(NSError * __nullable error);
typedef void (^METLogOutCompletionHandler)(NSError * __nullable error);

/*!
 A `METDDPClient` object acts as the main entry point into Meteor iOS.
 */
@interface METDDPClient : NSObject

#pragma mark - Initialization
/// @name Initializing a METDDPClient Object

- (instancetype)initWithConnection:(nullable METDDPConnection *)connection account:(nullable METAccount *)account;
- (instancetype)initWithConnection:(nullable METDDPConnection *)connection;
- (instancetype)initWithServerURL:(NSURL *)serverURL account:(nullable METAccount *)account;
- (instancetype)initWithServerURL:(NSURL *)serverURL;

#pragma mark - Delegate
/// @name Managing the Delegate

@property (nullable, weak, nonatomic) id<METDDPClientDelegate> delegate;

#pragma mark - Connection
/// @name Accessing Connection Status

@property (nullable, strong, nonatomic, readonly) NSURL *serverURL;
@property (assign, nonatomic, readonly, getter=isNetworkReachable) BOOL networkReachable;
@property (assign, nonatomic, readonly, getter=isConnected) BOOL connected;
@property (assign, nonatomic, readonly) METDDPConnectionStatus connectionStatus;

/// @name Managing the Connection

- (void)connect;
- (void)disconnect;

#pragma mark - Database
/// @name Accessing the Database

@property (nullable, copy, nonatomic) METAccount *account;
@property (strong, nonatomic, readonly) METDatabase *database;

#pragma mark - Subscriptions
/// @name Managing Subscriptions

- (METSubscription *)addSubscriptionWithName:(NSString *)name;
- (METSubscription *)addSubscriptionWithName:(NSString *)name completionHandler:(nullable METSubscriptionCompletionHandler)completionHandler;
- (METSubscription *)addSubscriptionWithName:(NSString *)name parameters:(nullable NSArray *)parameters;
- (METSubscription *)addSubscriptionWithName:(NSString *)name parameters:(nullable NSArray *)parameters completionHandler:(nullable METSubscriptionCompletionHandler)completionHandler;
- (void)removeSubscription:(METSubscription *)subscription;

#pragma mark - Method Invocations
/// @name Defining Method Stubs

- (void)defineStubForMethodWithName:(NSString *)methodName usingBlock:(METMethodStub)block;

/// @name Performing Method Invocations

- (nullable id)callMethodWithName:(NSString *)methodName parameters:(nullable NSArray *)parameters completionHandler:(nullable METMethodCompletionHandler)completionHandler;
- (nullable id)callMethodWithName:(NSString *)methodName parameters:(nullable NSArray *)parameters;

#pragma mark - Accounts
/// @name Accessing Account Status
- (nullable id)callMethodWithName:(NSString *)methodName parameters:(NSArray *)parameters completionHandler:(METMethodCompletionHandler)completionHandler methodStub:(METMethodStub)stub;
- (nullable id)callMethodWithName:(NSString *)methodName parameters:(NSArray *)parameters completionHandler:(METMethodCompletionHandler)completionHandler;
- (nullable id)callMethodWithName:(NSString *)methodName parameters:(NSArray *)parameters;

@property (assign, nonatomic, readonly, getter=isLoggingIn) BOOL loggingIn;
@property (nullable, copy, nonatomic, readonly) NSString *userID;

/// @name Logging Out

- (void)loginWithMethodName:(NSString *)methodName parameters:(nullable NSArray *)parameters completionHandler:(nullable METLogInCompletionHandler)completionHandler;
- (void)logoutWithCompletionHandler:(nullable METLogOutCompletionHandler)completionHandler;

@end

@protocol METDDPClientDelegate <NSObject>

@optional

- (void)client:(METDDPClient *)client reachabilityStatusDidChange:(BOOL)reachable;

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

NS_ASSUME_NONNULL_END
