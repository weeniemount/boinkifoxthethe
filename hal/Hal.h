/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set sw=2 ts=8 et ft=cpp : */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_Hal_h
#define mozilla_Hal_h

#include "base/platform_thread.h"
#include "nsTArray.h"
#include "mozilla/hal_sandbox/PHal.h"
#include "mozilla/HalScreenConfiguration.h"
#include "mozilla/HalBatteryInformation.h"
#include "mozilla/HalNetworkInformation.h"
#include "mozilla/HalWakeLockInformation.h"
#include "mozilla/HalTypes.h"
#include "mozilla/MozPromise.h"

#include <cstdint>

/*
 * Hal.h contains the public Hal API.
 *
 * By default, this file defines its functions in the hal namespace, but if
 * MOZ_HAL_NAMESPACE is defined, we'll define our functions in that namespace.
 *
 * This is used by HalImpl.h and HalSandbox.h, which define copies of all the
 * functions here in the hal_impl and hal_sandbox namespaces.
 */

class nsPIDOMWindowInner;

#ifndef MOZ_HAL_NAMESPACE
#  define MOZ_HAL_NAMESPACE hal
#  define MOZ_DEFINED_HAL_NAMESPACE 1
#endif

namespace mozilla {

namespace hal {

class WindowIdentifier;

}  // namespace hal

namespace MOZ_HAL_NAMESPACE {

/**
 * Initializes the HAL. This must be called before any other HAL function.
 */
void Init();

/**
 * Shuts down the HAL. Besides freeing all the used resources this will check
 * that all observers have been properly deregistered and assert if not.
 */
void Shutdown();

/**
 * Turn the default vibrator device on/off per the pattern specified
 * by |pattern|.  Each element in the pattern is the number of
 * milliseconds to turn the vibrator on or off.  The first element in
 * |pattern| is an "on" element, the next is "off", and so on.
 *
 * If |pattern| is empty, any in-progress vibration is canceled.
 *
 * Only an active window within an active tab may call Vibrate; calls
 * from inactive windows and windows on inactive tabs do nothing.
 *
 * If you're calling hal::Vibrate from the outside world, pass an
 * nsIDOMWindow* in place of the WindowIdentifier parameter.
 * The method with WindowIdentifier will be called automatically.
 */
void Vibrate(const nsTArray<uint32_t>& pattern, nsPIDOMWindowInner* aWindow);
void Vibrate(const nsTArray<uint32_t>& pattern, hal::WindowIdentifier&& id);

/**
 * Cancel a vibration started by the content window identified by
 * WindowIdentifier.
 *
 * If the window was the last window to start a vibration, the
 * cancellation request will go through even if the window is not
 * active.
 *
 * As with hal::Vibrate(), if you're calling hal::CancelVibrate from the outside
 * world, pass an nsIDOMWindow*. The method with WindowIdentifier will be called
 * automatically.
 */
void CancelVibrate(nsPIDOMWindowInner* aWindow);
void CancelVibrate(hal::WindowIdentifier&& id);

#define MOZ_DEFINE_HAL_OBSERVER(name_)                             \
  /**                                                              \
   * Inform the backend there is a new |name_| observer.           \
   * @param aObserver The observer that should be added.           \
   */                                                              \
  void Register##name_##Observer(hal::name_##Observer* aObserver); \
  /**                                                              \
   * Inform the backend a |name_| observer unregistered.           \
   * @param aObserver The observer that should be removed.         \
   */                                                              \
  void Unregister##name_##Observer(hal::name_##Observer* aObserver);

MOZ_DEFINE_HAL_OBSERVER(Battery);

/**
 * Returns the current battery information.
 */
void GetCurrentBatteryInformation(hal::BatteryInformation* aBatteryInfo);

/**
 * Notify of a change in the battery state.
 * @param aBatteryInfo The new battery information.
 */
void NotifyBatteryChange(const hal::BatteryInformation& aBatteryInfo);

/**
 * Register an observer for the sensor of given type.
 *
 * The observer will receive data whenever the data generated by the
 * sensor is avaiable.
 */
void RegisterSensorObserver(hal::SensorType aSensor,
                            hal::ISensorObserver* aObserver);

/**
 * Unregister an observer for the sensor of given type.
 */
void UnregisterSensorObserver(hal::SensorType aSensor,
                              hal::ISensorObserver* aObserver);

/**
 * Post a value generated by a sensor.
 *
 * This API is internal to hal; clients shouldn't call it directly.
 */
void NotifySensorChange(const hal::SensorData& aSensorData);

/**
 * Enable sensor notifications from the backend
 *
 * This method is only visible from implementation of sensor manager.
 * Rest of the system should not try this.
 */
void EnableSensorNotifications(hal::SensorType aSensor);

/**
 * Disable sensor notifications from the backend
 *
 * This method is only visible from implementation of sensor manager.
 * Rest of the system should not try this.
 */
void DisableSensorNotifications(hal::SensorType aSensor);

MOZ_DEFINE_HAL_OBSERVER(Network);

/**
 * Returns the current network information.
 */
void GetCurrentNetworkInformation(hal::NetworkInformation* aNetworkInfo);

/**
 * Notify of a change in the network state.
 * @param aNetworkInfo The new network information.
 */
void NotifyNetworkChange(const hal::NetworkInformation& aNetworkInfo);

/**
 * Enable wake lock notifications from the backend.
 *
 * This method is only used by WakeLockObserversManager.
 */
void EnableWakeLockNotifications();

/**
 * Disable wake lock notifications from the backend.
 *
 * This method is only used by WakeLockObserversManager.
 */
void DisableWakeLockNotifications();

MOZ_DEFINE_HAL_OBSERVER(WakeLock);

/**
 * Adjust a wake lock's counts for the current process.
 *
 * @param aTopic        lock topic
 * @param aLockAdjust   to increase or decrease active locks
 * @param aHiddenAdjust to increase or decrease hidden locks
 */
void ModifyWakeLock(const nsAString& aTopic, hal::WakeLockControl aLockAdjust,
                    hal::WakeLockControl aHiddenAdjust);

/**
 * Query the wake lock numbers of aTopic.
 * @param aTopic        lock topic
 * @param aWakeLockInfo wake lock numbers
 */
void GetWakeLockInfo(const nsAString& aTopic,
                     hal::WakeLockInformation* aWakeLockInfo);

/**
 * Notify of a change in the wake lock state.
 * @param aWakeLockInfo The new wake lock information.
 */
void NotifyWakeLockChange(const hal::WakeLockInformation& aWakeLockInfo);

/**
 * Lock the screen orientation to the specific orientation.
 * @return A promise indicating that the screen orientation has been locked.
 */
[[nodiscard]] RefPtr<GenericNonExclusivePromise> LockScreenOrientation(
    const hal::ScreenOrientation& aOrientation);

/**
 * Unlock the screen orientation.
 */
void UnlockScreenOrientation();

/**
 * Set the priority of the given process.
 *
 * Exactly what this does will vary between platforms.  On *nix we might give
 * background processes higher nice values.  On other platforms, we might
 * ignore this call entirely.
 */
void SetProcessPriority(int aPid, hal::ProcessPriority aPriority);

/**
 * Creates a PerformanceHintSession.
 *
 * A PerformanceHintSession represents a workload shared by a group of threads
 * that should be completed in a target duration each cycle.
 *
 * Each cycle, the actual work duration should be reported using
 * PerformanceHintSession::ReportActualWorkDuration(). The system can then
 * adjust the scheduling accordingly in order to achieve the target.
 */
UniquePtr<hal::PerformanceHintSession> CreatePerformanceHintSession(
    const nsTArray<PlatformThreadHandle>& aThreads,
    mozilla::TimeDuration aTargetWorkDuration);

/**
 * Returns information categorizing the CPUs on the system by performance class.
 *
 * Returns Nothing if we are unable to calculate the required information.
 *
 * See the definition of hal::HeterogeneousCpuInfo for more details.
 */
const Maybe<hal::HeterogeneousCpuInfo>& GetHeterogeneousCpuInfo();

/**
 * Perform haptic feedback
 */
void PerformHapticFeedback(int32_t aType);

}  // namespace MOZ_HAL_NAMESPACE
}  // namespace mozilla

#ifdef MOZ_DEFINED_HAL_NAMESPACE
#  undef MOZ_DEFINED_HAL_NAMESPACE
#  undef MOZ_HAL_NAMESPACE
#endif

#endif  // mozilla_Hal_h
