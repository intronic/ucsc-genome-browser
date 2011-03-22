/* VCF: Variant Call Format, version 4.0 / 4.1
 * http://www.1000genomes.org/wiki/Analysis/Variant%20Call%20Format/vcf-variant-call-format-version-40
 * http://www.1000genomes.org/wiki/Analysis/Variant%20Call%20Format/vcf-variant-call-format-version-41
 * The vcfFile object borrows many memory handling and error reporting tricks from MarkD's
 * gff3File; any local deficiencies are not to reflect poorly on Mark's fine work! :) */

#ifndef vcf_h
#define vcf_h

#include "hash.h"
#include "linefile.h"

enum vcfInfoType
/* VCF header defines INFO column components; each component has one of these types: */
    {
    vcfInfoNoType,	// uninitialized value (0) or unrecognized type name
    vcfInfoInteger,
    vcfInfoFloat,
    vcfInfoFlag,
    vcfInfoCharacter,
    vcfInfoString,
    };

union vcfDatum
/* Container for a value whose type is specified by an enum vcfInfoType. */
    {
    int datInt;
    double datFloat;
    boolean datFlag;
    char datChar;
    char *datString;
    };

struct vcfInfoDef
/* Definition of INFO column component from VCF header: */
    {
    struct vcfInfoDef *next;
    char *key;			// A short identifier, e.g. MQ for mapping quality
    int fieldCount;		// The number of values to follow the id, or -1 if it varies
    enum vcfInfoType type;	// The type of values that follow the id
    char *description;		// Brief description of info
    };

struct vcfInfoElement
/* A single INFO column component; each row's INFO column may contain multiple components. */
    {
    char *key;			// An identifier described by a struct vcfInfoDef
    int count;			// Number of data values following id
    union vcfDatum *values;	// Array of data values following id
    };

struct vcfGenotype
/* A single component of the optional GENOTYPE column. */
    {
    char *id;			// Name of individual/sample (pointer to vcfFile genotypeIds) or .
    unsigned char hapIxA;	// Index of one haplotype's allele: 0=reference, 1=alt, 2=other alt
    unsigned char hapIxB;	// Index of other haplotype's allele
    bool isPhased;		// True if haplotypes are phased
    bool isHaploid;		// True if there is only one haplotype (e.g. chrY)
    int infoCount;		// Number of components named in FORMAT column
    struct vcfInfoElement *infoElements;	// Array of info components
    };

struct vcfRecord
/* A VCF data row (or list of rows). */
{
    struct vcfRecord *next;
    char *chrom;		// Reference assembly sequence name
    unsigned int chromStart;	// Start offset in chrom
    unsigned int chromEnd;	// End offset in chrom
    char *name;			// Variant name from ID column
    char *ref;			// Allele found in reference assembly
    char *alt;			// Alternate allele(s)
    float qual;			// Phred-scaled score, i.e. -10log_10 P(call in ALT is wrong)
    char *filter;		// Either PASS or code(s) described in header for failed filters
    int infoCount;		// Number of components of INFO column
    struct vcfInfoElement *infoElements;	// Array of INFO column components
    char *format;		// Optional column containing ordered list of genotype components
    char **genotypeUnparsedStrings;	// Array of unparsed optional genotype columns
    struct vcfGenotype *genotypes;	// If built, array of parsed genotype components
    struct vcfFile *file;	// Pointer back to parent vcfFile
};

struct vcfFile
/* Info extracted from a VCF file.  Manages all memory for contents.
 * Clearly borrowing structure from MarkD's gff3File. :) */
{
    char *fileOrUrl;		// VCF local file path or URL
    struct hash *metaDataHash;	// Store all header metadata lines here
    int majorVersion;		// 4 etc.
    int minorVersion;		// 0, 1 etc.
    struct vcfInfoDef *infoDefs;	// Header's definitions of INFO column components
    struct vcfInfoDef *filterDefs;	// Header's definitions of FILTER column failure codes
    struct vcfInfoDef *altDefs;	// Header's defs of symbolic alternate alleles (e.g. DEL, INS)
    struct vcfInfoDef *gtFormatDefs;	// Header's defs of GENOTYPE compnts. listed in FORMAT col.
    int genotypeCount;		// Number of optional genotype columns described in header
    char **genotypeIds;		// Array of optional genotype column names described in header
    struct vcfRecord *records;	// VCF data rows, sorted by position
    struct hash *byName;		// Hash records by name -- not populated until needed.
    struct hash *pool;		// Used to allocate string values that tend to
				// be repeated in the files.  hash's localMem is also 
				// use to allocated memory for all other objects.
    struct lineFile *lf;	// Used only during parsing
    FILE *errFh;		// Write errors to this file
    int maxErr;			// Maximum number of errors before aborting
    int errCnt;			// Error count
};

