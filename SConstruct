# SConstruct

from pathlib import Path
from SCons.Variables import *
from SCons.Environment import *
from SCons.Node import *
from build_scripts.phony_targets import PhonyTargets
from build_scripts.utility import ParseSize, RemoveSuffix
import os

VARS = Variables('build_scripts/config.py', ARGUMENTS)
VARS.AddVariables(
    EnumVariable("config",
                 help="Build configuration",
                 default="debug",
                 allowed_values=("debug", "release")),
    EnumVariable("arch",
                 help="Target architecture",
                 default="x86_64",
                 allowed_values=("i686", "x86_64")),
    EnumVariable("imageType",
                 help="Type of image",
                 default="disk",
                 allowed_values=("floppy", "disk")),
    EnumVariable("imageFS",
                 help="Type of image",
                 default="fat32",
                 allowed_values=("fat12", "fat16", "fat32", "ext2"))
)
VARS.Add("imageSize",
         help="The size of the image, will be rounded up to the nearest multiple of 512. "
              "You can use suffixes (k/m/g). "
              "For floppies, the size is fixed to 1.44MB.",
         default="250m",
         converter=ParseSize)
VARS.Add("toolchain",
         help="Path to toolchain directory.",
         default="toolchain")

DEPS = {
    'binutils': '2.37',
    'gcc': '11.2.0'
}

#
# ***  Host environment ***
#
HOST_ENVIRONMENT = Environment(
    variables=VARS,
    ENV=os.environ,
    AS='nasm',
    CFLAGS=['-std=c99'],
    CXXFLAGS=['-std=c++17'],
    CCFLAGS=['-g'],
    STRIP='strip',
)

HOST_ENVIRONMENT.Append(
    PROJECTDIR=HOST_ENVIRONMENT.Dir('.').srcnode()
)

if HOST_ENVIRONMENT['config'] == 'debug':
    HOST_ENVIRONMENT.Append(CCFLAGS=['-O0'])
else:
    HOST_ENVIRONMENT.Append(CCFLAGS=['-O3'])

if HOST_ENVIRONMENT['imageType'] == 'floppy':
    HOST_ENVIRONMENT['imageFS'] = 'fat12'

HOST_ENVIRONMENT.Replace(
    ASCOMSTR="Assembling [$SOURCE]",
    CCCOMSTR="Compiling  [$SOURCE]",
    CXXCOMSTR="Compiling  [$SOURCE]",
    FORTRANPPCOMSTR="Compiling  [$SOURCE]",
    FORTRANCOMSTR="Compiling  [$SOURCE]",
    SHCCCOMSTR="Compiling  [$SOURCE]",
    SHCXXCOMSTR="Compiling  [$SOURCE]",
    LINKCOMSTR="Linking    [$TARGET]",
    SHLINKCOMSTR="Linking    [$TARGET]",
    INSTALLSTR="Installing [$TARGET]",
    ARCOMSTR="Archiving  [$TARGET]",
    RANLIBCOMSTR="Ranlib     [$TARGET]",
)

#
# ***  Target environment ***
#
platform_prefix = ''
if HOST_ENVIRONMENT['arch'] == 'i686':
    platform_prefix = 'i686-elf-'
elif HOST_ENVIRONMENT['arch'] == 'x86_64':
    platform_prefix = 'x86_64-elf-'

# toolchainDir = Path(HOST_ENVIRONMENT['toolchain'], RemoveSuffix(platform_prefix, '-')).resolve()
# toolchainBin = Path(toolchainDir, 'bin')
# toolchainGccLibs = Path(
#     toolchainDir,
#     'lib', 'gcc',
#     RemoveSuffix(platform_prefix, '-'),
#     DEPS['gcc']
# )
project_root = Path(HOST_ENVIRONMENT['PROJECTDIR'].abspath)

toolchainDir = (project_root /
                HOST_ENVIRONMENT['toolchain'] /
                RemoveSuffix(platform_prefix, '-')).resolve()


toolchainBin = toolchainDir / 'bin'
toolchainGccLibs = toolchainDir / 'lib' / 'gcc' / RemoveSuffix(platform_prefix, '-') / DEPS['gcc']



toolchainBinStr = str(toolchainBin)

# Use absolute paths to toolchain binaries so PATH issues cannot break us
TARGET_ENVIRONMENT = HOST_ENVIRONMENT.Clone(
    AR=str(toolchainBin / f'{platform_prefix}ar'),
    CC=str(toolchainBin / f'{platform_prefix}gcc'),
    CXX=str(toolchainBin / f'{platform_prefix}g++'),
    # Let gcc choose the right ld/emulation; don't override LD
    # LD=str(toolchainBin / f'{platform_prefix}ld'),
    RANLIB=str(toolchainBin / f'{platform_prefix}ranlib'),
    STRIP=str(toolchainBin / f'{platform_prefix}strip'),

    # toolchain metadata
    TOOLCHAIN_PREFIX=str(toolchainDir),
    TOOLCHAIN_LIBGCC=str(toolchainGccLibs),
    BINUTILS_URL=f'https://ftp.gnu.org/gnu/binutils/binutils-{DEPS["binutils"]}.tar.xz',
    GCC_URL=f'https://ftp.gnu.org/gnu/gcc/gcc-{DEPS["gcc"]}/gcc-{DEPS["gcc"]}.tar.xz',
)

