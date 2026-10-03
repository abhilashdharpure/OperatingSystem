#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

COMPAT_H="$SYSROOT/usr/include/systemd-musl-compat.h"

echo "========================================"
echo "Creating systemd musl compatibility header"
echo "========================================"
echo
echo "Target : $TARGET"
echo "Sysroot: $SYSROOT"
echo "File   : $COMPAT_H"
echo

mkdir -p "$SYSROOT/usr/include"

# ------------------------------------------------------------
# Backup existing header if present
# ------------------------------------------------------------

if [ -f "$COMPAT_H" ]; then
    BACKUP="$COMPAT_H.backup.$(date +%Y%m%d-%H%M%S)"

    echo "Existing systemd-musl-compat.h found."
    echo "Backing up to:"
    echo "  $BACKUP"

    cp -a "$COMPAT_H" "$BACKUP"
fi

# ------------------------------------------------------------
# Create systemd musl compatibility header
# ------------------------------------------------------------

cat > "$COMPAT_H" <<'EOF'
#ifndef SYSTEMD_MUSL_COMPAT_H
#define SYSTEMD_MUSL_COMPAT_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/*
 * ------------------------------------------------------------
 * comparison_fn_t
 * ------------------------------------------------------------
 *
 * glibc exposes comparison_fn_t publicly.
 * musl does not define this typedef.
 *
 * systemd uses comparison_fn_t with qsort() and bsearch().
 */

typedef int (*comparison_fn_t)(const void *, const void *);


/*
 * ------------------------------------------------------------
 * strerror_r()
 * ------------------------------------------------------------
 *
 * systemd 257.4 expects the GNU strerror_r() interface:
 *
 *     char *strerror_r(int, char *, size_t);
 *
 * musl provides the POSIX/XSI interface:
 *
 *     int strerror_r(int, char *, size_t);
 *
 * Provide a small compatibility wrapper which converts the
 * musl interface into the pointer-returning interface expected
 * by systemd.
 */

static inline char *systemd_musl_strerror_r(
        int errnum,
        char *buf,
        size_t buflen)
{
        int r;

        /*
         * Parentheses ensure that this call refers to the original
         * musl strerror_r() declaration rather than the compatibility
         * macro defined below.
         */
        r = (strerror_r)(errnum, buf, buflen);

        if (r == 0)
                return buf;

        /*
         * Return the standard error string if musl reports an error.
         */
        return strerror(errnum);
}


/*
 * Redirect systemd's strerror_r() calls to the compatibility
 * wrapper.
 */
#define strerror_r systemd_musl_strerror_r

#endif /* SYSTEMD_MUSL_COMPAT_H */
EOF

echo
echo "Created:"
ls -l "$COMPAT_H"

# ------------------------------------------------------------
# Check required definitions
# ------------------------------------------------------------

echo
echo "Checking required definitions..."

grep -n "typedef int (\*comparison_fn_t)" "$COMPAT_H"
grep -n "systemd_musl_strerror_r" "$COMPAT_H"
grep -n "#define strerror_r" "$COMPAT_H"

# ------------------------------------------------------------
# Test target compiler
# ------------------------------------------------------------

echo
echo "Testing target compiler..."

printf '%s\n' \
    '#include <systemd-musl-compat.h>' \
    'static int compare(const void *a, const void *b)' \
    '{' \
    '    (void)a;' \
    '    (void)b;' \
    '    return 0;' \
    '}' \
    'int main(void)' \
    '{' \
    '    comparison_fn_t fn = compare;' \
    '    char buffer[128];' \
    '    char *message;' \
    '    (void)fn;' \
    '    message = strerror_r(2, buffer, sizeof(buffer));' \
    '    return message != 0 ? 0 : 1;' \
    '}' |
    "$PREFIX/bin/$TARGET-gcc" \
        --sysroot="$SYSROOT" \
        -x c \
        -fsyntax-only -

echo
echo "========================================"
echo "SUCCESS"
echo "========================================"
echo
echo "Systemd musl compatibility header is installed."
echo
echo "Header:"
echo "  $COMPAT_H"
echo
echo "Provides:"
echo "  comparison_fn_t"
echo "  GNU-compatible strerror_r() wrapper"
echo
