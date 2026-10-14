#include "Window.h"

#import <Cocoa/Cocoa.h>

@interface LampView : NSView
@property(nonatomic) const LampWindow *lamp;
@end

@implementation LampView
- (BOOL)isFlipped
{
    return YES;
}

- (void)drawRect:(NSRect)dirty
{
    const LampWindow &lamp = *self.lamp;
    [[NSColor colorWithCalibratedRed:0.06 green:0.06 blue:0.07 alpha:1.0] setFill];
    NSRectFill(self.bounds);
    const double cw = self.bounds.size.width / lamp.columns;
    const double ch = self.bounds.size.height / lamp.rows;
    const double d = std::min(cw, ch) * 0.55;
    for (unsigned y = 0U; y < lamp.rows; ++y)
    {
        for (unsigned x = 0U; x < lamp.columns; ++x)
        {
            const double b = lamp.pixel(x, y) / 255.0;
            const double cx = (x + 0.5) * cw;
            const double cy = (y + 0.5) * ch;
            if (b > 0.0)
            {
                [[NSColor colorWithCalibratedRed:1.0 green:0.92 blue:0.75 alpha:0.2 * b] setFill];
                NSRectFillUsingOperation(NSMakeRect(cx - 0.75 * d, cy - 0.75 * d, 1.5 * d, 1.5 * d), NSCompositingOperationSourceOver);
            }
            // Off LEDs stay faintly visible, lit ones fade to warm white.
            [[NSColor colorWithCalibratedRed:0.14 + 0.86 * b
                                       green:0.14 + 0.82 * b
                                        blue:0.15 + 0.73 * b
                                       alpha:1.0] setFill];
            NSRectFill(NSMakeRect(cx - d / 2.0, cy - d / 2.0, d, d));
        }
    }
}
@end

@interface LampDelegate : NSObject <NSApplicationDelegate>
@end

@implementation LampDelegate
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app
{
    return YES;
}
@end

int runWindow(const LampWindow &lamp)
{
    @autoreleasepool
    {
        NSApplication *app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];
        LampDelegate *delegate = [[LampDelegate alloc] init];
        [app setDelegate:delegate];

        NSMenu *menubar = [[NSMenu alloc] init];
        NSMenuItem *appItem = [[NSMenuItem alloc] init];
        [menubar addItem:appItem];
        NSMenu *appMenu = [[NSMenu alloc] init];
        [appMenu addItemWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"];
        [appItem setSubmenu:appMenu];
        [app setMainMenu:menubar];

        const NSSize size = NSMakeSize(lamp.widthMm * lamp.scale, lamp.heightMm * lamp.scale);
        NSWindow *window = [[NSWindow alloc]
            initWithContentRect:NSMakeRect(0, 0, size.width, size.height)
                      styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                                NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
                        backing:NSBackingStoreBuffered
                          defer:NO];
        [window setTitle:[NSString stringWithUTF8String:lamp.title.c_str()]];
        [window setContentAspectRatio:size];
        LampView *view = [[LampView alloc] initWithFrame:NSMakeRect(0, 0, size.width, size.height)];
        view.lamp = &lamp;
        [window setContentView:view];
        [window center];
        [window makeKeyAndOrderFront:nil];
        [app activateIgnoringOtherApps:YES];

        NSTimer *timer = [NSTimer timerWithTimeInterval:1.0 / lamp.fps
                                                repeats:YES
                                                  block:^(NSTimer *) {
                                                    if (!lamp.step())
                                                    {
                                                        [NSApp terminate:nil];
                                                        return;
                                                    }
                                                    [view setNeedsDisplay:YES];
                                                  }];
        [[NSRunLoop currentRunLoop] addTimer:timer forMode:NSRunLoopCommonModes];
        [app run];
    }
    return 0;
}

bool savePng(const LampWindow &lamp, const char *path)
{
    @autoreleasepool
    {
        const NSRect frame = NSMakeRect(0, 0, lamp.widthMm * lamp.scale, lamp.heightMm * lamp.scale);
        LampView *view = [[LampView alloc] initWithFrame:frame];
        view.lamp = &lamp;
        NSBitmapImageRep *rep = [view bitmapImageRepForCachingDisplayInRect:frame];
        [view cacheDisplayInRect:frame toBitmapImageRep:rep];
        NSData *png = [rep representationUsingType:NSBitmapImageFileTypePNG properties:@{}];
        return [png writeToFile:[NSString stringWithUTF8String:path] atomically:YES];
    }
}
