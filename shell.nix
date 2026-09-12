{ pkgs ? import <nixpkgs> { } }:

pkgs.mkShell {
  packages = with pkgs; [
    tinyxxd
    qemu_kvm
	  gnumake
	  gdb
	  coreboot-toolchain.i386
	  grub2
    libisoburn
  ];

  shellHook = ''
    echo "OSDev Shell"
  '';
}
