meta:
  id: bam_alignment
  file-extension: bam_alignment
  endian: le

seq:
  - id: block_size
    type: s4
  - id: alignment_record
    size: block_size
    type: alignment_record
types:
  alignment_record: 
    seq:
    - id: ref_id
      type: s4
    - id: pos
      type: s4
    - id: l_read_name
      type: u1
    - id: mapq
      type: u1
    - id: bin
      type: u2
    - id: n_cigar_op
      type: u2
    - id: flag
      type: u2
    - id: l_seq
      type: u4
    - id: next_ref_id
      type: s4
    - id: next_pos
      type: s4
    - id: tlen
      type: s4
    - id: read_name
      type: str
      size: l_read_name
      encoding: UTF-8
    - id: cigar
      type: cigar_sequence
      repeat: expr
      repeat-expr: n_cigar_op
    - id: seq
      type: read_sequence
      repeat: expr
      repeat-expr: (l_seq + 1) / 2
    - id: qual
      type: u1
      repeat: expr
      repeat-expr: l_seq
  tag:
    seq:
      - id: tag
        type: str
        encoding: UTF-8
        size: 2
      - id: val_type
        type: str
        encoding: UTF-8
        size: 1
  read_sequence:
    seq:
      - id: char1
        type: b4
        enum: read_sequence_char
      - id: char2
        type: b4
        enum: read_sequence_char
    enums:
      read_sequence_char:
        0: equal
        1: a
        2: c
        3: m
        4: g
        5: r
        6: s
        7: v
        8: t
        9: w
        10: y
        11: h
        12: k
        13: d
        14: b
        15: n
  cigar_sequence:
    seq:
      - id: cigar
        type: u4
    enums:
      cigar_op:
        0: m
        1: i
        2: d
        3: n
        4: s
        5: h
        6: p
        7: equal
        8: x
    instances:
      op_len:
        value: cigar >> 4
      op:
        value: cigar ^ (op_len << 4)
        enum: cigar_op
