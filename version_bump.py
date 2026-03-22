import json
import os

# This script is executed by PlatformIO (SCons) during the build process.
# It automatically increments the patch version in version.json and
# injects the version string as a C++ macro.

try:
    from SCons.Script import Import
    Import("env")
except ImportError:
    pass

# In PlatformIO/SCons, __file__ is not defined. 
# Attempt to find the project root relative to the build context.
if 'env' in globals():
    project_dir = env.get("PROJECT_DIR", os.getcwd())
else:
    project_dir = os.getcwd()

version_file = os.path.join(project_dir, "version.json")

# Fallback: If not found, try common relative paths
if not os.path.exists(version_file):
    # Check if we are in a subdirectory like .esphome/build/xxx
    potential_root = os.path.abspath(os.path.join(project_dir, "..", "..", ".."))
    if os.path.exists(os.path.join(potential_root, "version.json")):
        project_dir = potential_root
        version_file = os.path.join(project_dir, "version.json")

def bump_version():
    if not os.path.exists(version_file):
        data = {"major": 0, "minor": 6, "patch": 0}
    else:
        with open(version_file, "r") as f:
            try:
                data = json.load(f)
            except Exception:
                data = {"major": 0, "minor": 6, "patch": 0}

    # Increment patch version
    data["patch"] += 1
    
    # Save updated version
    with open(version_file, "w") as f:
        json.dump(data, f, indent=2)
    
    version_str = f"{data['major']}.{data['minor']}.{data['patch']}"
    
    # Inject as a C++ macro: -DPROJECT_VERSION="0.6.1"
    if 'env' in globals():
        env.Append(CPPDEFINES=[
            ("PROJECT_VERSION", f'\\"{version_str}\\"')
        ])
    
    print(f"\n>>> AUTOMATED VERSION BUMP: {version_str} <<<\n")

if __name__ == "__main__" or 'env' in globals():
    bump_version()
