#!/bin/sh
# flat sh-only build, no make/kconfig. include/autoconf.h is pre-generated
# and checked in (from configs/win9x_defconfig via kconfig, done once, see
# configs/README) - if you change the config, regenerate it, don't hand-edit.
set -e
TOP="$(CDPATH= cd -- "$(dirname "$0")" && pwd)"
cd "$TOP"

# used by scripts/embedded_scripts ($srctree/applets/busybox.mkscripts) -
# a plain shell var read from the environment, not kconfig-related.
srctree="$TOP"
export srctree

test -f include/autoconf.h || {
	echo "include/autoconf.h missing - it should be checked into the repo." >&2
	exit 1
}

# applets.h/usage.h are real compile-time headers (used by applets/usage.c
# etc, not just kconfig) - generated from //config:/`//kbuild:`-annotated
# comments in each dir's .c files. Pure sed/shell, no compilation involved.
./scripts/gen_build_files.sh "$TOP" "$TOP" >/dev/null

CROSS_COMPILE=${CROSS_COMPILE:-i686-w64-mingw32-}
CC="${CROSS_COMPILE}gcc"

WINDRES="${CROSS_COMPILE}windres"
if ! command -v "$WINDRES"; then
	WINDRES="windres"
	command -v "$WINDRES"
fi

STRIP="${CROSS_COMPILE}strip"
if ! command -v "$STRIP"; then
	STRIP="strip"
	command -v "$STRIP"
fi

HOSTCC=${HOSTCC:-gcc}
export HOSTCC

host_build() {
	# some HOSTCC toolchains silently append .exe to -o names; fix
	# up the name after instead of guessing behavior up front.
	out="$1"; shift
	$HOSTCC -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer "$@" -o "$out"
	test -f "$out" || { test -f "$out.exe" && mv -f "$out.exe" "$out"; }
}

CFLAGS='-std=gnu99 -Iinclude -Ilibbb -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_TIME_BITS=64 -DMINGW_VER="" -march=i486 -mtune=i386 -mpreferred-stack-boundary=2 -malign-data=abi -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wunused-function -Wunused-value -Wmissing-prototypes -Wmissing-declarations -Wno-format-security -Wdeclaration-after-statement -Wold-style-definition -finline-limit=0 -fno-builtin-strlen -fomit-frame-pointer -ffunction-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-builtin-printf -Oz -Iwin32 -DHAVE_STRING_H=1 -DHAVE_CONFIG_H=0 -fno-builtin-stpcpy -fno-builtin-stpncpy -fno-ident -fno-builtin-strndup -fno-builtin-memmove'

# --- generated headers (all via the project's own sh scripts, no make) ---
echo '#define BB_VER "1.39.0.git"' > include/BB_VER.h
./scripts/mkconfigs include/bbconfigopts.h include/bbconfigopts_bz2.h
./scripts/generate_BUFSIZ.sh include/common_bufsiz.h
./scripts/embedded_scripts include/embedded_scripts.h "$TOP/embed" "$TOP/applets_sh"

host_build applets/usage -Iinclude -Iinclude applets/usage.c
./applets/usage_compressed include/usage_compressed.h applets
host_build applets/applet_tables applets/applet_tables.c
./applets/applet_tables include/applet_tables.h include/NUM_APPLETS.h
host_build applets/usage_pod -Iinclude -Iinclude applets/usage_pod.c

. ./vars.sh

