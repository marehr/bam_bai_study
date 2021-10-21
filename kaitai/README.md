Goto https://ide.kaitai.io/# and upload these files to the local storage.

You can view the binary representation in the editor.

First select e.g. bai.ksy by double clicking (loads the description to parse the binary .bai format) and then the
example1.bam.bai file.

Example Usage:

file			        kaitai file
example_bad.bam                 bgzf.ksy (Blocked - GZip File, Contains multiple BGZF-Blocks)
example_bad.bam.bai             bai.ksy (BAI - File Format)
example_bad.bam.uncompressed    bam.ksy (Complete BAM File with Header AND alignment records)
example_bad.sam                 no ksy file (Original Sam file converted to bam file with samtools)
example_bad.bam.0000            bam_header.ksy (First uncompressed BGZF-Block, it only contains the header)
example_bad.bam.0290            bam_alignments.ksy (Second uncompressed BGZF-Block, it only contains alignments)
example_bad.bam.4347            bam_alignments.ksy (Third uncompressed BGZF-Block, it only contains alignments)
example_bad.bam.5044            bam_alignments.ksy (Last uncompressed BGZF-Block which is by specification empty, to detect corruptions)

Note that 0000, 0290, 4347 and 5044 are the file position in the .bam file where the BGZF-Block begins (in compressed .bam file).
It corresponds to the vOffset used in the .bai file. For example vOffset = 0x1220000 where the lower 4 bytes 0x0000
corresponds to uoffsetWithinUncompressedBlock = 0 position and the upper 4 bytes 0x0122 to coffsetBgzfBlockBegin = 0290
position in the compressed .bam file.
