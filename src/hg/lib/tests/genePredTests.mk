# makefile with genePred and genePredReader tests
include ../../../inc/common.mk

BIN_DIR = bin/$(MACHTYPE)
GENE_PRED_TESTER = ${BIN_DIR}/genePredTester
EXP_DIR = expected/genePred
OUT_DIR = output/genePred
DB=hg16
TEST_TBL=gpTest_${USER}


# Data file used by tests:
#   - refSeq.psl - selected RefSeq  PSLs used to generate genePred, some chosen
#     because of frame problems, etc.
#   - refSeq.cds - CDS for above.
#   - refSeqMin.gp - miminmal genePred, only required columns
#   - refSeqId.gp - with id
#   - refSeqIdName2.gp - with id, name2
#   - refSeqFrame.gp - with id, name2, cdsStat and frame fields (from mrnaToGene)
#   - acembly.gff - data files used to build various tracks
#   - tigr.gff
#   - twinscan.gtf
#   - vegaGene.gtf
#   - vegaPseudo.gtf
#   - ncbi.gff - part GTF part GFF generated by NCBI
#   - incmpl.psl,cds - test creating genePreds where CDS of gene
#     is incomplete on one end or the other.
#   - flybase.gtf - used to test -impliedStopAfterCds

test: fileTests tableTests fromPslTests compatTblTests fromGxfTests

###
# test of reading/writing tab-separated files.
###
fileTests: fileMinTest fileIdTest fileIdName2Test fileFrameTest fileFrameStatTest
doFileTest = ${MAKE} -f genePredTests.mk doFileTest

fileMinTest:
	${doFileTest} id=$@ inGp=refSeqMin.gp

fileIdTest:
	${doFileTest} id=$@ inGp=refSeqId.gp opts="-idFld"

fileIdName2Test:
	${doFileTest} id=$@ inGp=refSeqIdName2.gp opts="-idFld -name2Fld"

fileFrameTest:
	${doFileTest} id=$@ inGp=refSeqFrame.gp opts="-cdsStatFld -exonFramesFld"

# this regress a bug parsing unk cds status, also tests auto-increment (ids are zero)
fileFrameStatTest:
	${doFileTest} id=$@ inGp=refSeqFrameStat.gp opts="-cdsStatFld -exonFramesFld"

# Recurisve target to run a file read/write test.  Will diff the output
# with the input, which should be identical.
# Expects the following variables to be set:
#  id - test id
#  inGp - genePred (omitting dir)
#  opts - genePredTester options to add
doFileTest: mkout
	${GENE_PRED_TESTER} ${opts} -needRows=5 -output=${OUT_DIR}/${id}.gp -info=${OUT_DIR}/${id}.info readFile input/genePred/${inGp}
	diff -u input/genePred/${inGp} ${OUT_DIR}/${id}.gp
	diff -u expected/genePred/${id}.info ${OUT_DIR}/${id}.info
	genePredCheck -verbose=0 ${OUT_DIR}/${id}.gp

###
# Test of loading and reading database tables.  When ids are not auto-assigned,
# then compare with input file, otherwise, we need an expected file.
###
tableTests: tableMinTest tableIdTest tableIdName2Test tableFrameTest tableFrameStatTest
	hgsql -e "drop table if exists ${TEST_TBL}" ${DB}

doTableTest = ${MAKE} -f genePredTests.mk doTableTest

tableMinTest:
	${doTableTest} id=$@ inGp=refSeqMin.gp expGp=input/genePred/refSeqMin.gp

tableIdTest:
	${doTableTest} id=$@ inGp=refSeqId.gp opts="-idFld" expGp=input/genePred/refSeqId.gp

tableIdName2Test:
	${doTableTest} id=$@ inGp=refSeqIdName2.gp opts="-idFld -name2Fld" expGp=input/genePred/refSeqIdName2.gp

tableFrameTest:
	${doTableTest} id=$@ inGp=refSeqFrame.gp opts="-cdsStatFld -exonFramesFld" expGp=expected/genePred/tableFrameTest.gp

tableFrameStatTest:
	${doTableTest} id=$@ inGp=refSeqFrameStat.gp opts="-cdsStatFld -exonFramesFld" expGp=expected/genePred/tableFrameStatTest.gp


