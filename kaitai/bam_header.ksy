meta:
  id: bam_header
  file-extension: bam_header
  endian: le
  
seq:
  - id: header
    type: header
  - id: reference_sequences
    type: reference_sequence
    repeat: expr
    repeat-expr: header.n_ref

types:
  header:
    seq:
      - id: magic
        contents: ["BAM", 1]
      - id: l_text
        type: s4
      - id: text
        type: str
        size: l_text
        encoding: UTF-8
      - id: n_ref
        type: s4
  reference_sequence:
    seq:
      - id: l_name
        type: s4
      - id: name
        type: str
        size: l_name
        encoding: UTF-8
      - id: l_ref
        type: s4