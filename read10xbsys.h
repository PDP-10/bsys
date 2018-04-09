/* read10xbsys.h */
#ifdef COMMENT

Make up of Bsys tapes:

-------------------------------------------------------------------------------
Directory
-------------------------------------------------------------------------------
Files
-------------------------------------------------------------------------------

The Directory consists of 1 320 bytes long record, and many other records
which contain the info about files.  This is all skipped over in read10x
because it is irrelevant for unix.

The files consist of n+1 records, where the lenght of the file is n records.
The first record contains the strings associated with the file.
Out of those, we are interested in the ones which start at offsets 24,40,and
67.  Those comprise the name of the file and directory in which it was
located.
Each other record has the text of the file starting at offset 2.  The offset
0 and 1 are supposed to contain the logical number of the record.  When
deciphered, these can be used instead of logrecord and eliminate the need
for -s and core dumping when the run is interrupted.
#endif

				/* Checksum is first word */
#define WdoffChecksum      0
#define BtoffChecksum      0
#define BtlenChecksum     36
				/* Page access bits is second word */
#define WdoffAccess        1
#define BtoffAccess        0
#define BtlenAccess       36
				/* SCD, first 3 bits in next word */
#define WdoffSCD           2
#define BtoffSCD           0
#define BtlenSCD           3
				/* Number of saveset on tape */
#define WdoffSaveSetNum    2
#define BtoffSaveSetNum    3
#define BtlenSaveSetNum   15
				/* Tape number of dump */
#define WdoffTapeNum       2
#define BtoffTapeNum      18
#define BtlenTapeNum      18
				/* F1, F2 Flag bits */
#define WdoffF1F2          3
#define BtoffF1F2          0
#define BtlenF1F2          2
				/* File Number in Set (new format only) */
#define WdoffFileNum       3
#define BtoffFileNum       2
#define BtlenFileNum      16
				/* Page Number in file */
#define WdoffPageNum       3
#define BtoffPageNum      18
#define BtlenPageNum      18
				/* Record type (2's complement) */
#define WdoffRectype       4
#define BtoffRectype       0
#define BtlenRectype      36
				/* Record sequence number */
#define WdoffRecseq        5
#define BtoffRecseq        0
#define BtlenRecseq       36


				/* SCD Values */
#define SCDNormal       0
#define SCDCollection   1
#define SCDArchive      2
#define SCDMigration    3

				/* F1, F2 Values */
#define F1F2Old            0
#define F1F2OldContinue    3
#define F1F2New            1
#define F1F2NewContinue    2

				/* Record type values */
#define RectypeData     0
#define RectypeTphd     1
#define RectypeFlhd     2
#define RectypeFltr     3
#define RectypeTptr     4
#define RectypeUsr      5
#define RectypeCtph     6
#define RectypeFill     7

#define WdoffSSDate        8            /* Saveset date offset (type 1, 6) */
#define WdoffSSName        9            /* Saveset name offset (type 1, 6) */
#define WdoffFLName        6            /* Filename offset (type 2) */
#define WdoffFDB         134            /* FDB offset (type 2) */

					/* Number of bits per byte */
#define WdoffFDB_BSZ     011+WdoffFDB
#define BtoffFDB_BSZ       6
#define BtlenFDB_BSZ       6

					/* Number of pages in the file */
#define WdoffFDB_PGC     011+WdoffFDB
#define BtoffFDB_PGC      18
#define BtlenFDB_PGC      18

					/* Number of bytes in the file */
#define WdoffFDB_Size    012+WdoffFDB
#define BtoffFDB_Size      0
#define BtlenFDB_Size     36

					/* Date of last write to file */
#define WdoffFDB_Wrt     014+WdoffFDB

