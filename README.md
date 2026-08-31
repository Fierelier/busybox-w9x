This is an experimental fork of [busybox-w32](https://github.com/rmyorston/busybox-w32) made to run with Windows 95 (currently untested) and Windows 98. AI has been used.

`ash` and some of the other programs I tested seem to work fine. Use `build.sh` to build the project. `clean.sh` cleans the project. I recommend you use [mingw-lite](https://github.com/redpanda-cpp/mingw-lite). You can build right on Windows 9x if you want, but I recommend you have at least a Pentium 4.

* [Releases (i486) + sources](https://github.com/Fierelier/busybox-w9x/releases)
* [Newest source .zip](https://codeload.github.com/Fierelier/busybox-w9x/zip/refs/heads/master)

**I recommend you install [NANSI](https://www.kegel.com/nansi/)**, to gain some of the required ANSI codes.

### Known issues

- `ls -lh` shows weird file size units
- `less` makes the window go fullscreen with ALT+ENTER disabled (at least under NANSI)
- ...

### Changes in busybox-w9x

- **`win32/winansi.c`** — `winansi_vsnprintf()` no longer trusts the `_vsnprintf(NULL,0,...)` measure call (always returns -1 on old msvcrt, was killing every formatted print in the binary). Uses the real write's own return value instead.
- **`shell/ash.c`** — `cvtnum()` guards against `fmtstr()` returning -1 (prevents signed-to-huge-unsigned `size_t` wraparound). Plus unrelated Win9x fixes: `console_state()`/`hide_console()` lazyload `GetConsoleWindow`, job-control calls `mingw_forget_process()` on child reap, `forkshell` failure path now raises a real error with `GetLastError()`/`errno` instead of a bare message, `SO_PEERCRED`-style euid/egid executable check skipped on mingw (own uid/gid model doesn't support ownership narrowing).
- **`Config.in`** — new `MINGW_TIME32` config option, gates the whole old-msvcrt compat bundle.
- **`include/libbb.h`** — `LL_FMT` picks `"I64"` only when `MINGW_TIME32` is off (so this bundle's own %lld handling isn't double-patched).
- **`include/mingw.h`** — under `MINGW_TIME32`: redirects `time/ctime/gmtime/mktime` and `strtoll/strtoull` to `mingw_*` builtins (classic msvcrt lacks `_time32`-family and `_strtoi64`/`_strtoui64`); unconditional `mingw_ftruncate`, `mingw_getprocessid`/`mingw_forget_process` (GetProcessId is XP+), `freeaddrinfo`/`getnameinfo` macros.
- **`win32/mingw.c`** — implements the above: pure FILETIME/SYSTEMTIME-based time functions (real 64-bit range to year 30828), portable `strtoull`/`strtoll`, Toolhelp32-based PID cache (since `GetProcessId` doesn't exist pre-XP), `mingw_ftruncate`.
- **`win32/net.c`** — `getaddrinfo`/`freeaddrinfo`/`getnameinfo` fall back to `gethostbyname`/manual struct-building when ws2_32 lacks the real functions (pre-2000 Winsock, IPv4-only, no service-name lookup).
- **`win32/mntent.c`** — volume-GUID mount-point enumeration (`FindFirstVolume` etc., Win2000+) becomes a no-op fallback instead of crashing on Win9x.
- **`win32/process.c`** — `spawnve`/`CreateProcess` calls get a forward-slash→backslash conversion pass (old CRT/CreateProcess can't find files otherwise); `IsWow64Process` guarded (XP+); large comment justifying why `spawnve` can't be replaced by raw `CreateProcess` for ash's pipeline fd-passing.
- **`win32/popen.c`** — same backslash conversion for its `CreateProcess` call.
- **`libbb/appletlib.c`** — applet hardlink install falls back to `copy_file()` on `ENOENT`/no-hardlink filesystems (FAT/FAT32).
- **`miscutils/drop.c`**, **`miscutils/inotifyd.c`** — lazyload SAFER-token and `ReadDirectoryChangesW` APIs (both NT-only), die cleanly with "not supported" instead of crashing on missing imports.
- **`build.sh`, `clean.sh`, `vars.sh`, `configs/win9x_defconfig`** — new flat non-kconfig build path and matching defconfig for this target.

Note that the rest of this README contains information concerning the original busybox-w32. **Do not bother the author of the original project with any issues stemming from this fork.**

---

### Status

Things may work for you, or may not.  Things may never work because of huge differences between Linux and Windows.  Or things may work in future, if you report the problem on [GitHub](https://github.com/rmyorston/busybox-w32) or [GitLab](https://gitlab.com/rmyorston/busybox-w32).  If you don't have an account on one of those or you'd prefer to communicate privately you can email [rmy@pobox.com](mailto:rmy@pobox.com).

Additional information is available from the [BusyBox for Windows](https://frippery.org/busybox/index.html) web page.  In particular:

- There are [downloads](https://frippery.org/busybox/index.html#downloads) of precompiled binaries for i686, x86_64 and aarch64.
- Release notes for the [current](https://frippery.org/busybox/release-notes/current.html) and [previous](https://frippery.org/busybox/release-notes/index.html) releases are available.

### Building

You need a MinGW toolchain and a POSIX environment.  I cross-compile on Linux.  On Fedora the following should pull in everything required:

`dnf install gcc make ncurses-devel perl-Pod-Html`

`dnf install mingw64-gcc` (for a 64-bit build)

`dnf install mingw32-gcc` (for a 32-bit build)

On Microsoft Windows you can install [w64devkit](https://github.com/skeeto/w64devkit/releases).  Get the `-i686` variant for a 32-bit build.  Unzip the file and run `w64devkit/w64devkit.exe`.

On either Linux or Windows the commands `make mingw64_defconfig` or `make mingw32_defconfig` will pick up the default configuration.  You can then customize your build with `make menuconfig` or by editing `.config`, if you know what you're doing.

Then just `make`.

See the [Building busybox-w32](https://frippery.org/busybox/build.html) web page for additional information.

### Hints

 - Use forward slashes in paths:  Windows doesn't mind and the shell will be happier.
 - Windows paths are different from Unix ([more detail](https://frippery.org/busybox/paths.html)):
   * Absolute paths: `c:/path` or `//host/share/path`
   * Relative to current directory of other drive: `c:path`
   * Relative to current root (drive or share): `/path`
   * Relative to current directory of current root (drive or share): `path`
 - Handling of users, groups and permissions is totally bogus.  The system only admits to knowing about the current user and employs various heuristics to synthesise uid, gid and permission values.
 - To improve performance, tell Windows Security to ignore BusyBox processes: `Settings -> Update & Security -> Windows Security -> Virus & threat protection -> Manage settings (under Virus & threat protection settings) -> Add or remove exclusions (under Exclusions) -> Add an exclusion -> Process.` Enter the name of your executable and press `Add`.
 - Some crufty old Windows code (Windows XP, cmd.exe) doesn't like forward slashes in environment variables.  The -X shell option prevents busybox-w32 from changing backslashes to forward slashes.  If Windows programs don't run from the shell it's worth trying it.
 - If you want to install 32-bit BusyBox in a system directory on a 64-bit version of Windows you should put it in `C:\Windows\SysWOW64`, not `C:\Windows\System32` as you might expect.  On 64-bit systems the latter is for 64-bit binaries.
 - The system tries to detect the best way to handle the terminal being used.  If this doesn't work you can try setting the environment variable `BB_TERMINAL_MODE=1` to force the use of literal ANSI escapes or `BB_TERMINAL_MODE=0` to emulate them using the Windows console API.
 - busybox-w32 prefers built-in applets to external programs when running commands. This preference can be overridden by setting the environment variable `BB_OVERRIDE_APPLETS` to a space-separated list of applet names. Thus, to use an external `make` in preference to the built-in applet set `BB_OVERRIDE_APPLETS="make"`.
 - Emulations of several Unix-style device files are provided: `/dev/null`, `/dev/tty`, `/dev/zero` and `/dev/urandom`, for example. These can be used as file arguments to applets or in shell redirections. They can't be used as arguments to other programs.
