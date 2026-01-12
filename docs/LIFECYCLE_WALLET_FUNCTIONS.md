# Wallet Lifecycle Functions

This document outlines the key functions modified or introduced in the recent feature work regarding sync pausing, skipping, minimization behavior, and application exit.

## `src/libwalletqt/Wallet.cpp`

### `syncStatusUpdated(quint64 height, quint64 target)`
**Role:** Signals the UI about synchronization progress.
**Modification:**
- Updated to check for `m_stopHeight` during range syncs.
- Calls `skipToTip()` if the range sync target is reached.
- Emits `syncStatus` signal to update the UI.
- **Note:** Contains a `TODO` regarding the necessity of `updateBalance()`.
- Emits `syncStatus` signal to update the UI.
- **Note:** Contains a `TODO` regarding the necessity of `updateBalance()`.

### `initAsync(const QString &daemonAddress, bool trustedDaemon, quint64 searchHeight)`
**Role:** Initializes the wallet asynchronously.
**Details:**
- Sets connection status to `Connecting`.
- Attempts to connect to the daemon.
- On success, if sync is not paused, calls `startRefresh()`.

### `startRefresh()`
**Role:** Enables the background refresh loop.
**Implementation:**
- Sets `m_refreshEnabled = true`.
- Sets `m_refreshNow = true` to force an immediate check in the refresh thread.

### `pauseRefresh()`
**Role:** Disables the background refresh loop.
**Implementation:**
- Sets `m_refreshEnabled = false`.
- Used by `setSyncPaused`, `skipToTip`, and other methods that need to interrupt scanning.

### `startRefreshThread()`
**Role:** The main background loop for wallet synchronization.
**Details:**
- Runs continuously on a separate thread.
- Checks `m_refreshEnabled` every 10 seconds (or immediately if `m_refreshNow` is true).
- Calls `m_walletImpl->refresh()` to fetch blocks.
- Emits `refreshed` signals.
 
### `setSyncPaused(bool paused)`
**Role:** Pauses or resumes the wallet synchronization process.
**Implementation:**
- Sets `m_syncPaused` flag.
- Calls `pauseRefresh()` or `startRefresh()` on the underlying `wallet2` instance.

### `skipToTip()`
**Role:** Fast-forwards the wallet to the current network tip.
**Usage:** Used when the user wants to skip scanning old blocks (e.g. data saver mode).
**Details:**
- Gets current blockchain height.
- Updates refresh height.
- Logs the jump ("Head moving from X to Y").

### `syncDateRange(const QDate &start, const QDate &end)`
**Role:** Syncs the wallet for a specific date range.
**Workflow:**
- Converts dates to block heights using `RestoreHeightLookup`.
- Sets `m_wallet2` refresh height to start date.
- Sets internal `m_stopHeight`.
- Enables `m_rangeSyncActive`.

### `fullSync()`
**Role:** Resets the wallet to scan from the original creation height.
**Details:**
- Retrieves `feather.creation_height` from cache.
- Fallback to current height if creation height is missing (with warning).
- Clears `m_rangeSyncActive`.
- Triggers rescan.

## `src/MainWindow.cpp`

### `onSyncStatus(quint64 height, quint64 target, bool daemonSync)`
**Role:** Updates the status bar with sync progress.
**Modifications:**
- Added detailed logging (escalated to `qWarning` for visibility).
- Improved tooltip to show Wallet Height vs Network Tip.
- Formats "Blocks remaining" and "Approximate size".

### `changeEvent(QEvent *event)`
**Role:** Handles window state changes (minimization).
**Work:**
- **Reverted Attempt:** Initial `QWindow::visibilityChanged` hook was unreliable on Wayland.
- **Final Solution:** Uses `QEvent::ActivationChange`.
- **Implementation:**
    - Checks `isActiveWindow()`.
    - Uses a `QTimer::singleShot` (500ms) to delay the check for `!isExposed()`.
    - Minimizes to tray if the window is truly hidden/not exposed.
    - Logs events with `qWarning` (previously `qInfo`) for debugging Wayland behavior.
    - Handles `lockOnMinimize`.

### `setPausedSyncStatus()`
**Role:** Updates UI when sync is explicitly paused.
**Details:**
- Sets status text to "Sync paused".
- Improved logging (`qWarning`).

## `src/WindowManager.cpp`

### `close()`
**Role:** Handles application shutdown.
**Critical Fix:**
- Reordered shutdown logic to close all windows *before* waiting for the global thread pool.
- Prevents deadlocks where background threads (refreshing wallets) would hang waiting for the thread pool while the UI was still partially active.
- Added recursion guard `m_closing` to prevent infinite loops during tray exit.

### `toggleVisibility()` (Concept)
**Role:** Toggles window show/hide state.
**Context:**
- Integrated into the tray icon left-click handler logic within `DataManager` / `WindowManager` context (specifically `QSystemTrayIcon::activated`).
- Replaced the "Context Menu" on left click default behavior.
