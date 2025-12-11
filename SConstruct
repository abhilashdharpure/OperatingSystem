# SConstruct

from pathlib import Path
from SCons.Variables import *
from SCons.Environment import *
from SCons.Node import *
from build_scripts.phony_targets import PhonyTargets
from build_scripts.utility import ParseSize, RemoveSuffix

VARS = Variables('build_scripts/config.py', ARGUMENTS)
VARS.AddVariables(
    EnumVariable("config",
                 help="Build configuration",
                 default="debug",
                 allowed_values=("debug", "release")),

    EnumVariable("arch", 
                 help="Target architecture", 
                 default="i686",
                 allowed_values=("i686")),

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
         help="The size of the image, will be rounded up to the nearest multiple of 512. " +
              "You can use suffixes (k/m/g). " +
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

HOST_ENVIRONMENT = Environment(variables=VARS,
    ENV = os.environ,
    AS = 'nasm',
    CFLAGS = ['-std=c99'],
    CXXFLAGS = ['-std=c++17'],
    CCFLAGS = ['-g'],
    STRIP = 'strip',
)

HOST_ENVIRONMENT.Append(
    PROJECTDIR = HOST_ENVIRONMENT.Dir('.').srcnode()
)

if HOST_ENVIRONMENT['config'] == 'debug':
    HOST_ENVIRONMENT.Append(CCFLAGS = ['-O0'])
else:
    HOST_ENVIRONMENT.Append(CCFLAGS = ['-O3'])

if HOST_ENVIRONMENT['imageType'] == 'floppy':
    HOST_ENVIRONMENT['imageFS'] = 'fat12'

HOST_ENVIRONMENT.Replace(ASCOMSTR        = "Assembling [$SOURCE]",
                         CCCOMSTR        = "Compiling  [$SOURCE]",
                         CXXCOMSTR       = "Compiling  [$SOURCE]",
                         FORTRANPPCOMSTR = "Compiling  [$SOURCE]",
                         FORTRANCOMSTR   = "Compiling  [$SOURCE]",
                         SHCCCOMSTR      = "Compiling  [$SOURCE]",
                         SHCXXCOMSTR     = "Compiling  [$SOURCE]",
                         LINKCOMSTR      = "Linking    [$TARGET]",
                         SHLINKCOMSTR    = "Linking    [$TARGET]",
                         INSTALLSTR      = "Installing [$TARGET]",
                         ARCOMSTR        = "Archiving  [$TARGET]",
                         RANLIBCOMSTR    = "Ranlib     [$TARGET]")


#
# ***  Target environment ***
#

platform_prefix = ''
if HOST_ENVIRONMENT['arch'] == 'i686':
    platform_prefix = 'i686-elf-'

toolchainDir = Path(HOST_ENVIRONMENT['toolchain'], RemoveSuffix(platform_prefix, '-')).resolve()
toolchainBin = Path(toolchainDir, 'bin')
toolchainGccLibs = Path(toolchainDir, 'lib', 'gcc', RemoveSuffix(platform_prefix, '-'), DEPS['gcc'])

TARGET_ENVIRONMENT = HOST_ENVIRONMENT.Clone(
    AR = f'{platform_prefix}ar',
    CC = f'{platform_prefix}gcc',
    CXX = f'{platform_prefix}g++',
    LD = f'{platform_prefix}g++',
    RANLIB = f'{platform_prefix}ranlib',
    STRIP = f'{platform_prefix}strip',

    # toolchain
    TOOLCHAIN_PREFIX = str(toolchainDir),
    TOOLCHAIN_LIBGCC = str(toolchainGccLibs),
    BINUTILS_URL = f'https://ftp.gnu.org/gnu/binutils/binutils-{DEPS["binutils"]}.tar.xz',
    GCC_URL = f'https://ftp.gnu.org/gnu/gcc/gcc-{DEPS["gcc"]}/gcc-{DEPS["gcc"]}.tar.xz',
)


TARGET_ENVIRONMENT.Append(
    ASFLAGS = [
        '-f', 'elf',
        '-g'
    ],
    CCFLAGS = [
        '-ffreestanding',
        '-nostdlib'
    ],
    CXXFLAGS = [
        '-fno-exceptions',
        '-fno-rtti',
    ],
    LINKFLAGS = [
        '-nostdlib'
    ],
    LIBS = ['gcc'],
    LIBPATH = [ str(toolchainGccLibs) ],
)

TARGET_ENVIRONMENT['ENV']['PATH'] += os.pathsep + str(toolchainBin)

Help(VARS.GenerateHelpText(HOST_ENVIRONMENT))
Export('HOST_ENVIRONMENT')
Export('TARGET_ENVIRONMENT')

variantDir = 'build/{0}_{1}'.format(TARGET_ENVIRONMENT['arch'], TARGET_ENVIRONMENT['config'])
variantDirStage1 = variantDir + '/stage1_{0}'.format(TARGET_ENVIRONMENT['imageFS'])





#########################################################################################################################
# SConscript('src/libs/core/SConscript', variant_dir=variantDir + '/libs/core', duplicate=0)
# SConscript('src/bootloader/stage1/SConscript', variant_dir=variantDirStage1, duplicate=0)
# SConscript('src/bootloader/stage2/SConscript', variant_dir=variantDir + '/stage2', duplicate=0)
# SConscript('src/kernel/SConscript', variant_dir=variantDir + '/kernel', duplicate=0)
# SConscript('src/luma-compositor/SConscript', variant_dir=variantDir + '/user', duplicate=0)
# SConscript('image/SConscript', variant_dir=variantDir, duplicate=0)

# Import('image')
# Default(image)

# # Phony targets
# PhonyTargets(HOST_ENVIRONMENT, 
#              run=['./scripts/run.sh', HOST_ENVIRONMENT['imageType'], image[0].path],
#              debug=['./scripts/debug.sh', HOST_ENVIRONMENT['imageType'], image[0].path],
#              bochs=['./scripts/bochs.sh', HOST_ENVIRONMENT['imageType'], image[0].path],
#              toolchain=['./scripts/setup_toolchain.sh', HOST_ENVIRONMENT['toolchain']])

# Depends('run', image)
# Depends('debug', image)
# Depends('bochs', image)

#########################################################################################################################



# -------------------------------------------------------
# Copy user binary into the rootfs BEFORE image is built
# -------------------------------------------------------
import os, shutil

luma_compositor_binary = TARGET_ENVIRONMENT.Dir(variantDir + '/user').File('luma-compositor')
rootfs_bin_dir = 'image/root/bin'

def copy_user_binary_action(target, source, env):
    os.makedirs(rootfs_bin_dir, exist_ok=True)
    src = str(source[0])
    dst = os.path.join(rootfs_bin_dir, "init")
    print(f"[Copy] {src} → {dst}")
    shutil.copy2(src, dst)
    os.chmod(dst, 0o755)

copy_user_binary = TARGET_ENVIRONMENT.Command(
    target='copy_user_binary_done',
    source=luma_compositor_binary,
    action=copy_user_binary_action
)

Export('copy_user_binary')

# # -------------------------------------------------------
# # FAT32 Image creation builder (NEW IMAGE)
# # -------------------------------------------------------
# def build_fat32_image_action(target, source, env):
#     img = str(target[0])
#     script = "./scripts/create_fat32_image.sh"
#     print(f"[FAT32] Creating image: {img}")
#     os.system(f"bash {script} {img}")
#     return 0

# image = TARGET_ENVIRONMENT.Command(
#     target=variantDir + "/image.img",
#     source='copy_user_binary_done',
#     action=build_fat32_image_action
# )

# AlwaysBuild(image)
# Export('image')

# -------------------------------------------------------
# Kernel / bootloader / user compilation
# -------------------------------------------------------
SConscript('src/libs/core/SConscript', variant_dir=variantDir + '/libs/core', duplicate=0)
SConscript('src/bootloader/stage1/SConscript', variant_dir=variantDirStage1, duplicate=0)
SConscript('src/bootloader/stage2/SConscript', variant_dir=variantDir + '/stage2', duplicate=0)
SConscript('src/kernel/SConscript', variant_dir=variantDir + '/kernel', duplicate=0)
SConscript('src/luma-compositor/SConscript', variant_dir=variantDir + '/user', duplicate=0)
SConscript('image/SConscript', variant_dir=variantDir, duplicate=0)

Import('image')


# -------------------------------------------------------
# DO NOT import or use the old "image" target anymore!
# -------------------------------------------------------

# Use the new FAT32 image for default build target
Default(image)

# # Ensure FAT32 image rebuilds when init is updated
# # Depends(image, copy_user_binary)

# -------------------------------------------------------
# Run / Debug / Bochs use new image
# -------------------------------------------------------
from build_scripts.phony_targets import PhonyTargets
PhonyTargets(
    HOST_ENVIRONMENT,
    run=['./scripts/run.sh', HOST_ENVIRONMENT['imageType'], image[0].path],
    debug=['./scripts/debug.sh', HOST_ENVIRONMENT['imageType'], image[0].path],
    bochs=['./scripts/bochs.sh', HOST_ENVIRONMENT['imageType'], image[0].path],
    toolchain=['./scripts/setup_toolchain.sh', HOST_ENVIRONMENT['toolchain']])

Depends('run', image)
Depends('debug', image)
Depends('bochs', image)

