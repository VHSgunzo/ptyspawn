# Maintainer: VHSgunzo <vhsgunzo.github.io>
pkgname='ptyspawn'
pkgver='0.0.5'
pkgrel='1'
pkgdesc='Tool for executing a command in a new PTY (pseudo-terminal) with new PGID and SID'
arch=("aarch64" "x86_64")
url="https://github.com/VHSgunzo/${pkgname}"
provides=("${pkgname}")
conflicts=("${pkgname}")
source=("https://github.com/Azathothas/${pkgname}/releases/download/v${pkgver}/${pkgname}-${CARCH}-Linux")
sha256sums=('SKIP')

package() {
    install -Dm755 "${pkgname}-${CARCH}-Linux" "$pkgdir/usr/bin/${pkgname}"
}