# snp.sql was originally generated by the autoSql program, which also 
# generated snp.c and snp.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Polymorphism data from dbSnp XML or genotyping arrays
CREATE TABLE snp (
    bin        smallint(5) unsigned NOT NULL,
    chrom      enum(
                   'unknown',
                   'chr',
                   'chr1',  'chr1_random',
                   'chr2',  'chr2_random',
                   'chr3',  'chr3_random',
                   'chr4',  'chr4_random',
                   'chr5',  'chr5_random',
                   'chr6',  'chr6_random',
                   'chr7',  'chr7_random',
                   'chr8',  'chr8_random',
                   'chr9',  'chr9_random',
                   'chr10', 'chr10_random',
                   'chr11', 'chr11_random',
                   'chr12', 'chr12_random',
                   'chr13', 'chr13_random',
                   'chr14', 'chr14_random',
                   'chr15', 'chr15_random',
                   'chr16', 'chr16_random',
                   'chr17', 'chr17_random',
                   'chr18', 'chr18_random',
                   'chr19', 'chr19_random',
                   'chr20', 'chr20_random',
                   'chr21', 'chr21_random',
                   'chr22', 'chr22_random',
                   'chr23', 'chr23_random',
                   'chr24', 'chr24_random',
                   'chr25', 'chr25_random',
                   'chr26', 'chr26_random',
                   'chr27', 'chr27_random',
                   'chr28', 'chr28_random',
                   'chr29', 'chr29_random',
                   'chr30', 'chr30_random',
                   'chr31', 'chr31_random',
                   'chr32', 'chr32_random',
                   'chr33', 'chr33_random',
                   'chr34', 'chr34_random',
                   'chr35', 'chr35_random',
                   'chr36', 'chr36_random',
                   'chr37', 'chr37_random',
                   'chr38', 'chr38_random',
                   'chr6_hla_hap1',  'chr6_hla_hap2',
                   'chrFinished',
                   'chr2L', 'chr2L_random',
                   'chr2R', 'chr2R_random',
                   'chr2h', 'chr2h_random',
                   'chr3L', 'chr3L_random',
                   'chr3R', 'chr3R_random',
                   'chr3h', 'chr3h_random',
                   'chrI',  'chrII', 'chrIII', 'chrIV',
                   'chrM',  'chrM_random',
                   'chrNA', 'chrNA_random',
                   'chrU',  'chrU_random',
                   'chrUn', 'chrUn_random',
                   'chrV',  'chrV_random',
                   'chrW',  'chrW_random',
                   'chrX',  'chrX_random',
                   'chrXh', 'chrXh_random',
                   'chrY',  'chrY_random',
                   'chrYh', 'chrYh_random',
                   'chrZ',  'chrZ_random'
--                   'CP1',
--                   'MP1',
--                   'chrE22C19W28',
--                   'chrE26C13',
--                   'chrE50C23',
--                   'chrE64',
--                   'ece1',
--                   'pLeu',
--                   'pNRC100',
--                   'pNRC200',
--                   'pO157',
--                   'pOSAK1',
--                   'pSLT',
--                   'pTrp'
               ) DEFAULT 'unknown' NOT NULL,
    chromStart int(10) unsigned NOT NULL,
    chromEnd   int(10) unsigned NOT NULL,
    name       varchar(255) NOT NULL,
    score      int(10) unsigned NOT NULL,
    strand     enum('?','+','-') DEFAULT '?' NOT NULL,
    observed   varchar(255) NOT NULL,
    molType    enum(
                   'unknown',
                   'genomic',
                   'cDNA',
                   'mito',
                   'chloro') DEFAULT 'unknown' NOT NULL,
    class      enum(
                   'unknown',
                   'snp',
                   'in-del',
                   'het',
                   'microsat',
                   'named',
                   'no-variation',
                   'mixed',
                   'mnp') DEFAULT 'unknown' NOT NULL,
    valid      set('unknown',
                   'other-pop',
                   'by-frequency',
                   'by-cluster',
                   'by-2hit-2allele',
                   'by-hapmap',
                   'genotype') DEFAULT 'unknown' NOT NULL,
    avHet      float NOT NULL,
    avHetSE    float NOT NULL,
    func       set(
                   'unknown',
                   'locus-region',
                   'coding',
                   'coding-synon',
                   'coding-nonsynon',
                   'mrna-utr',
                   'intron',
                   'splice-site',
                   'reference',
                   'exception') DEFAULT 'unknown' NOT NULL,
    locType    enum(
                   'unknown',
                   'range',
                   'exact',
                   'between') DEFAULT 'unknown' NOT NULL,
    source     enum(
                   'unknown',
                   'dbSnp',
                   'Affy10K',
                   'Affy10Kv2',
                   'Affy50K_HindIII',
                   'Affy50K_XbaI') DEFAULT 'unknown' NOT NULL,
    exception  set( '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9', '10', 
		   '11', '12', '13', '14', '15', '16', '17', '18', '19', '20', 
		   '21', '22', '23', '24' ) NOT NULL,
    INDEX      chrom      (chrom,bin),
    INDEX      chromStart (chrom,chromStart),
    INDEX      chromEnd   (chrom,chromEnd),
    INDEX      name       (name)
);
