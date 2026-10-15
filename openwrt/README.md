# OpenWrt for TP-Link Deco X50 v1 (experimental)

> **Status: installed and running from flash on real hardware.** Ethernet
> (WAN/LAN), both Wi-Fi radios, LEDs, the real MAC address, `sysupgrade` and
> reboots have been tested.

## Hardware (v1 only)

The X50 **v3** is a completely different MediaTek MT7981 board. Nothing here applies to it.

| Part | Value |
| --- | --- |
| SoC | Qualcomm IPQ5018, 512 MiB RAM (U-Boot machid `0x8040002` = `ap-mp03.3`) |
| Flash | ESMT F50D1G41LB 128 MiB SPI-NAND |
| 2.4 GHz | IPQ5018 internal (board data `bdwlan_<country>.b24`) |
| 5 GHz | QCN6102 / QCN6122 family (board data `bdwlan_<country>.b60`) |
| Switch | **RTL8367S-VB** (chip id `0x6642`), mdio1 address 29, reset GPIO39 |
| CPU link | gmac1 → uniphy0 → 2.5G HSGMII → switch port 6 (extif0) |
| Jacks | top = switch port 2 (LAN), middle = port 1 (LAN), bottom = port 0 (**WAN**) |
| GE PHY | IPQ5018 internal GE PHY (gmac0) is **not** wired to a jack |
| Reset button | GPIO33 (active low) |
| RGB LED | red GPIO23, green GPIO24, blue GPIO29 |
| MAC | `factory_data` UBI → volume `ubi_factory_data` → `default-mac` |
| Caldata | `0:ART` at 0x1000 (2.4 GHz) and 0x26800 (5 GHz) |
| UART | GPIO20/21, 115200 8N1, **1.8 V** |
| Secure boot | Fuse not enabled. U-Boot boots unsigned FIT images. |
| Boot slot | U-Boot env `tp_boot_idx` (0 = `rootfs`, 1 = `rootfs_1`) |

The hardware description comes from `GPL_X55.tar.gz`, which TP-Link's Deco X50 v1 (US) download page links to, and from the device itself.

### Switch driver

Neither `rtl8365mb` (DSA) nor the stock `rtl8367b` (swconfig) driver can bring up the RTL8367S-VB's HSGMII interface. The patch extends `rtl8367b` as follows:

- A new extif mode `18` (`RTL8367D_EXTIF_MODE_HSGMII`) runs the SerDes 0 patch and HSGMII sequence from the Realtek SDK in the GPL release, then disables in-band autonegotiation.
- A new DT flag `realtek,skip-initvals` skips the RTL8367C init table. That table breaks the SerDes on this chip. The driver then only sets the GPHY OCP prefix and powers up the UTP PHYs, like the Realtek SDK does.
- A new `arl_table` switch attribute (`swconfig dev switch0 get arl_table`) lists the learned unicast MACs per port, in the same format as `ar8xxx`.

The switch conduit `eth0` (gmac1) has no MAC source of its own, so board.d pins it to the label MAC. Otherwise it would get a random address on every boot.

## Build

Requires Docker. The tree is kept in a Docker volume because it needs a case-sensitive filesystem.

1. Wi-Fi board data. These are TP-Link's files, so they are not in git; generate them from your own unit's stock rootfs:

   ```
   # from the stock rootfs: /lib/firmware/IPQ5018/fw_0.tar.gz -> bdwlan_US.b24
   #                        /lib/firmware/qcn6122/bdwlan_US.b60
   python3 openwrt/make-board2.py bdwlan_US.b24 bdwlan_US.b60 openwrt/wifi-board
   ```

   Use your unit's `country` from `factory_data/product-info`. Without these files, Wi-Fi does not come up.

