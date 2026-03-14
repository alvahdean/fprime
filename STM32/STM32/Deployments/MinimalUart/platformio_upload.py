from pathlib import Path
import shutil
import subprocess
import time

Import("env")


def _resolve_artifact(option_name: str) -> Path:
    project_dir = Path(env["PROJECT_DIR"])
    artifact = project_dir / env.GetProjectOption(option_name)
    if not artifact.is_file():
        raise FileNotFoundError(
            f"Expected F' artifact at {artifact}. Build with "
            f"'fprime-util build --build-cache build-minimaluart-image --target STM32MinimalUart' first."
        )
    return artifact


def _upload_action(source, target, env):
    elf = _resolve_artifact("custom_fprime_elf")
    interface_cfg = env.GetProjectOption("custom_openocd_interface")
    target_cfg = env.GetProjectOption("custom_openocd_target")
    openocd_dir = env.PioPlatform().get_package_dir("tool-openocd")
    if not openocd_dir:
        raise RuntimeError("PlatformIO tool-openocd package is not available for this environment.")

    openocd = Path(openocd_dir) / "bin" / "openocd"
    if not openocd.is_file():
        raise FileNotFoundError(f"Unable to locate openocd binary under {openocd_dir}")

    cmd = [
        str(openocd),
        "-f",
        interface_cfg,
        "-f",
        target_cfg,
        "-c",
        f"program {elf} verify reset exit",
    ]
    return env.Execute(" ".join(f'\"{part}\"' if " " in part else part for part in cmd))


def _validate_action(source, target, env):
    _resolve_artifact("custom_fprime_bin")
    _resolve_artifact("custom_fprime_elf")
    return 0


def _upload_dfu_action(source, target, env):
    bin_path = _resolve_artifact("custom_fprime_bin")
    touch_port = env.GetProjectOption("custom_touch_port")
    vidpid = env.GetProjectOption("custom_dfu_vidpid")
    dfu_address = env.GetProjectOption("custom_dfu_address")
    dfu_util = shutil.which("dfu-util")
    if not dfu_util:
        raise RuntimeError("dfu-util was not found on PATH.")

    touch = subprocess.run(
        ["stty", "-F", touch_port, "1200", "hupcl"],
        capture_output=True,
        text=True,
        check=False,
    )
    if touch.returncode != 0:
        message = touch.stderr.strip() or touch.stdout.strip() or "unknown error"
        raise RuntimeError(
            f"Unable to switch {touch_port} into bootloader mode via 1200-baud touch: {message}"
        )

    time.sleep(3)
    cmd = [
        dfu_util,
        "-d",
        vidpid,
        "-a",
        "0",
        "-s",
        f"{dfu_address}:leave",
        "-D",
        str(bin_path),
    ]
    return env.Execute(" ".join(f'\"{part}\"' if " " in part else part for part in cmd))


env.AddCustomTarget(
    name="checkfprime",
    dependencies=None,
    actions=[_validate_action],
    title="Check Fprime Artifacts",
    description="Verify the prebuilt F' firmware artifacts exist",
)

env.AddCustomTarget(
    name="uploadfprime",
    dependencies=None,
    actions=[_upload_action],
    title="Upload Fprime Firmware",
    description="Flash the prebuilt F' firmware image with PlatformIO OpenOCD",
)

env.AddCustomTarget(
    name="uploadfprime_dfu",
    dependencies=None,
    actions=[_upload_dfu_action],
    title="Upload Fprime Firmware (DFU)",
    description="Flash the prebuilt F' firmware image with dfu-util after 1200-baud touch",
)
