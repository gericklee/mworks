//
//  ViewController.m
//  MWServer-iOS
//
//  Created by Christopher Stawarz on 2/17/17.
//
//

#import "ViewController.h"

#import "AppDelegate.h"


@implementation ViewController


- (BOOL)shouldAutorotate {
    return NO;
}


- (void)viewWillAppear:(BOOL)animated {
    AppDelegate *appDelegate = (AppDelegate *)(UIApplication.sharedApplication.delegate);
    self.listeningAddress.text = appDelegate.listeningAddress;
    self.listeningPort.text = appDelegate.listeningPort.stringValue;
    
    [super viewWillAppear:animated];
}


- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    
    AppDelegate *appDelegate = (AppDelegate *)(UIApplication.sharedApplication.delegate);
    if (appDelegate.alert) {
        [self presentViewController:appDelegate.alert animated:YES completion:nil];
    }
}


@end



























