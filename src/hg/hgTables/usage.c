/* usage - functions that display helpful usage info text go here. 
 * (Soon to be Donna's domain. -jk)*/

#include "common.h"
#include "jksql.h"
#include "hgTables.h"

void printMainHelp()
/* Put up main page help info. */
{
hPrintf("%s",
"Here is a line-by-line description of the controls:\n"
"<UL>\n"
"   <LI><B>organism: </B>Which organism to work with.</LI>\n"
"   \n"
"   <LI><B>assembly: </B>Which version of that organism's genome\n"
"   sequence to use.</LI>\n"
"   \n"
"   <LI><B>group: </B>Selects which group the track is in. This\n"
"   corresponds to the grouping of tracks as separated by the blue\n"
"   bars in the genome browser.</LI>\n"
"   \n"
"   <LI><B>track: </B>Which track to work with. (Hint: there is a lot\n"
"   of data associated with the known genes, RefSeq, and Ensembl\n"
"   tracks, all of which are in the Gene and Gene Prediction\n"
"   group).</LI>\n"
"   \n"
"   <LI><B>region: </B>With this one can restrict the query to a\n"
"   particular chromosome or region. Select genome to apply the query\n"
"   to the entire genome.  Alternatively select range and type in either \n"
"   the chromosome name (such as chrX) or something like chrX:100000-200000 \n"
"   in the text box.</LI>\n"
"   \n"
"   <LI><B>select identifiers: </B>You can restrict your attention to\n"
"   a items in the track that match a list of identifiers (for\n"
"   instance RefSeq accessions for the RefSeq track) here. Ignore this\n"
"   line if you wish to consider all items in the track.</LI>\n"
"   \n"
"   <LI><B>filter: </B>You can restrict the query to only items that\n"
"   match certain criteria by creating a filter. For instance you\n"
"   could restrict your attention only to genes of a single exon.</LI>\n"
"   \n"
"   <LI><B>offset: </B>In many cases the output of the track\n"
"   intersector is a list of regions in the genome or DNA&nbsp;from\n"
"   these regions. The offset control lets you adjust the final output\n"
"   relative to these regions. The 'start' field specifies the\n"
"   relative offset of the starting position. A negative number here\n"
"   will make the region bigger, a positive number will make it\n"
"   smaller. A positive number in the end field makes the region\n"
"   bigger. The 'relative to' control has three values. If it is\n"
"   'start' then both start and end offsets are relative to the start\n"
"   position in the original region. Similarly if it is 'end' then the\n"
"   offsets are relative to the end position in the original region.\n"
"   If the 'relative to' control is 'both' (the default)&nbsp;then the\n"
"   start is relative to the original start position, and the end is\n"
"   relative to the original end position. Here are some examples of\n"
"   common usages:\n"
"   \n"
"   <UL>\n"
"      <LI>Promoters: start: -1000 end: 50 relative to: start</LI>\n"
"      \n"
"      <LI>10,000 bases on either side of a gene: start: -10000 end\n"
"      10000 relative to:&nbsp;both</LI>\n"
"      \n"
"      <LI>The last 50 bases of a gene: start: -50 end: 0 relative to:\n"
"      end</LI>\n"
"   </UL>\n"
"   </LI>\n"
"   \n"
"   <LI><B>output: </B>This controls the output format. Options\n"
"   include:\n"
"   \n"
"   <UL>\n"
"      <LI>Primary Table - the primary table associated with the track\n"
"      in a tab-separated format suitable for import into spreadsheets\n"
"      and relational databases.  This format is reasonably human readable \n"
"      as well.\n"
"      <LI>Sequence - get DNA or for some tracks protein sequence\n"
"      associated with track.</LI>\n"
"      <LI>Selected Fields - select fields from the various tables associated\n"
"      with the track.  The result is tab-separated\n"
"      <LI>Schema - output description of tables associated with track, but\n"
"      not the actual data in the tables\n"
"      <LI>Statistics - this calculates summary information such as\n"
"      number of items and how many bases are coverd by the results of\n"
"      a query.</LI>\n"
"      <LI>BED - saves the positions of all the items in a standard\n"
"      UCSC Browser format.</LI>\n"
"      \n"
"      <LI>GTF - saves the positions of all the items in a standard\n"
"      format for gene predictions. (Both BED&nbsp;and&nbsp;GTF can be\n"
"      used as the basis for custom tracks).</LI>\n"
"      \n"
"      <LI>Custom Track - this creates a custom track in the browser\n"
"      based on the results of the query.</LI>\n"
"      \n"
"   </UL>\n"
"   </LI>\n"
"   \n"
"   <LI><B>submit: </B>Press this button to have the server compute\n"
"   and send the output.</LI>\n"
"   \n"
"   <LI><B>intersection: </B>Press this button if you wish to combine\n"
"   this query with another query. This can be useful for such things\n"
"   as finding all SNPs which intersect RefSeq coding regions.</LI>\n"
"</UL>\n");
}
