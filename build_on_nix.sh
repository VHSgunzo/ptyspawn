#!/usr/bin/env bash

###-----------------------------------------------------###
### Setups ENV, Install Deps & Builds Binaries (+ upx)  ###
### This Script must be run as `root` (passwordless)    ###
### Assumptions: OS (Ubuntu) | Arch (x86_64 | aarch64)  ###
### Can also run in chroot | Docker (nix may not work)  ###
###-----------------------------------------------------###


##-------------------------------------------------------#
##Install ptyspawn & Deps
export DEBIAN_FRONTEND="noninteractive"
echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections
sudo apt update -y -qq
sudo apt install 7zip b3sum bc binutils binutils-aarch64-linux-gnu ptyspawn curl dos2unix fdupes jq moreutils wget -y -qq
sudo apt-get install apt-transport-https apt-utils b3sum bc binutils binutils-aarch64-linux-gnu ca-certificates ptyspawn dos2unix fdupes gnupg2 jq moreutils p7zip-full rename rsync software-properties-common texinfo tmux upx util-linux wget -y -qq 2>/dev/null ; sudo apt-get update -y 2>/dev/null
#Do again, sometimes fails
sudo apt install 7zip b3sum bc binutils binutils-aarch64-linux-gnu ptyspawn curl dos2unix fdupes jq moreutils wget -y -qq
sudo apt-get install apt-transport-https apt-utils b3sum bc binutils binutils-aarch64-linux-gnu ca-certificates ptyspawn dos2unix fdupes gnupg2 jq moreutils p7zip-full rename rsync software-properties-common texinfo tmux upx util-linux wget -y -qq2>/dev/null ; sudo apt-get update -y 2>/dev/null
##-------------------------------------------------------#

##-------------------------------------------------------#
##ENV
SYSTMP="$(dirname $(mktemp -u))" && export SYSTMP="${SYSTMP}" ; mkdir -p "${SYSTMP}/nix_builder"
TMPDIRS="mktemp -d --tmpdir=${SYSTMP}/nix_builder XXXXXXX_linux_$(uname -m)-$(uname -s)" && export TMPDIRS="${TMPDIRS}"
##Artifacts
ARTIFACTS="${SYSTMP}/ARTIFACTS-$(uname -m)-$(uname -s)" && export "ARTIFACTS=${ARTIFACTS}"
rm -rf "${ARTIFACTS}" 2>/dev/null ; mkdir -p "${ARTIFACTS}"
##User-Agent
USER_AGENT="$(curl -qfsSL 'https://pub.ajam.dev/repos/Azathothas/Wordlists/Misc/User-Agents/ua_chrome_macos_latest.txt')" && export USER_AGENT="${USER_AGENT}"
##-------------------------------------------------------#

##-------------------------------------------------------#
##Install Nix
##Official Installers break
#curl -qfsSL "https://nixos.org/nix/install" | bash -s -- --no-daemon
#source "$HOME/.bash_profile" ; source "$HOME/.nix-profile/etc/profile.d/nix.sh" ; . "$HOME/.nix-profile/etc/profile.d/nix.sh"
##https://github.com/DeterminateSystems/nix-installer
"/nix/nix-installer" uninstall --no-confirm 2>/dev/null
curl -qfsSL "https://install.determinate.systems/nix" | bash -s -- install linux --init none --no-confirm
source "/nix/var/nix/profiles/default/etc/profile.d/nix-daemon.sh"
sudo chown --recursive "$(whoami)" "/nix"
echo "root    ALL=(ALL:ALL) ALL" | sudo tee -a "/etc/sudoers"
nix --version && nix-channel --list && nix-channel --update --cores "$(($(nproc)+1))" --quiet
##-------------------------------------------------------#

##-------------------------------------------------------#
##Install 7z
pushd "$($TMPDIRS)" >/dev/null 2>&1 && curl -A "${USER_AGENT}" -qfsSLJO "https://www.7-zip.org/$(curl -A "${USER_AGENT}" -qfsSL "https://www.7-zip.org/download.html" | grep -o 'href="[^"]*"' | sed 's/href="//' | grep -i "$(uname -s)-$(uname -m | sed 's/x86_64/x64\\|x86_64/;s/aarch64/arm64\\|aarch64/')" | sed 's/"$//' | sort -n -r | head -n 1)" 2>/dev/null
find "." -type f -name '*.xz' -exec tar -xf {} \; 2>/dev/null
sudo find "." -type f -name '7zzs' ! -name '*.xz' -exec mv {} "/usr/bin/7z" \; 2>/dev/null
sudo cp "/usr/bin/7z" "/usr/local/bin/7z" 2>/dev/null
sudo chmod +x "/usr/bin/7z" "/usr/local/bin/7z" 2>/dev/null
popd >/dev/null 2>&1
##-------------------------------------------------------#

