create table snp 
(
	bin		smallint(5) unsigned,
	chrom		enum(
				'unknown',
--				'CP1',
--				'MP1',
				'chr',
				'chr2', 'chr2_random',
				'chr3', 'chr3_random',
				'chr4', 'chr4_random',
				'chr5', 'chr5_random',
				'chr6', 'chr6_random',
				'chr6_hla_hap1',
				'chr6_hla_hap2',
				'chr7', 'chr7_random',
				'chr8', 'chr8_random',
				'chr9', 'chr9_random',
				'chr10','chr10_random',
				'chr11','chr11_random',
				'chr12','chr12_random',
				'chr13','chr13_random',
				'chr14','chr14_random',
				'chr15','chr15_random',
				'chr16','chr16_random',
				'chr17','chr17_random',
				'chr18','chr18_random',
				'chr19','chr19_random',
				'chr20','chr20_random',
				'chr21','chr21_random',
				'chr22','chr22_random',
				'chr23','chr23_random',
				'chr24','chr24_random',
				'chr25','chr25_random',
				'chr26','chr26_random',
				'chr27','chr27_random',
				'chr28','chr28_random',
				'chr29','chr29_random',
				'chr30','chr30_random',
				'chr31','chr31_random',
				'chr32','chr32_random',
				'chr33','chr33_random',
				'chr34','chr34_random',
				'chr35','chr35_random',
				'chr36','chr36_random',
				'chr37','chr37_random',
				'chr38','chr38_random',
				'chr2L','chr2L_random',
				'chr2R','chr2R_random',
				'chr2h','chr2h_random',
				'chr3L','chr3L_random',
				'chr3R','chr3R_random',
				'chr3h','chr3h_random',
--				'chrE22C19W28',
--				'chrE26C13',
--				'chrE50C23',
--				'chrE64',
				'chrFinished',
				'chrI',
				'chrII',
				'chrIII',
				'chrIV',
				'chrM', 'chrM_random',
				'chrNA','chrNA_random',
				'chrU', 'chrU_random',
				'chrUn','chrUn_random',
				'chrV', 'chrV_random',
				'chrW', 'chrW_random',
				'chrX', 'chrX_random',
				'chrXh','chrXh_random',
				'chrY', 'chrY_random',
				'chrYh','chrYh_random',
				'chrZ', 'chrZ_random'
--				'ece1',
--				'pLeu',
--				'pNRC100',
--				'pNRC200',
--				'pO157',
--				'pOSAK1',
--				'pSLT',
--				'pTrp',
				) DEFAULT 'unknown',
	chromStart	int(10) unsigned,
	chromEnd	int(10) unsigned,
	name		varchar(255),
	score		float,
	strand		enum('?','+','-'),
	alleles		varchar(255),
	source		enum(
				'unknown',
				'dbSnp',
				'BAC_OVERLAP',
				'MIXED',
				'RANDOM',
				'OTHER',
				'Affy10K',
				'Affy120K') DEFAULT 'unknown',
	class		enum(
				'unknown',
				'snp',
				'in-del',
				'microsat',
				'named',
				'mnp',
				'mixed') DEFAULT 'unknown',
	valid		set(
				'no-information',
				'by-2hit-2allele',
				'by-cluster',
				'by-frequency',
				'other-pop') DEFAULT 'no-information',
	avHet		float,
	avHetSE		float,
	func		set(
				'unknown',
				'coding-synon',
				'coding-nonsynon',
				'mrna-utr',
				'intron',
				'splice-site',
				'exception',
				'coding',
				'reference',
				'locus-region') DEFAULT 'unknown',
	INDEX 		chrom   (chrom,bin),
	INDEX 		chrom_2 (chrom,chromStart),
	INDEX 		chrom_3 (chrom,chromEnd),
	INDEX 		name    (name)
);
