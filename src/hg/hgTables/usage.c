/* usage - functions that display helpful usage info text go here. -jk)*/

#include "common.h"
#include "jksql.h"
#include "hgTables.h"


void printMainHelp()
/* Put up main page help info. */
{
hPrintf("%s",
"This section provides brief line-by-line descriptions of the Table \n"
"Browser controls. For more information on using this program, see the \n"
"<A HREF=\"../goldenPath/help/hgTablesHelp.html\" TARGET=_blank>Table \n"
"Browser User's Guide</A>.\n"
"        <UL>\n"
"        <LI><B>clade: </B>Specifies which clade the organism is in.</LI>\n"
"        \n"
"        <LI><B>genome: </B>Specifies which organism data to use.</LI>\n"
"        \n"
"        <LI><B>assembly: </B>Specifies which version of the organism's genome\n"
"        sequence to use.</LI>\n"
"        \n"
"        <LI><B>group: </B>Selects the type of tracks to be displayed in \n"
"        the <em>track</em> list. The options correspond to the track groupings\n"
"        shown in the Genome Browser. Select 'All Tracks' for an alphabetical list\n"
"        of all available tracks in all groups.  Select 'All Tables' to see all tables\n"
"        including those not associated with a track.</LI>\n"
"        \n"
"        <LI><B>database: </B> (with &quot;All Tables&quot; group option) Determines \n"
"	 which database should be used for options in table menu.</LI>\n"
"        \n"
"        <LI><B>track: </B>Selects the annotation track data to work with. This \n"
"        list displays all tracks belonging to the group specified in the \n"
"        <em>group</em> list. </LI>\n"
"        \n"
"        <LI><B>table: </B>Selects the SQL table data to use. This list shows \n"
"        all tables associated with the track specified in the <em>track</em> \n"
"        list.</LI>\n"
"        \n"
"        <LI><B>describe table schema: </B>Displays schema information for the \n"
"        tables associated with the selected track.</LI>\n"
"        \n"
"        <LI><B>region: </B>Restricts the query to a\n"
"        particular chromosome or region. Select <em>genome</em> to apply the \n"
"        query to the entire genome or <em>ENCODE</em> to examine only the \n"
"        ENCODE Pilot regions.\n"
"        To limit the query to a specific position, type a \n"
"        chromosome name, e.g. <em>chrX</em>, or a chromosome coordinate \n"
"        range, such as chrX:100000-200000, or a gene name or other id in \n"
"        the text box.\n"
"        You can select multiple genomic regions by clicking the &quot;define regions&quot; \n"
"        button and entering up to 1,000 regions in a 3- or 4-field <a \n"
"		 href=\"../FAQ/FAQformat.html#format1\">BED</a> file \n"
"		 format.</LI>\n"
"        \n"
"        <LI><B>lookup:</B> Press this button after typing in a gene name or \n"
"        other id in the position text box to look up the chromosome position\n"
"        \n"
"        <LI><B>identifiers</B> (selected tracks only)<B>: </B>Restricts the \n"
"        output to table data that match a list of identifiers, for\n"
"        instance RefSeq accessions for the RefSeq track. If no identifiers \n"
"        are entered, all table data within the specified region will be \n"
"        displayed.</LI>\n"
"        \n"
"        <LI><B>filter: </B>Restricts the query to only those items that\n"
"        match certain criteria, e.g. genes with a single exon. Click the \n"
"        <em>Create</em> button to add a filter, the <em>Edit</em> button to \n"
"        modify an existing filter, or the <em>Clear</em> button to remove an \n"
"        existing filter.</LI>\n"
"        \n"
"        <LI><B>intersection </B> (selected tracks only)<B>: </B>Combines the output \n"
"	 of two queries into a \n"
"        single set of data based on specific join criteria. For example, this \n"
"        can be used to find all SNPs that intersect with RefSeq coding \n"
"        regions. The intersection can be configured to retain the \n"
"        existing alignment structure of the table with a specified amount of \n"
"        overlap, or discard the structure in favor of a simple list of position\n"
"        ranges using a base-pair intersection or union of the two data sets. \n"
"        The button functionalities are similar to those of the <em>filter</em>\n"
"        option.</LI> \n"
"        <LI><B>output: </B>Specifies the output format (not all options are \n"
"        available for some tracks). Formats include:\n"
"        <UL>\n"
"        <LI><em><B>all fields from selected table</B></em> - data from the selected table\n"
"        displayed in a tab-separated format suitable \n"
"        for import into spreadsheets and relational databases. The ASCII \n"
"        format may be read in any web browser or text editor.\n"
"        <LI><em><B>selected fields from primary and related tables</B></em> - user-selected \n"
"        set of tab-separated fields \n"
"        from the selected table and (optionally) other related tables as well. \n"
"        <LI><em><B>sequence</B></em> - DNA (or protein sequence, in some cases) \n"
"        associated with the table.</LI>\n"
"        <LI><em><B>BED</B></em> - positions of data items in a standard\n"
"        UCSC Browser format.</LI>\n"
"        \n"
"        <LI><em><B>GTF</B></em> - positions of all data items in a standard\n"
"        gene prediction format. (Both BED and GTF formats can be\n"
"        used as the basis for custom tracks).</LI>\n"
"        \n"
"        <LI><em><B>CDS FASTA alignment from multiple alignment</B></em> \n"
"        - FASTA alignments of the CDS regions of a gene prediction track \n"
"        using any of the multiple alignment tracks for the current database. \n"
"        Output sequence can be in either nucleotide-space or translated to \n"
"        protein-space. Available only for genePred tracks.</LI>\n"
"        \n"
"        <LI><em><B>custom track</B></em> -  customized Genome Browser annotation  \n"
"        track based on the results of the query.</LI>\n"
"        \n"
"        <LI><em><B>hyperlinks to Genome Browser</B></em> -  returns a page full of\n"
"        hyperlinks to the UCSC Genome Browser, one for each item in the table.</LI>\n"
"        \n"
"        <LI><em><B>data points</B></em> -  the data points that make up a graph \n"
"	 (aka wiggle) track.</LI>\n"
"        \n"
"        <LI><em><B>MAF</B></em> - multiple alignments in MAF format</LI>\n"
"        \n"
"        </UL>\n"
"        </LI>\n"
"        \n"
"        <LI><B>Send output to Galaxy:</B> displays results of query in \n"
"        <A HREF=\"http://g2.bx.psu.edu\">Galaxy</A>, a framework for \n"
"        interactive genome analysis.\n"
"        <LI><B>Send output to GREAT:</B> displays the functional enrichments of the \n"
"        query results in <A HREF=\"http://great.stanford.edu\">GREAT</A>, a tool for\n"
"        analysis of the biological function of cis-regulatory regions.\n"
"        <LI><B>file type returned: </B>When a filename is entered in the \n"
"	 &quot;output file&quot; text box, specifies the format of the output file:\n"
"        <UL>\n"
"        <LI><em><B>plain text</B></em> - data is in ASCII format\n"
"        \n"
"        <LI><em><B>gzip compressed</B></em> - data is compressed in gzip format\n"
"        \n"
"        </UL>\n"
"        </LI>\n"
"        <LI><B>get output: </B>Submits a data query based on the specified \n"
"        parameters and returns the output.</LI>\n"
"        \n"
"        <LI><B>summary/statistics: </B>Displays statistics about the data \n"
"        specified by the parameters.</LI>\n"
"        \n"
"        </UL>\n");
}