##-------------------------------------------------------#
##Install upX
pushd "$($TMPDIRS)" >/dev/null 2>&1
curl -qfLJO "$(curl -qfsSL https://api.github.com/repos/upx/upx/releases/latest | jq -r '.assets[].browser_download_url' | grep -i "$(uname -m | sed 's/x86_64/amd64\\|x86_64/;s/aarch64/arm64\\|aarch64/')_$(uname -s)")" 2>/dev/null
find "." -type f -name '*tar*' -exec tar -xvf {} \; 2>/dev/null
find "." -type f -exec file -i "{}" \; | grep "application/.*executable" | cut -d":" -f1 | xargs realpath | xargs -I {} sudo cp --force {} "/usr/bin/upx"
sudo cp "/usr/bin/upx" "/usr/local/bin/upx" 2>/dev/null
sudo chmod +x "/usr/bin/upx" "/usr/local/bin/upx" 2>/dev/null
popd >/dev/null 2>&1
##-------------------------------------------------------#

###-------------------------------------------------------#
###Build
#pushd "$($TMPDIRS)" >/dev/null 2>&1
#NIXPKGS_ALLOW_BROKEN="1" NIXPKGS_ALLOW_UNSUPPORTED_SYSTEM="1" nix-build '<nixpkgs>' --attr "pkgsStatic.ptyspawn" --cores "$(($(nproc)+1))" --max-jobs "$(($(nproc)+1))" --log-format bar-with-logs
#PKG_VERSION="$(nix derivation show "nixpkgs#pkgsStatic.ptyspawn" 2>&1 | grep '"version"' | awk -F': ' '{print $2}' | tr -d '"')" && export PKG_VERSION="${PKG_VERSION}"
#BIN_DIR="$(find "." -maxdepth 1 -type d -o -type l -exec realpath {} \; | grep -Ev '^\.$')"
#sudo rsync -av --copy-links --no-relative "$(find "$BIN_DIR" -type d -path '*/bin*' -print0 | xargs --null -I {} realpath {})/." "${ARTIFACTS}"
#sudo chown -R "$(whoami):$(whoami)" "${ARTIFACTS}" && chmod -R 755 "${ARTIFACTS}"
#find "${ARTIFACTS}" -type f -name '*.sh' -delete 2>/dev/null
#find "${ARTIFACTS}" -type f -exec bash -c 'mv "$0" "${0}-$(uname -m)-$(uname -s)"' {} \; 2>/dev/null
##Strip
#find "${ARTIFACTS}" -type f -exec strip --strip-debug --strip-dwo --strip-unneeded -R ".comment" -R ".gnu.version" --preserve-dates "{}" \; 2>/dev/null
##upx
#find "${ARTIFACTS}" -type f | xargs realpath | xargs -I {} upx --best "{}" -f --force-overwrite -o"{}.upx" -qq 2>/dev/null
##End
#nix-collect-garbage >/dev/null 2>&1 ; popd >/dev/null 2>&1
###-------------------------------------------------------#

