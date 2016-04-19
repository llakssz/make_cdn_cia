**From what I remember, this code was completely rewritten, not just by 173210. The original repo was deleted so I'm not sure who to credit... sorry.**

**Because of the rewrite, I would call this a Version 2.0 - if this code really is the latest code from the original repo.**

#### CTR_Toolkit - make_cdn_cia - Generates CIA files from CDN Content ####
#### Version: 1.1 by 3DSGuy and 173210 ####

### Usage ###

Usage: make_cdn_cia CDN_Content_Dir Output_CIA_File

CDN_Content_Dir - This is the directory where CDN content for the title for generating a CIA, is located. Do not modify any of the files, they must be raw.

Output_CIA_File - The name of the output CIA file.

Example:

make_cdn_cia 0004001000021400 Nintendo_3DS_Sound.cia

### Creating Input Files ###

1/ See the 3DS Title List here: http://www.3dbrew.org/wiki/Title_list

2/ Obtain the Title ID for your selected system title.

3/ And download the following:

http://nus.cdn.c.shop.nintendowifi.net/ccs/download/<Title_ID>/cetk
http://nus.cdn.c.shop.nintendowifi.net/ccs/download/<Title_ID>/tmd

4/ Read the TMD with ctrtool(or other program), to identify the number of content and their Content ID(s), each content can be download here:

http://nus.cdn.c.shop.nintendowifi.net/ccs/download/<Title_ID>/<Content_ID>

5/ Put all of these files in one directory and do not modify any of them.

6/ Done, the input is now ready for use with make_cdn_cia.

### Change Log ###

Version 1.1:
* Unspecified improvements by 173210.

Version 1.0:
* Initial Public release
* Supports generating valid CIA files from Title Content on Nintendo's CDN
