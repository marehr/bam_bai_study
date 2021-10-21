meta:
  id: bai
  file-extension: bai
  endian: le
seq:
  - id: magic
    contents:
      - BAI
      - 0x01
  - id: n_ref
    type: u4
  - id: references
    type: reference
    repeat: expr
    repeat-expr: n_ref
  - id: n_no_coor
    type: u8
types:
  voffset:
    seq:
      - id: virtual_file_offset
        type: u8
    instances:
      coffset_bgzf_block_begin:
        value: virtual_file_offset >> 16
      uoffset_within_uncompressed_block:
        value: virtual_file_offset ^ (coffset_bgzf_block_begin << 16)
  chunk:
    seq:
      - id: chunk_beg
        type: voffset
        size: 8
      - id: chunk_end
        type: voffset
        size: 8
  bin:
    seq:
      - id: bin
        type: u4
      - id: n_chunk
        type: s4
      - id: chunks
        type: chunk
        repeat: expr
        repeat-expr: n_chunk
  reference:
    seq:
      - id: n_bin
        type: s4
      - id: bins
        type: bin
        repeat: expr
        repeat-expr: n_bin
      - id: n_intv
        type: s4
      - id: ioffset
        type: voffset
        repeat: expr
        repeat-expr: n_intv
  # Not used currently, bought here for completeness
  # pseudo_bin:
  #   seq:
  #     - id: bin
  #       type: u4
  #       contents: 37450
  #     - id: n_chunk
  #       type: s4
  #       contents: '2'
  #     - id: unmapped_beg
  #       type: voffset
  #       size: 8
  #     - id: unmapped_end
  #       type: voffset
  #       size: 8
  #     - id: n_mapped
  #       type: u8
  #     - id: n_unmapped
  #       type: u8