# Recurisve target to run a table read/write test.  Will diff the output
# with the input, which should be identical.
# Expects the following variables to be set:
#  id - test id
#  inGp - genePred (omitting dir)
#  expGp - expected gp, full path.
#  opts - genePredTester options to add
doTableTest: mkout
	${GENE_PRED_TESTER} ${opts} -needRows=5 -output=${OUT_DIR}/${id}.gp loadTable ${DB} ${TEST_TBL} input/genePred/${inGp}
	${GENE_PRED_TESTER} ${opts} -needRows=5 -output=${OUT_DIR}/${id}.gp -info=${OUT_DIR}/${id}.info readTable ${DB} ${TEST_TBL}
	diff -u ${expGp} ${OUT_DIR}/${id}.gp
	diff -u expected/genePred/${id}.info ${OUT_DIR}/${id}.info
	genePredCheck -verbose=0 ${OUT_DIR}/${id}.gp

###
# test of genePredFromPsl2 with various various optional fields
###
fromPslTests: fromPslMinTest fromPslCdsFrameTest fromPslIncmplTest
doFromPslTest = ${MAKE} -f genePredTests.mk doFromPslTest

fromPslMinTest:
	${doFromPslTest} id=$@ inBase=refSeq

fromPslCdsFrameTest:
	${doFromPslTest} id=$@ inBase=refSeq opts="-cdsStatFld -exonFramesFld"

# CDS incomplete on one end or the other
fromPslIncmplTest:
	${doFromPslTest} id=$@ inBase=incmpl opts="-cdsStatFld -exonFramesFld"

# Recurisve target to run a file psl to genePred tests
#  id - test id
#  inBase - base name for psl and cds files
#  opts - genePredTester options to add
doFromPslTest: mkout
	${GENE_PRED_TESTER} ${opts} -output=${OUT_DIR}/${id}.gp fromPsl input/genePred/${inBase}.psl input/genePred/${inBase}.cds
	${GENE_PRED_TESTER} ${opts} readFile ${OUT_DIR}/${id}.gp
	diff -u ${EXP_DIR}/${id}.gp ${OUT_DIR}/${id}.gp
	genePredCheck -verbose=0 ${OUT_DIR}/${id}.gp


# tests of reading existing tables; these don't diff output, since tables
# are not stable, but at least see if we can read them
compatTblTests: refFlatTest refGeneTest knownGeneTest
doCompatTblTest = ${MAKE} -f genePredTests.mk doCompatTblTest

refFlatTest:
	${doCompatTblTest} id=$@ tbl=refFlat
refGeneTest:
	${doCompatTblTest} id=$@ tbl=refGene
knownGeneTest:
	${doCompatTblTest} id=$@ tbl=knownGene

# recursive target to read table
# id, tbl
doCompatTblTest: mkout
	${GENE_PRED_TESTER} -maxRows=250 -needRows=250 -info=${OUT_DIR}/${id}.info readTable ${DB} ${tbl}
	diff -u ${EXP_DIR}/${id}.info ${OUT_DIR}/${id}.info

##
# GFF/GTF conversion tests; try bits of files from several different
# sources.
##
fromGxfTests: fromGxfMinTest fromGxfFrameTest \
	fromGxfExonSelTest fromGxfExonSelFrameTest \
	fromGxfVegaTest fromGxfVegaFrameTest fromGxfVegaPseudoTest \
	fromGxfAcemblyTest fromGxfAcemblyFrameTest \
	fromGxfNcbiTest fromGtfRegressTest fromGffRegressTest \
	fromGffCeSangerTest fromGffCeSangerTypeTest \
	fromGtfImpliedStopTest fromGffFrameBug

doFromGxfTest = ${MAKE} -f genePredTests.mk doFromGxfTest

fromGxfMinTest:
	${doFromGxfTest} id=$@ what=fromGtf inBase=twinscan.gtf

fromGxfFrameTest:
	${doFromGxfTest} id=$@ what=fromGtf inBase=twinscan.gtf opts="-cdsStatFld -exonFramesFld -name2Fld"