2. Build:

   ```
   ./openwrt/build.sh                        # open Wi-Fi "X50-OpenWrt"
   WIFI_KEY=yourpassword ./openwrt/build.sh  # WPA2 instead
   ```

   Images end up in `openwrt/bin/`. The patch applies to OpenWrt `main` at the commit pinned in `build.sh`.

   Wi-Fi comes up enabled on first boot with one SSID on both bands. Set it with `WIFI_SSID`, `WIFI_KEY` (empty means an open network) and `WIFI_COUNTRY` (default `US`). The values are baked into the image and written to `bin/wifi-credentials.txt`.

   **The root password is empty after installation.** Set one in LuCI right away, especially with an open Wi-Fi.

## Boot from RAM (nothing is written to flash)

1. Connect the UART (1.8 V!). Put `openwrt-qualcommax-ipq50xx-tplink_deco-x50-v1-initramfs-uImage.itb` on a TFTP server at `192.168.1.2`, and plug the PC into any jack.
2. Power on and type `tpl` at the autoboot prompt. The window is 1 second.
3. Boot:

   ```
   setenv ipaddr 192.168.1.1
   setenv serverip 192.168.1.2
   tftpboot 0x44000000 openwrt-qualcommax-ipq50xx-tplink_deco-x50-v1-initramfs-uImage.itb
   bootm 0x44000000
   ```

   Right after power-on the switch link may not be up yet. If `tftpboot` reports no link, run it again.

4. Use the top or middle jack for LAN (`192.168.1.1`, LuCI). The bottom jack is WAN (DHCP client).

A power cycle returns the unit to the stock firmware.

## Install to flash

1. **Back up the whole flash first.** From the RAM-booted OpenWrt, read every `/dev/mtdN` over SSH, for example `ssh root@192.168.1.1 'cat /dev/mtd12' > mtd12.rootfs_1.bin`. Compare checksums with `sha256sum /dev/mtdN` on the device.
2. Still in the RAM-booted OpenWrt, copy over and install the sysupgrade image:

   ```
   ssh root@192.168.1.1 'cat > /tmp/sysupgrade.bin' < openwrt-qualcommax-ipq50xx-tplink_deco-x50-v1-squashfs-sysupgrade.bin
   ssh root@192.168.1.1 'sysupgrade -n /tmp/sysupgrade.bin'
   ```

When run from RAM, the image goes to `rootfs_1` and `tp_boot_idx` is set to 1. `rootfs` (slot 0) keeps the stock firmware.

## Updating (no UART needed)

Once OpenWrt runs from flash, update it like any OpenWrt device: use LuCI's *Flash new firmware image* or `sysupgrade` with the `-squashfs-sysupgrade.bin`. Settings are kept unless you use `-n`. The update is written to the slot that is not running, and `tp_boot_idx` then points to it, so the previous image stays behind as a fallback. **The first update from flash therefore overwrites the stock firmware in slot 0.** After that, the slots alternate between the current and the previous OpenWrt.

If the device boots but cannot be reached, for example after a bad configuration, press the reset button repeatedly while the LED blinks fast during boot. That enters OpenWrt failsafe mode at `192.168.1.1` with SSH. Both paths have been tested on the device.

How the stock U-Boot boots (`cmd_bootqca.c` / `ipq5018.c` in the GPL):

- It boots the slot in `tp_boot_idx` and passes `ubi.mtd=rootfs` or `ubi.mtd=rootfs_1`.
- It picks the FIT config by its own `config_name`, which is `config@mp03.3`.
- If it cannot load the kernel from that slot, it boots the other slot. A kernel that loads but then fails is not detected.

### Going back to stock

- Quick: in U-Boot (`tpl`), run `setenv tp_boot_idx 0; saveenv; reset` to boot the stock firmware in slot 0.
- Full (untested): write the backed-up `rootfs_1` back from a RAM-booted OpenWrt with `ubiformat /dev/mtd12 -f mtd12.rootfs_1.bin`, then run `fw_setenv tp_boot_idx 1`.

## Not done yet

- LED triggers beyond the status LED.
- Installing from stock without UART. The stock web UI only accepts TP-Link-signed firmware.
