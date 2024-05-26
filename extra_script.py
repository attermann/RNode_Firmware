import time
import hashlib
import shutil

Import("env")

#
# Custom targets
#

def target_package(target, source, env):
    print("target_package...")
    print("Platform:", env.GetProjectOption("platform"))
    print("Board:", env.GetProjectOption("board"))
    print("Variant:", env.GetProjectOption("variant"))
    # do some actions
    platform = env.GetProjectOption("platform")
    board = env.GetProjectOption("board")
    firmware_package(env)

env.AddCustomTarget(
    name="package",
    dependencies="$BUILD_DIR/${PROGNAME}.bin",
    actions=[
        target_package
    ],
    title="Package",
    description="Package firmware for delivery"
)

#
# Upload actions
#

def pre_upload(source, target, env):
    print("pre_upload...")
    # do some actions

def post_upload(source, target, env):
    print("post_upload...")
    print("Platform:", env.GetProjectOption("platform"))
    print("Board:", env.GetProjectOption("board"))
    print("Serial port:", env.subst("$UPLOAD_PORT"))
    # do some actions
    platform = env.GetProjectOption("platform")
    board = env.GetProjectOption("board")
    if (platform == "espressif32"):
        time.sleep(5)
        # device provisioning is incomplete and only currently appropriate for 915MHz T-Beam
        #device_provision(env)
        firmware_hash(source, env)
        # firmware pacakaging is incomplete due to missing console image
        #firmware_package(env)
    elif (platform == "nordicnrf52"):
        time.sleep(5)
        # device provisioning is incomplete and only currently appropriate for 915MHz T-Beam
        #device_provision(env)
        #firmware_hash(source, env)
        # firmware pacakaging is incomplete due to missing console image
        #firmware_package(env)

def post_clean(source, target, env):
    print("post_clean...")
    core_dir = env.subst("$CORE_DIR")
    print("core_dir:", core_dir)
    packages_dir = env.subst("$PACKAGES_DIR")
    print("packages_dir:", packages_dir)
    project_dir = env.subst("$PROJECT_DIR")
    print("project_dir:", project_dir)
    #build_dir = env.subst("$BUILD_DIR").get_abspath()
    build_dir = env.subst("$BUILD_DIR")
    print("build_dir:", build_dir)
    build_cache_dir = env.subst("$PLATFORMIO_BUILD_CACHE_DIR")
    print("build_cache_dir:", build_cache_dir)
    workspace_dir = env.subst("$PLATFORMIO_WORKSPACE_DIR")
    print("workspace_dir:", workspace_dir)
    #shutil.rmtree(directory_path)

env.AddPreAction("upload", pre_upload)
env.AddPostAction("upload", post_upload)
env.AddPostAction("clean", post_clean)

def device_provision(env):
    # Device provision
    print("Provisioning device...")
    platform = env.GetProjectOption("platform")
    board = env.GetProjectOption("board")
    if (platform == "espressif32"):
        if (board == "ttgo-t-beam"):
            print("Provisioning t-beam device...")
            env.Execute("rnodeconf --product e0 --model e9 --hwrev 1 --rom " + env.subst("$UPLOAD_PORT"))
    elif (platform == "nordicnrf52"):
        if (board == "wiscore_rak4631"):
            print("Provisioning rak4631 device...")
            env.Execute("rnodeconf --platform 70 --product f0 --model ff --hwrev 1 --rom " + env.subst("$UPLOAD_PORT"))

def firmware_hash(source, env):
    # Firmware hash
    print("Updating firmware hash...")
    source_file = source[0].get_abspath()
    print("source_file:", source_file)
    firmware_data = open(source_file, "rb").read()
    calc_hash = hashlib.sha256(firmware_data[0:-32]).digest()
    part_hash = firmware_data[-32:]
    if calc_hash == part_hash:
        hex_hash = part_hash.hex()
        env.Execute("rnodeconf --firmware-hash " + hex_hash + " " + env.subst("$UPLOAD_PORT"))
    else:
        print("Hash calculation failed!")

def firmware_package(env):
    platform = env.GetProjectOption("platform")
    board = env.GetProjectOption("board")
    # Firmware package
    print("Building firmware package...")
    platform = env.GetProjectOption("platform")
    board = env.GetProjectOption("board")
    print("board:", board)
    variant = env.GetProjectOption("variant")
    print("variant:", variant)
    core_dir = env.subst("$CORE_DIR")
    print("core_dir:", core_dir)
    packages_dir = env.subst("$PACKAGES_DIR")
    print("packages_dir:", packages_dir)
    workspace_dir = env.subst("$WORKSPACE_DIR")
    print("workspace_dir:", workspace_dir)
    project_dir = env.subst("$PROJECT_DIR")
    print("project_dir:", project_dir)
    #build_dir = env.subst("$BUILD_DIR").get_abspath()
    build_dir = env.subst("$BUILD_DIR")
    print("build_dir:", build_dir)
    if (platform == "espressif32"):
        #env.Execute("cp " + packages_dir + "/framework-arduinoespressif32/tools/partitions/boot_app0.bin " + build_dir + "/rnode_firmware_" + variant + ".boot_app0")
        env.Execute("cp ~/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin " + build_dir + "/rnode_firmware_" + variant + ".boot_app0")
    elif (platform == "nordicnrf52"):
        pass
    env.Execute("cp " + project_dir + "/Release/esptool/esptool.py " + build_dir + "/esptool.py")
    env.Execute("cp " + project_dir + "/Release/console_image.bin " + build_dir + "/console_image.bin")
    env.Execute("cp " + build_dir + "/firmware.bin " + build_dir + "/rnode_firmware_" + variant + ".bin")
    env.Execute("cp " + build_dir + "/bootloader.bin " + build_dir + "/rnode_firmware_" + variant + ".bootloader")
    env.Execute("cp " + build_dir + "/partitions.bin " + build_dir + "/rnode_firmware_" + variant + ".partitions")
    env.Execute("rm " + project_dir + "/Release/rnode_firmware_" + variant + ".zip")
    zip_cmd = "zip --move --junk-paths "
    zip_cmd += project_dir + "/Release/rnode_firmware_" + variant + ".zip "
    zip_cmd += build_dir + "/esptool.py "
    zip_cmd += build_dir + "/console_image.bin "
    zip_cmd += build_dir + "/rnode_firmware_" + variant + ".boot_app0 "
    zip_cmd += build_dir + "/rnode_firmware_" + variant + ".bin "
    zip_cmd += build_dir + "/rnode_firmware_" + variant + ".bootloader "
    zip_cmd += build_dir + "/rnode_firmware_" + variant + ".partitions "
    env.Execute(zip_cmd)