# Still add toolchain bin to PATH inside the build env (nice for scripts etc.)
TARGET_ENVIRONMENT.PrependENVPath('PATH', toolchainBinStr)
TARGET_ENVIRONMENT['ENV']['PATH'] += os.pathsep + toolchainBinStr

# Disable default .asm builder on TARGET_ENVIRONMENT
# TARGET_ENVIRONMENT['AS'] = 'nasm'
# TARGET_ENVIRONMENT['ASFLAGS'] = []
# TARGET_ENVIRONMENT['ASCOM'] = ''
# # TARGET_ENVIRONMENT['BUILDERS']['Object'].add_action('.asm', None)

TARGET_ENVIRONMENT['AS'] = 'nasm'
TARGET_ENVIRONMENT['ASFLAGS'] = []
TARGET_ENVIRONMENT['ASCOM'] = '$AS $ASFLAGS -o $TARGET $SOURCE'

# IMPORTANT: re-enable .asm handling on the Object builder
TARGET_ENVIRONMENT['BUILDERS']['Object'].add_action(
    '.asm',
    TARGET_ENVIRONMENT['ASCOM']
)


TARGET_ENVIRONMENT.Append(
    CCFLAGS=[
        '-ffreestanding',
        '-nostdlib',
    ],
    CXXFLAGS=[
        '-fno-exceptions',
        '-fno-rtti',
    ],
    LINKFLAGS=[
        '-nostdlib',
    ],
    LIBPATH=[str(toolchainGccLibs)],
)

if TARGET_ENVIRONMENT['arch'] == 'x86_64':
    # Multiboot2 kernels MUST be 32-bit ELF files
    TARGET_ENVIRONMENT.Append(
        # CCFLAGS=['-m32'],
        LINKFLAGS=['-m32'],
    )
elif TARGET_ENVIRONMENT['arch'] == 'i686':
    pass

Help(VARS.GenerateHelpText(HOST_ENVIRONMENT))
Export('HOST_ENVIRONMENT')
Export('TARGET_ENVIRONMENT')

variantDir = 'build/{0}_{1}'.format(TARGET_ENVIRONMENT['arch'], TARGET_ENVIRONMENT['config'])
variantDirStage1 = variantDir + '/stage1_{0}'.format(TARGET_ENVIRONMENT['imageFS'])

# -------------------------------------------------------
# Kernel / bootloader / user compilation
# -------------------------------------------------------
SConscript('src/libs/core/SConscript', variant_dir=variantDir + '/libs/core', duplicate=0)

if TARGET_ENVIRONMENT['arch'] == 'i686':
    SConscript('src/bootloader/stage1/SConscript', variant_dir=variantDirStage1, duplicate=0)
    SConscript('src/bootloader/stage2/SConscript', variant_dir=variantDir + '/stage2', duplicate=0)

SConscript('src/kernel/SConscript', variant_dir=variantDir + '/kernel', duplicate=0)
SConscript('src/luma-compositor/SConscript', variant_dir=variantDir + '/user', duplicate=0)
SConscript('image/SConscript', variant_dir=variantDir, duplicate=1)

Import('image')

# -------------------------------------------------------
# DO NOT import or use the old "image" target anymore!
# -------------------------------------------------------

# Use the new FAT32 image for default build target
Default(image)

# -------------------------------------------------------
# Run / Debug / Bochs use new image
# -------------------------------------------------------

Import('image', 'root_img')

def run_qemu(target, source, env):
    iso = str(source[0])
    build_dir = Path(iso).parent
    root_img = build_dir / "root.img"

    if not Path(iso).exists():
        print("ISO not found:", iso)
        return 1
    if not root_img.exists():
        print("root.img not found:", root_img)
        return 1

    import subprocess
    subprocess.check_call([
        "qemu-system-x86_64",
        "-M", "pc",
        "-m", "512M",
        "-cdrom", iso,
        "-boot", "d",
        "-drive", f"file={root_img},format=raw,if=ide,index=0",
        "-serial", "mon:stdio",
        "-monitor", "none",
        "-no-reboot",
    ])

    return None


run = HOST_ENVIRONMENT.Alias(
    "run",
    [image, root_img],
    Action(run_qemu, "Running...")
)
HOST_ENVIRONMENT.AlwaysBuild(run)

PhonyTargets(
    HOST_ENVIRONMENT,
    debug=['./scripts/debug.sh', HOST_ENVIRONMENT['imageType'], image[0].path],
    bochs=['./scripts/bochs.sh', HOST_ENVIRONMENT['imageType'], image[0].path],
    toolchain=['./scripts/setup_toolchain.sh', HOST_ENVIRONMENT['toolchain']],
)

Depends('run', image)
Depends('debug', image)
Depends('bochs', image)
