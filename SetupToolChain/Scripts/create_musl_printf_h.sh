#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

PRINTF_H="$SYSROOT/usr/include/printf.h"

echo "========================================"
echo "Creating musl-compatible printf.h"
echo "========================================"
echo
echo "Target : $TARGET"
echo "Sysroot: $SYSROOT"
echo "File   : $PRINTF_H"
echo

mkdir -p "$SYSROOT/usr/include"

# ------------------------------------------------------------
# Backup existing header if present
# ------------------------------------------------------------

if [ -f "$PRINTF_H" ]; then
    BACKUP="$PRINTF_H.backup.$(date +%Y%m%d-%H%M%S)"

    echo "Existing printf.h found."
    echo "Backing up to:"
    echo "  $BACKUP"

    cp -a "$PRINTF_H" "$BACKUP"
fi

# ------------------------------------------------------------
# Create musl-compatible printf.h
# ------------------------------------------------------------

cat > "$PRINTF_H" <<'EOF'
#ifndef _PRINTF_H
#define _PRINTF_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Argument type information used by parse_printf_format().
 *
 * These values follow the GNU printf.h interface because systemd
 * uses the GNU printf argument classification API.
 */

/* Basic argument types. */
#define PA_INT        0
#define PA_CHAR       1
#define PA_WCHAR      2
#define PA_STRING     3
#define PA_WSTRING    4
#define PA_POINTER    5
#define PA_FLOAT      6
#define PA_DOUBLE     7
#define PA_LAST       8

/* Flag bits. */
#define PA_FLAG_MASK          0xff00

#define PA_FLAG_LONG_LONG     (1 << 8)
#define PA_FLAG_LONG_DOUBLE   (1 << 9)
#define PA_FLAG_LONG          (1 << 10)
#define PA_FLAG_PTR           (1 << 11)
#define PA_FLAG_SHORT         (1 << 12)

/*
 * Parse printf format string and return the number of arguments.
 *
 * This is the GNU printf.h API expected by systemd.
 *
 * The implementation is provided separately for the musl target.
 */
size_t parse_printf_format(
        const char *format,
        size_t n,
        int *argtypes);

#ifdef __cplusplus
}
#endif

#endif /* _PRINTF_H */
EOF

echo
echo "Created:"
ls -l "$PRINTF_H"

echo
echo "Checking required definitions..."

grep -n "parse_printf_format" "$PRINTF_H"
grep -n "PA_FLAG_PTR" "$PRINTF_H"
grep -n "PA_INT" "$PRINTF_H"
grep -n "PA_DOUBLE" "$PRINTF_H"

echo
echo "Testing target compiler..."

printf '#include <printf.h>\nint main(void) { return 0; }\n' |
    "$PREFIX/bin/$TARGET-gcc" \
        --sysroot="$SYSROOT" \
        -x c \
        -fsyntax-only -

echo
echo "========================================"
echo "SUCCESS"
echo "========================================"
echo
echo "Musl-compatible printf.h is installed."
echo
echo "Next file to implement:"
echo "  parse_printf_format()"
echo
echo "The header alone will not provide the function implementation."
echo