fromGxfExonSelTest:
	${doFromGxfTest} id=$@ what=fromGff inBase=tigr.gff opts="-exonSelectWord=TC"

fromGxfExonSelFrameTest:
	${doFromGxfTest} id=$@ what=fromGff inBase=tigr.gff opts="-exonSelectWord=TC -cdsStatFld -exonFramesFld"

fromGxfVegaTest:
	${doFromGxfTest} id=$@ what=fromGtf inBase=vegaGene.gtf opts="-name2Fld"

fromGxfVegaFrameTest:
	${doFromGxfTest} id=$@ what=fromGtf inBase=vegaGene.gtf opts="-name2Fld -cdsStatFld -exonFramesFld -idFld"

fromGxfVegaPseudoTest:
	${doFromGxfTest} id=$@ what=fromGtf inBase=vegaPseudo.gtf opts="-name2Fld"

fromGxfAcemblyTest:
	${doFromGxfTest} id=$@ what=fromGff inBase=acembly.gff

fromGxfAcemblyFrameTest:
	${doFromGxfTest} id=$@ what=fromGff inBase=acembly.gff opts="-name2Fld -cdsStatFld -exonFramesFld -idFld"

# this crashed ldHgLoad gene
fromGxfNcbiTest:
	${doFromGxfTest} id=$@ what=fromGff inBase=ncbi.gff opts="-name2Fld -cdsStatFld -exonFramesFld -idFld -exonSelectWord=exon"

# GTF bug regression:
#   - stop_codon wiping out frame on last CDS 
#   - stop_codon being the only codon in an exon didn't get frame right
fromGtfRegressTest:
	${doFromGxfTest} id=$@ what=fromGtf inBase=regress.gtf opts="-cdsStatFld -exonFramesFld"

# GFF bug regressions
#  - none right now
fromGffRegressTest:
###	${doFromGxfTest} id=$@ what=fromGff inBase=regress.gff opts="-cdsStatFld -exonFramesFld -exonSelectWord=exon -ignoreUnconverted"

# ce2 sangerGene GFF bugs
#   - single exon CDS genes with frame column not being set (from ce2/sangerGene)
#   - bogus key name fields that get converted to ids 'ZC101.2c ; Confirmed_by_EST'
# FIXME: this is not currently producing the correct frame information without specifying -exonSelectWork=coding_exon
# probably need some serious work to be able to take in the majority off GFFs.
# FIXME: just disabled checking until fixed
fromGffCeSangerTest:
	${doFromGxfTest} id=$@ what=fromGff inBase=ceSangerGene.gff opts="-noCheck -cdsStatFld -exonFramesFld -exonSelectWord=exon -ignoreUnconverted" readOpts=-noCheck

# ce2 sangerGene GFF bugs
# specifying exon type of coding_exon created only one exon.
fromGffCeSangerTypeTest:
	${doFromGxfTest} id=$@ what=fromGff inBase=ceSangerGene.gff opts="-cdsStatFld -exonFramesFld -ignoreUnconverted -exonSelectWord=coding_exon"

# adding implied stop
fromGtfImpliedStopTest:
	${doFromGxfTest} id=$@ what=fromGtf inBase=flybase.gtf opts="-cdsStatFld -impliedStopAfterCds"

# case where frame was lost
fromGffFrameBug:
	${doFromGxfTest} id=$@ what=fromGff inBase=frameBug.gff opts="-genePredExt -exonSelectWord=exon"

# recursive target for GFF/GTF tests
#  id - test id
#  what - fromGtf or fromGff
#  inFile - base name for GFF/GTF file
#  opts - genePredTester options to add
#  readOpts - special options for readFile
doFromGxfTest: mkout
	${GENE_PRED_TESTER} ${opts} -output=${OUT_DIR}/${id}.gp ${what} input/genePred/${inBase}
	${GENE_PRED_TESTER} ${readOpts} readFile ${OUT_DIR}/${id}.gp
	diff -u ${EXP_DIR}/${id}.gp ${OUT_DIR}/${id}.gp
	genePredCheck -verbose=0 ${OUT_DIR}/${id}.gp

mkout:
	@${MKDIR} ${OUT_DIR}
