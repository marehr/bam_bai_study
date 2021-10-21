# based on https://delagoya.github.io/rusty-bio/binary-file-parsing-in-bioinformatics

meta:
  id: bam
  file-extension: bam
  endian: le
  imports:
    - bam_alignment
    - bam_header
seq:
  - id: header
    type: bam_header
  - id: alignments
    type: bam_alignment
    repeat: eos