##-------------------------------------------------------#
##Build
#Deps
sudo apt-get update -y -qq
sudo apt-get install autoconf automake autopoint binutils bison build-essential byacc ca-certificates ccache clang diffutils dos2unix gawk flex file imagemagick jq liblz-dev librust-lzma-sys-dev libsqlite3-dev libtool libtool-bin lzip lzma lzma-dev make musl musl-dev musl-tools p7zip-full patch patchelf pkg-config qemu-user-static rsync scons sqlite3 sqlite3-pcre sqlite3-tools texinfo wget -y -qq 2>/dev/null
#Re
sudo apt-get install autoconf automake autopoint binutils bison build-essential byacc ca-certificates ccache clang diffutils dos2unix gawk flex file imagemagick jq liblz-dev librust-lzma-sys-dev libsqlite3-dev libtool libtool-bin lzip lzma lzma-dev make musl musl-dev musl-tools p7zip-full patch patchelf pkg-config qemu-user-static rsync scons sqlite3 sqlite3-pcre sqlite3-tools texinfo wget -y -qq 2>/dev/null
sudo apt install python3-pip python3-venv -y -qq 2>/dev/null
pip install --break-system-packages --upgrade pip || pip install --upgrade pip
sudo apt install lm-sensors pciutils procps python3-distro python3-netifaces sysfsutils virt-what -y -qq 2>/dev/null
pip install build ansi2txt cffi pipx scons scuba py2static pytest typer --upgrade --force 2>/dev/null 
pip install build ansi2txt cffi pipx scons scuba py2static pytest typer --break-system-packages --upgrade --force 2>/dev/null
#https://github.com/VHSgunzo/ptyspawn-static/blob/main/build.sh
#Musl
pushd "$($TMPDIRS)" >/dev/null 2>&1 && git clone --filter "blob:none" "https://git.musl-libc.org/git/musl" && cd "./musl"
unset AR CC CFLAGS CXX CPPFLAGS CXXFLAGS DLLTOOL HOST_CC HOST_CXX LDFLAGS OBJCOPY RANLIB
make dest clean 2>/dev/null ; make clean 2>/dev/null
bash "./configure" --prefix="/usr/local/musl"
sudo make --jobs="$(($(nproc)+1))" --keep-going install ; popd >/dev/null 2>&1
sudo ldconfig && sudo ldconfig -p
#ptyspawn
pushd "$($TMPDIRS)" >/dev/null 2>&1 && git clone --filter "blob:none" "https://github.com/VHSgunzo/ptyspawn" && cd "./ptyspawn"
git tag --sort="-v:refname" | head -n 1 > "${ARTIFACTS}/VERSION.txt"
PKG_VERSION="$(cat "${ARTIFACTS}/VERSION.txt")" && export PKG_VERSION="${PKG_VERSION}"
export CC="$(which musl-gcc 2>/dev/null)"
export CFLAGS="-O2 -flto=auto -static -w -pipe ${CFLAGS}"
export CXXFLAGS="${CFLAGS}"
export CPPFLAGS="${CFLAGS}"
export LDFLAGS="-static -s -Wl,-S -Wl,--build-id=none ${LDFLAGS}"
make --jobs="$(($(nproc)+1))" --keep-going
find "." -maxdepth 1 -type f -exec file -i "{}" \; | grep "application/.*executable" | cut -d":" -f1 | xargs realpath | xargs -I {} rsync -av --copy-links {} "${ARTIFACTS}/"
sudo chown -R "$(whoami):$(whoami)" "${ARTIFACTS}" && chmod -R 755 "${ARTIFACTS}"
find "${ARTIFACTS}" -type f -name '*.sh' -delete 2>/dev/null
find "${ARTIFACTS}" -type f -exec bash -c 'mv "$0" "${0}-$(uname -m)-$(uname -s)"' {} \; 2>/dev/null
#Strip
find "${ARTIFACTS}" -type f -exec strip --strip-debug --strip-dwo --strip-unneeded -R ".comment" -R ".gnu.version" --preserve-dates "{}" \; 2>/dev/null
#upx
find "${ARTIFACTS}" -type f | xargs realpath | xargs -I {} upx --best "{}" -f --force-overwrite -o"{}.upx" -qq 2>/dev/null
#End
popd >/dev/null 2>&1
##-------------------------------------------------------#

##-------------------------------------------------------#
#Generate METADATA & Release Notes
pushd "${ARTIFACTS}" >/dev/null 2>&1
find "./" -maxdepth 1 -type f | sort | grep -v -E '\.txt$' | xargs file > "${ARTIFACTS}/FILE.txt"
find "./" -maxdepth 1 -type f | sort | grep -v -E '\.txt$' | xargs sha256sum > "${ARTIFACTS}/SHA256SUM.txt"
echo "${PKG_VERSION}" > "${ARTIFACTS}/VERSION.txt"
popd >/dev/null 2>&1
echo -e "\n[+] Built in $(realpath ${ARTIFACTS})\n"
ls "${ARTIFACTS}" -lah && echo -e "\n"
##-------------------------------------------------------#