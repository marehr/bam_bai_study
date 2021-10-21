meta:
  id: bgzf
  endian: le
seq:
  - id: blocks
    type: block
    repeat: eos
types:
  block:
    seq:
    - id: id1
      contents: [31]
    - id: id2
      contents: [139]
    - id: cm
      contents: [8]
    - id: flg
      contents: [4]
    - id: mtime
      type: u4
    - id: xfl
      type: u1
    - id: os
      type: u1
    - id: xlen
      type: u2
    - id: extra_subfields
      type: extra_subfields
      size: xlen
    - id: cdata
      size: extra_subfields.bsize - xlen - 19
      # process: zlib
    - id: crc32
      type: u4
    - id: isize
      type: u4
  extra_subfields:
    seq:
      - id: si1
        contents: [66]
      - id: si2
        contents: [67]
      - id: slen
        contents: [2, 0]
      - id: bsize
        type: u2

