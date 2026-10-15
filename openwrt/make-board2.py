"""Build ath11k board-2.bin files for the Deco X50 v1 from stock board data.

The stock firmware selects its board data by region (see
/lib/read_caldata_to_fs.sh in the stock rootfs). For a US unit without a
hardware version in product-info, those are:

  2.4 GHz (IPQ5018): bdwlan_US.b24  from /lib/firmware/IPQ5018/fw_0.tar.gz
  5 GHz   (QCN6122): bdwlan_US.b60  from /lib/firmware/qcn6122/

Usage:
  python3 make-board2.py <bdwlan.b24> <bdwlan.b60> <output dir>

The output files are picked up by build.sh. They are TP-Link's data, so
they are kept out of git.
"""
import os
import struct
import sys

MAGIC = b'QCA-ATH11K-BOARD'
IE_BOARD = 0
IE_BOARD_NAME = 0
IE_BOARD_DATA = 1
NAME = 'bus=ahb,qmi-chip-id=0,qmi-board-id=255,variant=TP-Link-Deco-X50-v1'


def ie(ie_id, data):
    pad = (4 - len(data) % 4) % 4
    return struct.pack('<II', ie_id, len(data)) + data + b'\0' * pad


def board2(data):
    board = ie(IE_BOARD_NAME, NAME.encode()) + ie(IE_BOARD_DATA, data)
    magic = MAGIC + b'\0'
    magic += b'\0' * ((4 - len(magic) % 4) % 4)
    return magic + ie(IE_BOARD, board)


def main():
    if len(sys.argv) != 4:
        sys.exit(__doc__)
    b24, b60, out = sys.argv[1:]
    os.makedirs(out, exist_ok=True)
    for src, suffix in ((b24, 'ipq5018'), (b60, 'qcn6122')):
        data = open(src, 'rb').read()
        dst = os.path.join(out, 'board-tplink_deco-x50-v1.' + suffix)
        open(dst, 'wb').write(board2(data))
        print(f'{dst}: {len(data)} bytes of board data')


if __name__ == '__main__':
    main()