/* Reserved but optional INFO keys: */
extern const char *vcfInfoAncestralAllele;
extern const char *vcfInfoPerAlleleGtCount;	// allele count in genotypes, for each ALT allele,
						// in the same order as listed
extern const char *vcfInfoAlleleFrequency;  	// allele frequency for each ALT allele in the same
						// order as listed: use this when estimated from
						// primary data, not called genotypes
extern const char *vcfInfoNumAlleles;		// total number of alleles in called genotypes
extern const char *vcfInfoBaseQuality;		// RMS base quality at this position
extern const char *vcfInfoCigar;		// cigar string describing how to align an
						// alternate allele to the reference allele
extern const char *vcfInfoIsDbSnp;		// dbSNP membership
extern const char *vcfInfoDepth;		// combined depth across samples, e.g. DP=154
extern const char *vcfInfoEnd;			// end position of the variant described in this
						// record (esp. for CNVs)
extern const char *vcfInfoIsHapMap2;		// membership in hapmap2
extern const char *vcfInfoIsHapMap3;		// membership in hapmap3
extern const char *vcfInfoIs1000Genomes;	// membership in 1000 Genomes
extern const char *vcfInfoMappingQuality;	// RMS mapping quality, e.g. MQ=52
extern const char *vcfInfoMapQual0Count;	// number of MAPQ == 0 reads covering this record
extern const char *vcfInfoNumSamples;		// Number of samples with data
extern const char *vcfInfoStrandBias;		// strand bias at this position
extern const char *vcfInfoIsSomatic;		// indicates that the record is a somatic mutation,
						// for cancer genomics
extern const char *vcfInfoIsValidated;		// validated by follow-up experiment

/* Reserved but optional per-genotype keys: */
extern const char *vcfGtGenotypes;	// numeric allele values separated by "/" (unphased)
					// or "|" (phased). Allele values are 0 for
					// reference allele, 1 for the first allele in ALT,
					// 2 for the second allele in ALT and so on.
extern const char *vcfGtDepth;		// read depth at this position for this sample
extern const char *vcfGtFilter;		// analogous to variant's FILTER field
extern const char *vcfGtLikelihoods;	// three floating point log10-scaled likelihoods for
					// AA,AB,BB genotypes where A=ref and B=alt;
					// not applicable if site is not biallelic.
extern const char *vcfGtPhred;		// Phred-scaled genotype likelihoods rounded to closest int
extern const char *vcfGtConditionalQual;	// Conditional genotype quality
					// i.e. phred quality -10log_10 P(genotype call is wrong,
					// conditioned on the site's being variant)
extern const char *vcfGtHaplotypeQualities;	// Two phred qualities comma separated
extern const char *vcfGtPhaseSet;	// Set of phased genotypes to which this genotype belongs
extern const char *vcfGtPhasingQuality;	// Phred-scaled P(alleles ordered wrongly in heterozygote)
extern const char *vcfGtExpectedAltAlleleCount;	// Typically used in association analyses

struct vcfFile *vcfFileMayOpen(char *fileOrUrl, int maxErr, FILE *errFh);
/* Parse a VCF file into a vcfFile object; return NULL if unable.
 * If maxErr not zero, then continue to parse until this number of error have been reached.
 * A maxErr less than zero does not stop and reports all errors. Write errors to errFh,
 * if NULL, use stderr. */

struct vcfFile *vcfTabixFileMayOpen(char *fileOrUrl, char *chrom, int start, int end,
				    int maxErr, FILE *errFh);
/* Parse header and rows within the given position range from a VCF file that has been
 * compressed and indexed by tabix into a vcfFile object; return NULL if or if file has
 * no items in range.
 * If maxErr not zero, then continue to parse until this number of error have been reached.
 * A maxErr less than zero does not stop and reports all errors. Write errors to errFh,
 * if NULL, use stderr. */

void vcfFileFree(struct vcfFile **vcffPtr);
/* Free a vcfFile object. */

const struct vcfRecord *vcfFileFindVariant(struct vcfFile *vcff, char *variantId);
/* Return all records with name=variantId, or NULL if not found. */

const struct vcfInfoElement *vcfRecordFindInfo(const struct vcfRecord *record, char *key);
/* Find an INFO element, or NULL. */

const struct vcfGenotype *vcfRecordFindGenotype(struct vcfRecord *record, char *sampleId);
/* Find the genotype and associated info for the individual, or return NULL. */

#endif // vcf_h