# --- compile every needed .c (list captured from the config's build) ---
OBJS=
for src in \
	applets/applets.c \
	archival/ar.c \
	archival/bbunzip.c \
	archival/bzip2.c \
	archival/chksum_and_xwrite_tar_header.c \
	archival/cpio.c \
	archival/dpkg.c \
	archival/dpkg_deb.c \
	archival/gzip.c \
	archival/libarchive/common.c \
	archival/libarchive/data_align.c \
	archival/libarchive/data_extract_all.c \
	archival/libarchive/data_extract_to_stdout.c \
	archival/libarchive/data_skip.c \
	archival/libarchive/decompress_bunzip2.c \
	archival/libarchive/decompress_gunzip.c \
	archival/libarchive/decompress_uncompress.c \
	archival/libarchive/decompress_unlzma.c \
	archival/libarchive/decompress_unxz.c \
	archival/libarchive/filter_accept_all.c \
	archival/libarchive/filter_accept_list.c \
	archival/libarchive/filter_accept_list_reassign.c \
	archival/libarchive/filter_accept_reject_list.c \
	archival/libarchive/find_list_entry.c \
	archival/libarchive/get_header_ar.c \
	archival/libarchive/get_header_cpio.c \
	archival/libarchive/get_header_tar.c \
	archival/libarchive/get_header_tar_bz2.c \
	archival/libarchive/get_header_tar_gz.c \
	archival/libarchive/get_header_tar_lzma.c \
	archival/libarchive/get_header_tar_xz.c \
	archival/libarchive/header_list.c \
	archival/libarchive/header_skip.c \
	archival/libarchive/header_verbose_list.c \
	archival/libarchive/init_handle.c \
	archival/libarchive/lzo1x_1.c \
	archival/libarchive/lzo1x_1o.c \
	archival/libarchive/lzo1x_d.c \
	archival/libarchive/open_transformer.c \
	archival/libarchive/seek_by_jump.c \
	archival/libarchive/seek_by_read.c \
	archival/libarchive/unpack_ar_archive.c \
	archival/libarchive/unsafe_prefix.c \
	archival/libarchive/unsafe_symlink_target.c \
	archival/lzop.c \
	archival/rpm.c \
	archival/tar.c \
	archival/unzip.c \
	console-tools/clear.c \
	console-tools/reset.c \
	coreutils/basename.c \
	coreutils/cat.c \
	coreutils/chmod.c \
	coreutils/cksum.c \
	coreutils/comm.c \
	coreutils/cp.c \
	coreutils/cut.c \
	coreutils/date.c \
	coreutils/dd.c \
	coreutils/df.c \
	coreutils/dirname.c \
	coreutils/dos2unix.c \
	coreutils/du.c \
	coreutils/echo.c \
	coreutils/env.c \
	coreutils/expand.c \
	coreutils/expr.c \
	coreutils/factor.c \
	coreutils/false.c \
	coreutils/fold.c \
	coreutils/head.c \
	coreutils/id.c \
	coreutils/install.c \
	coreutils/join.c \
	coreutils/libcoreutils/cp_mv_stat.c \
	coreutils/link.c \
	coreutils/ln.c \
	coreutils/logname.c \
	coreutils/ls.c \
	coreutils/md5_sha1_sum.c \
	coreutils/mkdir.c \
	coreutils/mktemp.c \
	coreutils/mv.c \
	coreutils/nl.c \
	coreutils/nproc.c \
	coreutils/od.c \
	coreutils/paste.c \
	coreutils/printenv.c \
	coreutils/printf.c \
	coreutils/pwd.c \
	coreutils/readlink.c \
	coreutils/realpath.c \
	coreutils/rm.c \
	coreutils/rmdir.c \
	coreutils/seq.c \
	coreutils/shred.c \
	coreutils/shuf.c \
	coreutils/sleep.c \
	coreutils/sort.c \
	coreutils/split.c \
	coreutils/stat.c \
	coreutils/stty.c \
	coreutils/sum.c \
	coreutils/sync.c \
	coreutils/tac.c \
	coreutils/tail.c \
	coreutils/tee.c \
	coreutils/test.c \
	coreutils/test_ptr_hack.c \
	coreutils/timeout.c \
	coreutils/touch.c \
	coreutils/tr.c \
	coreutils/true.c \
	coreutils/truncate.c \
	coreutils/tsort.c \
	coreutils/uname.c \
	coreutils/uniq.c \
	coreutils/unlink.c \
	coreutils/usleep.c \
	coreutils/uudecode.c \
	coreutils/uuencode.c \
	coreutils/wc.c \
	coreutils/whoami.c \
	coreutils/yes.c \
	debianutils/pipe_progress.c \
	debianutils/which.c \
	e2fsprogs/chattr.c \
	e2fsprogs/e2fs_lib.c \
	e2fsprogs/lsattr.c \
	editors/awk.c \
	editors/cmp.c \
	editors/diff.c \
	editors/ed.c \
	editors/patch.c \
	editors/sed.c \
	editors/vi.c \
	findutils/find.c \
	findutils/grep.c \
	findutils/xargs.c \
	libbb/appletlib.c \
	libbb/ask_confirmation.c \
	libbb/auto_string.c \
	libbb/bb_bswap_64.c \
	libbb/bb_cat.c \
	libbb/bb_do_delay.c \
	libbb/bb_get_servport_by_name.c \
	libbb/bb_getgroups.c \
	libbb/bb_getsockname.c \
	libbb/bb_pwd.c \
	libbb/bb_qsort.c \
	libbb/bb_strtonum.c \
	libbb/bitops.c \
	libbb/c_escape.c \
	libbb/chomp.c \
	libbb/common_bufsiz.c \
	libbb/compare_string_array.c \
	libbb/concat_path_file.c \
	libbb/concat_subpath_file.c \
	libbb/const_hack.c \
	libbb/copy_file.c \
	libbb/copyfd.c \
	libbb/crc32.c \
	libbb/default_error_retval.c \
	libbb/dump.c \
	libbb/duration.c \
	libbb/endofname.c \
	libbb/executable.c \
	libbb/fclose_nonstdin.c \
	libbb/fflush_stdout_and_exit.c \
	libbb/fgets_str.c \
	libbb/find_mount_point.c \
	libbb/find_pid_by_name.c \
	libbb/full_write.c \
	libbb/get_last_path_component.c \
	libbb/get_line_from_file.c \
	libbb/get_shell_name.c \
	libbb/getopt32.c \
	libbb/getopt_allopts.c \
	libbb/hash_hmac.c \
	libbb/hash_md5_sha.c \
	libbb/hash_sha256_block.c \
	libbb/herror_msg.c \
	libbb/human_readable.c \
	libbb/inode_hash.c \
	libbb/isdirectory.c \
	libbb/isqrt.c \
	libbb/iterate_on_dir.c \
	libbb/last_char_is.c \
	libbb/lineedit.c \
	libbb/lineedit_ptr_hack.c \
	libbb/llist.c \
	libbb/make_directory.c \
	libbb/messages.c \
	libbb/missing_syscalls.c \
	libbb/mode_string.c \
	libbb/nuke_str.c \
	libbb/parse_config.c \
	libbb/parse_mode.c \
	libbb/percent_decode.c \
	libbb/perror_msg.c \
	libbb/perror_nomsg_and_die.c \
	libbb/platform.c \
	libbb/popcnt.c \
	libbb/print_numbered_lines.c \
	libbb/printable.c \
	libbb/printable_string.c \
	libbb/process_escape_sequence.c \
	libbb/procps.c \
	libbb/progress.c \
	libbb/ptr_to_globals.c \
	libbb/read.c \
	libbb/read_key.c \
	libbb/read_printf.c \
	libbb/recursive_action.c \
	libbb/remove_file.c \
	libbb/replace.c \
	libbb/run_shell.c \
	libbb/safe_gethostname.c \
	libbb/safe_poll.c \
	libbb/safe_strncpy.c \
	libbb/safe_write.c \
	libbb/securetty.c \
	libbb/simplify_path.c \
	libbb/single_argv.c \
	libbb/skip_whitespace.c \
	libbb/str_tolower.c \
	libbb/strrstr.c \
	libbb/sysconf.c \
	libbb/time.c \
	libbb/trim.c \
	libbb/u_signal_names.c \
	libbb/ubi.c \
	libbb/uuencode.c \
	libbb/verror_msg.c \
	libbb/vfork_daemon_rexec.c \
	libbb/warn_ignoring_args.c \
	libbb/wfopen.c \
	libbb/wfopen_input.c \
	libbb/xatonum.c \
	libbb/xconnect.c \
	libbb/xfunc_die.c \
	libbb/xfuncs.c \
	libbb/xfuncs_printf.c \
	libbb/xgetcwd.c \
	libbb/xreadlink.c \
	libbb/xrealloc_vector.c \
	libbb/xregcomp.c \
	libpwdgrp/uidgid_get.c \
	loginutils/suw32.c \
	miscutils/ascii.c \
	miscutils/bc.c \
	miscutils/crond.c \
	miscutils/crontab.c \
	miscutils/iconv.c \
	miscutils/jn.c \
	miscutils/less.c \
	miscutils/make.c \
	miscutils/man.c \
	miscutils/strings.c \
	miscutils/time.c \
	miscutils/ts.c \
	miscutils/ttysize.c \
	networking/ftpgetput.c \
	networking/httpd.c \
	networking/ipcalc.c \
	networking/nc.c \
	networking/parse_pasv_epsv.c \
	networking/ssl_client.c \
	networking/tls.c \
	networking/tls_aes.c \
	networking/tls_aesgcm.c \
	networking/tls_fe.c \
	networking/tls_pstm.c \
	networking/tls_pstm_montgomery_reduce.c \
	networking/tls_pstm_mul_comba.c \
	networking/tls_pstm_sqr_comba.c \
	networking/tls_rsa.c \
	networking/tls_sp_c32.c \
	networking/wget.c \
	networking/whois.c \
	procps/free.c \
	procps/kill.c \
	procps/pgrep.c \
	procps/pidof.c \
	procps/ps.c \
	procps/uptime.c \
	procps/watch.c \
	shell/ash.c \
	shell/ash_ptr_hack.c \
	shell/math.c \
	shell/random.c \
	shell/shell_common.c \
	util-linux/cal.c \
	util-linux/flock.c \
	util-linux/getopt.c \
	util-linux/hexdump.c \
	util-linux/hexdump_xxd.c \
	util-linux/rev.c \
	util-linux/uuidgen.c \
	win32/actype.c \
	win32/dirent.c \
	win32/dirname.c \
	win32/env.c \
	win32/flock.c \
	win32/fnmatch.c \
	win32/fnmatch2.c \
	win32/fsync.c \
	win32/glob.c \
	win32/inet_pton.c \
	win32/ioctl.c \
	win32/mingw.c \
	win32/mntent.c \
	win32/net.c \
	win32/poll.c \
	win32/popen.c \
	win32/process.c \
	win32/regex.c \
	win32/select.c \
	win32/statfs.c \
	win32/strndup.c \
	win32/strptime.c \
	win32/strverscmp.c \
	win32/system.c \
	win32/termios.c \
	win32/timegm.c \
	win32/uname.c \
	win32/winansi.c \
; do
	obj="${src%.c}.o"
	base=$(basename "$src" .c)
	echo "CC $obj"
	$CC $CFLAGS -DKBUILD_BASENAME="\"$base\"" -DKBUILD_MODNAME="\"$base\"" -c -o "$obj" "$src"
	OBJS="$OBJS $obj"
done

concurrent_wait

echo "WINDRES win32/resources/resources.o"
$WINDRES -D"BB_VER=1.39.0.git" -D"BB_VERSION=1" -D"BB_PATCHLEVEL=39" -D"BB_SUBLEVEL=0" -D"BB_EXTRAVERSION=0" \
	--include-dir=include --include-dir=win32/resources \
	win32/resources/resources.rc win32/resources/resources.o
OBJS="$OBJS win32/resources/resources.o"

# --- link: reuse the project's own link-negotiation script (already plain sh) ---
./scripts/trylink busybox_unstripped.exe "${CROSS_COMPILE}gcc" "$CFLAGS" "" "$OBJS" "" "ws2_32 bcrypt secur32" ""
./scripts/generate_BUFSIZ.sh --post include/common_bufsiz.h

$STRIP -s --remove-section=.note --remove-section=.comment busybox_unstripped.exe -o busybox.exe
chmod a+x busybox.exe
echo "built: busybox.exe"
