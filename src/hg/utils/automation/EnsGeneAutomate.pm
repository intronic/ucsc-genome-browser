# EnsGeneAutomate: mapping Ensembl species/gff file names to UCSC db
#
# DO NOT EDIT the /cluster/bin/scripts copy of this file --
# edit ~/kent/src/hg/utils/automation/EnsGeneAutomate.pm instead.

# $Id: EnsGeneAutomate.pm,v 1.18 2009/07/14 22:26:08 hiram Exp $
package EnsGeneAutomate;

use warnings;
use strict;
use Carp;
use vars qw(@ISA @EXPORT_OK);
use Exporter;

@ISA = qw(Exporter);

# This is a listing of the public methods and variables (which should be
# treated as constants) exported by this module:
@EXPORT_OK = (
    # Support for common command line options:
    qw( ensVersionList ensGeneVersioning
      ),
);

#	Location of Ensembl FTP site for the releases
my $ensemblFTP = "ftp://ftp.ensembl.org/pub/";

#	version to date relationship for Ensembl archive reference
my @verToDate;
$verToDate[27] = "dec2004";
$verToDate[32] = "jul2005";
$verToDate[33] = "sep2005";
$verToDate[34] = "oct2005";
$verToDate[35] = "nov2005";
$verToDate[37] = "feb2006";
$verToDate[38] = "apr2006";
$verToDate[39] = "jun2006";
$verToDate[41] = "oct2006";
$verToDate[42] = "dec2006";
# before 43 there are no GTF files
$verToDate[43] = "feb2007";
$verToDate[44] = "apr2007";
$verToDate[45] = "jun2007";
$verToDate[46] = "aug2007";
$verToDate[47] = "oct2007";
$verToDate[48] = "dec2007";
$verToDate[49] = "mar2008";
$verToDate[50] = "jul2008";
$verToDate[51] = "nov2008";
$verToDate[52] = "dec2008";
$verToDate[55] = "jul2009";

#	older versions for archive purposes, there are different
#	directory structures for these, thus, the full path name
#	to append to the release-NN/ top level directory.
#  Fugu fr1 needs help here since there is no GTF file, fetch it from
#	EnsMart
my %ensGeneGtfFileNames_35 = (
'fr1' => 'fugu_rubripes_35_2g/data/fasta/dna/README',
);
my %ensGeneFtpPeptideFileNames_35 = (
'fr1' => 'fugu_rubripes_35_2g/data/fasta/pep/Fugu_rubripes.FUGU2.nov.pep.fa.gz',
);
my %ensGeneFtpMySqlFileNames_35 = (
'fr1' => 'fugu_rubripes_35_2g/data/mysql/fugu_rubripes_core_35_2g/assembly.txt.table.gz',
);

my %ensGeneGtfFileNames_46 = (
'mm8' => 'mus_musculus_46_36g/data/gtf/Mus_musculus.NCBIM36.46.gtf.gz',
);
my %ensGeneFtpPeptideFileNames_46 = (
'mm8' => 'mus_musculus_46_36g/data/fasta/pep/Mus_musculus.NCBIM36.46.pep.all.fa.gz',
);
my %ensGeneFtpMySqlFileNames_46 = (
'mm8' => 'mus_musculus_46_36g/data/mysql/mus_musculus_core_46_36g',
);

#	This listings are created by going to the FTP site and running
#	an ls on the gtf directory.  Edit that listing into this hash.
# key is UCSC db name, result is FTP file name under the gtf directory
my %ensGeneGtfFileNames_47 = (
'aedAeg0' => 'Aedes_aegypti.AaegL1.47.gtf.gz',
'anoGam2' => 'Anopheles_gambiae.AgamP3.47.gtf.gz',
'bosTau3' => 'Bos_taurus.Btau_3.1.47.gtf.gz',
'ce5' => 'Caenorhabditis_elegans.WS180.47.gtf.gz',
'canFam2' => 'Canis_familiaris.BROADD2.47.gtf.gz',
'cavPor2' => 'Cavia_porcellus.GUINEAPIG.47.gtf.gz',
'ci2' => 'Ciona_intestinalis.JGI2.47.gtf.gz',
'cioSav2' => 'Ciona_savignyi.CSAV2.0.47.gtf.gz',
'danRer5' => 'Danio_rerio.ZFISH7.47.gtf.gz',
'dasNov1' => 'Dasypus_novemcinctus.ARMA.47.gtf.gz',
'dm4' => 'Drosophila_melanogaster.BDGP4.3.47.gtf.gz',
'echTel1' => 'Echinops_telfairi.TENREC.47.gtf.gz',
'eriEur1' => 'Erinaceus_europaeus.HEDGEHOG.47.gtf.gz',
'felCat3' => 'Felis_catus.CAT.47.gtf.gz',
'galGal3' => 'Gallus_gallus.WASHUC2.47.gtf.gz',
'gasAcu1' => 'Gasterosteus_aculeatus.BROADS1.47.gtf.gz',
'hg18' => 'Homo_sapiens.NCBI36.47.gtf.gz',
'loxAfr1' => 'Loxodonta_africana.BROADE1.47.gtf.gz',
'rheMac2' => 'Macaca_mulatta.MMUL_1.47.gtf.gz',
'monDom5' => 'Monodelphis_domestica.BROADO5.47.gtf.gz',
'mm9' => 'Mus_musculus.NCBIM37.47.gtf.gz',
'myoLuc1' => 'Myotis_lucifugus.MICROBAT1.47.gtf.gz',
'ornAna1' => 'Ornithorhynchus_anatinus.OANA5.47.gtf.gz',
'oryCun1' => 'Oryctolagus_cuniculus.RABBIT.47.gtf.gz',
'oryLat1' => 'Oryzias_latipes.MEDAKA1.47.gtf.gz',
'otoGar1' => 'Otolemur_garnettii.BUSHBABY1.47.gtf.gz',
'panTro2' => 'Pan_troglodytes.CHIMP2.1.47.gtf.gz',
'rn4' => 'Rattus_norvegicus.RGSC3.4.47.gtf.gz',
'sacCer1' => 'Saccharomyces_cerevisiae.SGD1.01.47.gtf.gz',
'sorAra0' => 'Sorex_araneus.COMMON_SHREW1.47.gtf.gz',
'speTri0' => 'Spermophilus_tridecemlineatus.SQUIRREL.47.gtf.gz',
'fr2' => 'Takifugu_rubripes.FUGU4.47.gtf.gz',
'tetNig1' => 'Tetraodon_nigroviridis.TETRAODON7.47.gtf.gz',
'tupBel1' => 'Tupaia_belangeri.TREESHREW.47.gtf.gz',
'xenTro2' => 'Xenopus_tropicalis.JGI4.1.47.gtf.gz',
);

my %ensGeneFtpPeptideFileNames_47 = (
'aedAeg0' => 'aedes_aegypti_47_1a/pep/Aedes_aegypti.AaegL1.47.pep.all.fa.gz',
'anoGam2' => 'anopheles_gambiae_47_3i/pep/Anopheles_gambiae.AgamP3.47.pep.all.fa.gz',
'bosTau3' => 'bos_taurus_47_3d/pep/Bos_taurus.Btau_3.1.47.pep.all.fa.gz',
'ce5' => 'caenorhabditis_elegans_47_180/pep/Caenorhabditis_elegans.WS180.47.pep.all.fa.gz',
'canFam2' => 'canis_familiaris_47_2e/pep/Canis_familiaris.BROADD2.47.pep.all.fa.gz',
'cavPor2' => 'cavia_porcellus_47_1b/pep/Cavia_porcellus.GUINEAPIG.47.pep.all.fa.gz',
'ci2' => 'ciona_intestinalis_47_2g/pep/Ciona_intestinalis.JGI2.47.pep.all.fa.gz',
'cioSav2' => 'ciona_savignyi_47_2e/pep/Ciona_savignyi.CSAV2.0.47.pep.all.fa.gz',
'danRer5' => 'danio_rerio_47_7a/pep/Danio_rerio.ZFISH7.47.pep.all.fa.gz',
'dasNov1' => 'dasypus_novemcinctus_47_1d/pep/Dasypus_novemcinctus.ARMA.47.pep.all.fa.gz',
'dm4' => 'drosophila_melanogaster_47_43b/pep/Drosophila_melanogaster.BDGP4.3.47.pep.all.fa.gz',
'echTel1' => 'echinops_telfairi_47_1d/pep/Echinops_telfairi.TENREC.47.pep.all.fa.gz',
'eriEur1' => 'erinaceus_europaeus_47_1b/pep/Erinaceus_europaeus.HEDGEHOG.47.pep.all.fa.gz',
'felCat3' => 'felis_catus_47_1b/pep/Felis_catus.CAT.47.pep.all.fa.gz',
'galGal3' => 'gallus_gallus_47_2e/pep/Gallus_gallus.WASHUC2.47.pep.all.fa.gz',
'gasAcu1' => 'gasterosteus_aculeatus_47_1d/pep/Gasterosteus_aculeatus.BROADS1.47.pep.all.fa.gz',
'hg18' => 'homo_sapiens_47_36i/pep/Homo_sapiens.NCBI36.47.pep.all.fa.gz',
'loxAfr1' => 'loxodonta_africana_47_1c/pep/Loxodonta_africana.BROADE1.47.pep.all.fa.gz',
'rheMac2' => 'macaca_mulatta_47_10f/pep/Macaca_mulatta.MMUL_1.47.pep.all.fa.gz',
'monDom5' => 'monodelphis_domestica_47_5b/pep/Monodelphis_domestica.BROADO5.47.pep.all.fa.gz',
'mm9' => 'mus_musculus_47_37/pep/Mus_musculus.NCBIM37.47.pep.all.fa.gz',
'myoLuc0' => 'myotis_lucifugus_47_1c/pep/Myotis_lucifugus.MICROBAT1.47.pep.all.fa.gz',
'ornAna1' => 'ornithorhynchus_anatinus_47_1d/pep/Ornithorhynchus_anatinus.OANA5.47.pep.all.fa.gz',
'oryCun1' => 'oryctolagus_cuniculus_47_1d/pep/Oryctolagus_cuniculus.RABBIT.47.pep.all.fa.gz',
'oryLat1' => 'oryzias_latipes_47_1c/pep/Oryzias_latipes.MEDAKA1.47.pep.all.fa.gz',
'otoGar1' => 'otolemur_garnettii_47_1a/pep/Otolemur_garnettii.BUSHBABY1.47.pep.all.fa',
'panTro2' => 'pan_troglodytes_47_21f/pep/Pan_troglodytes.CHIMP2.1.47.pep.all.fa.gz ',
'rn4' => 'rattus_norvegicus_47_34q/pep/Rattus_norvegicus.RGSC3.4.47.pep.all.fa.gz',
'sacCer1' => 'saccharomyces_cerevisiae_47_1g/pep/Saccharomyces_cerevisiae.SGD1.01.47.pep.all.fa.gz',
'sorAra0' => 'sorex_araneus_47_1a/pep/Sorex_araneus.COMMON_SHREW1.47.pep.all.fa.gz',
'speTri0' => 'spermophilus_tridecemlineatus_47_1c/pep/Spermophilus_tridecemlineatus.SQUIRREL.47.pep.all.fa.gz',
'fr2' => 'takifugu_rubripes_47_4g/pep/Takifugu_rubripes.FUGU4.47.pep.all.fa.gz',
'tetNig1' => 'tetraodon_nigroviridis_47_1i/pep/Tetraodon_nigroviridis.TETRAODON7.47.pep.all.fa.gz',
'tupBel1' => 'tupaia_belangeri_47_1b/pep/Tupaia_belangeri.TREESHREW.47.pep.all.fa.gz',
'xenTro2' => 'xenopus_tropicalis_47_41g/pep/Xenopus_tropicalis.JGI4.1.47.pep.all.fa.gz',
);

#	directory name under release-47/mysql/ to find 'seq_region' and
#	'assembly' table copies for GeneScaffold coordinate conversions
my %ensGeneFtpMySqlFileNames_47 = (
'aedAeg0' => 'aedes_aegypti_core_47_1a',
'anoGam2' => 'anopheles_gambiae_core_47_3i',
'bosTau3' => 'bos_taurus_core_47_3d',
'ce5' => 'caenorhabditis_elegans_core_47_180',
'canFam2' => 'canis_familiaris_core_47_2e',
'cavPor2' => 'cavia_porcellus_core_47_1b',
'ci2' => 'ciona_intestinalis_core_47_2g',
'cioSav2' => 'ciona_savignyi_core_47_2e',
'danRer5' => 'danio_rerio_core_47_7a',
'dasNov1' => 'dasypus_novemcinctus_core_47_1d',
'dm4' => 'drosophila_melanogaster_core_47_43b',
'echTel1' => 'echinops_telfairi_core_47_1d',
'eriEur1' => 'erinaceus_europaeus_core_47_1b',
'felCat3' => 'felis_catus_core_47_1b',
'galGal3' => 'gallus_gallus_core_47_2e',
'gasAcu1' => 'gasterosteus_aculeatus_core_47_1d',
'hg18' => 'homo_sapiens_core_47_36i',
'loxAfr1' => 'loxodonta_africana_core_47_1c',
'rheMac2' => 'macaca_mulatta_core_47_10f',
'monDom5' => 'monodelphis_domestica_core_47_5b',
'mm9' => 'mus_musculus_core_47_37',
'myoLuc0' => 'myotis_lucifugus_core_47_1c',
'ornAna1' => 'ornithorhynchus_anatinus_core_47_1d',
'oryCun1' => 'oryctolagus_cuniculus_core_47_1d',
'oryLat1' => 'oryzias_latipes_core_47_1c',
'otoGar1' => 'otolemur_garnettii_core_47_1a',
'panTro2' => 'pan_troglodytes_core_47_21f',
'rn4' => 'rattus_norvegicus_core_47_34q',
'sacCer1' => 'saccharomyces_cerevisiae_core_47_1g',
'sorAra0' => 'sorex_araneus_core_47_1a',
'speTri0' => 'spermophilus_tridecemlineatus_core_47_1c',
'fr2' => 'takifugu_rubripes_core_47_4g',
'tetNig1' => 'tetraodon_nigroviridis_core_47_1i',
'tupBel1' => 'tupaia_belangeri_core_47_1b',
'xenTro2' => 'xenopus_tropicalis_core_47_41g',
);

# key is UCSC db name, result is FTP file name under the gtf directory
my %ensGeneGtfFileNames_48 = (
'aedAeg0' => 'aedes_aegypti/Aedes_aegypti.AaegL1.48.gtf.gz',
'anoGam2' => 'anopheles_gambiae/Anopheles_gambiae.AgamP3.48.gtf.gz',
'bosTau3' => 'bos_taurus/Bos_taurus.Btau_3.1.48.gtf.gz',
'ce5' => 'caenorhabditis_elegans/Caenorhabditis_elegans.WS180.48.gtf.gz',
'canFam2' => 'canis_familiaris/Canis_familiaris.BROADD2.48.gtf.gz',
'cavPor2' => 'cavia_porcellus/Cavia_porcellus.GUINEAPIG.48.gtf.gz',
'ci2' => 'ciona_intestinalis/Ciona_intestinalis.JGI2.48.gtf.gz',
'cioSav2' => 'ciona_savignyi/Ciona_savignyi.CSAV2.0.48.gtf.gz',
'danRer5' => 'danio_rerio/Danio_rerio.ZFISH7.48.gtf.gz',
'dasNov1' => 'dasypus_novemcinctus/Dasypus_novemcinctus.ARMA.48.gtf.gz',
'dm4' => 'drosophila_melanogaster/Drosophila_melanogaster.BDGP4.3.48.gtf.gz',
'echTel1' => 'echinops_telfairi/Echinops_telfairi.TENREC.48.gtf.gz',
'eriEur1' => 'erinaceus_europaeus/Erinaceus_europaeus.HEDGEHOG.48.gtf.gz',
'felCat3' => 'felis_catus/Felis_catus.CAT.48.gtf.gz',
'galGal3' => 'gallus_gallus/Gallus_gallus.WASHUC2.48.gtf.gz',
'gasAcu1' => 'gasterosteus_aculeatus/Gasterosteus_aculeatus.BROADS1.48.gtf.gz',
'hg18' => 'homo_sapiens/Homo_sapiens.NCBI36.48.gtf.gz',
'loxAfr1' => 'loxodonta_africana/Loxodonta_africana.BROADE1.48.gtf.gz',
'rheMac2' => 'macaca_mulatta/Macaca_mulatta.MMUL_1.48.gtf.gz',
'micMur0' => 'microcebus_murinus/Microcebus_murinus.micMur1.48.gtf.gz',
'monDom5' => 'monodelphis_domestica/Monodelphis_domestica.BROADO5.48.gtf.gz',
'mm9' => 'mus_musculus/Mus_musculus.NCBIM37.48.gtf.gz',
'myoLuc0' => 'myotis_lucifugus/Myotis_lucifugus.MICROBAT1.48.gtf.gz',
'ochPri0' => 'ochotona_princeps/Ochotona_princeps.pika.48.gtf.gz',
'ornAna1' => 'ornithorhynchus_anatinus/Ornithorhynchus_anatinus.OANA5.48.gtf.gz',
'oryCun1' => 'oryctolagus_cuniculus/Oryctolagus_cuniculus.RABBIT.48.gtf.gz',
'oryLat1' => 'oryzias_latipes/Oryzias_latipes.MEDAKA1.48.gtf.gz',
'otoGar1' => 'otolemur_garnettii/Otolemur_garnettii.BUSHBABY1.48.gtf.gz',
'panTro2' => 'pan_troglodytes/Pan_troglodytes.CHIMP2.1.48.gtf.gz',
'rn4' => 'rattus_norvegicus/Rattus_norvegicus.RGSC3.4.48.gtf.gz',
'sacCer1' => 'saccharomyces_cerevisiae/Saccharomyces_cerevisiae.SGD1.48.gtf.gz',
'sorAra0' => 'sorex_araneus/Sorex_araneus.COMMON_SHREW1.48.gtf.gz',
'speTri0' => 'spermophilus_tridecemlineatus/Spermophilus_tridecemlineatus.SQUIRREL.48.gtf.gz',
'fr2' => 'takifugu_rubripes/Takifugu_rubripes.FUGU4.48.gtf.gz',
'tetNig1' => 'tetraodon_nigroviridis/Tetraodon_nigroviridis.TETRAODON7.48.gtf.gz',
'tupBel1' => 'tupaia_belangeri/Tupaia_belangeri.TREESHREW.48.gtf.gz',
'xenTro2' => 'xenopus_tropicalis/Xenopus_tropicalis.JGI4.1.48.gtf.gz',
);

# key is UCSC db name, result is FTP file name under the fasta directory
my %ensGeneFtpPeptideFileNames_48 = (
'aedAeg0' => 'aedes_aegypti/pep/Aedes_aegypti.AaegL1.48.pep.all.fa.gz',
'anoGam2' => 'anopheles_gambiae/pep/Anopheles_gambiae.AgamP3.48.pep.all.fa.gz',
'bosTau3' => 'bos_taurus/pep/Bos_taurus.Btau_3.1.48.pep.all.fa.gz',
'ce5' => 'caenorhabditis_elegans/pep/Caenorhabditis_elegans.WS180.48.pep.all.fa.gz',
'canFam2' => 'canis_familiaris/pep/Canis_familiaris.BROADD2.48.pep.all.fa.gz',
'cavPor2' => 'cavia_porcellus/pep/Cavia_porcellus.GUINEAPIG.48.pep.all.fa.gz',
'ci2' => 'ciona_intestinalis/pep/Ciona_intestinalis.JGI2.48.pep.all.fa.gz',
'cioSav2' => 'ciona_savignyi/pep/Ciona_savignyi.CSAV2.0.48.pep.all.fa.gz',
'danRer5' => 'danio_rerio/pep/Danio_rerio.ZFISH7.48.pep.all.fa.gz',
'dasNov1' => 'dasypus_novemcinctus/pep/Dasypus_novemcinctus.ARMA.48.pep.all.fa.gz',
'dm4' => 'drosophila_melanogaster/pep/Drosophila_melanogaster.BDGP4.3.48.pep.all.fa.gz',
'echTel1' => 'echinops_telfairi/pep/Echinops_telfairi.TENREC.48.pep.all.fa.gz',
'eriEur1' => 'erinaceus_europaeus/pep/Erinaceus_europaeus.HEDGEHOG.48.pep.all.fa.gz',
'felCat3' => 'felis_catus/pep/Felis_catus.CAT.48.pep.all.fa.gz',
'galGal3' => 'gallus_gallus/pep/Gallus_gallus.WASHUC2.48.pep.all.fa.gz',
'gasAcu1' => 'gasterosteus_aculeatus/pep/Gasterosteus_aculeatus.BROADS1.48.pep.all.fa.gz',
'hg18' => 'homo_sapiens/pep/Homo_sapiens.NCBI36.48.pep.all.fa.gz',
'loxAfr1' => 'loxodonta_africana/pep/Loxodonta_africana.BROADE1.48.pep.all.fa.gz',
'rheMac2' => 'macaca_mulatta/pep/Macaca_mulatta.MMUL_1.48.pep.all.fa.gz',
'micMur0' => 'microcebus_murinus/pep/Microcebus_murinus.micMur1.48.pep.all.fa.gz',
'monDom5' => 'monodelphis_domestica/pep/Monodelphis_domestica.BROADO5.48.pep.all.fa.gz',
'mm9' => 'mus_musculus/pep/Mus_musculus.NCBIM37.48.pep.all.fa.gz',
'myoLuc0' => 'myotis_lucifugus/pep/Myotis_lucifugus.MICROBAT1.48.pep.all.fa.gz',
'ochPri0' => 'ochotona_princeps/pep/Ochotona_princeps.pika.48.pep.all.fa.gz',
'ornAna1' => 'ornithorhynchus_anatinus/pep/Ornithorhynchus_anatinus.OANA5.48.pep.all.fa.gz',
'oryCun1' => 'oryctolagus_cuniculus/pep/Oryctolagus_cuniculus.RABBIT.48.pep.all.fa.gz',
'oryLat1' => 'oryzias_latipes/pep/Oryzias_latipes.MEDAKA1.48.pep.all.fa.gz',
'otoGar1' => 'otolemur_garnettii/pep/Otolemur_garnettii.BUSHBABY1.48.pep.all.fa.gz',
'panTro2' => 'pan_troglodytes/pep/Pan_troglodytes.CHIMP2.1.48.pep.all.fa.gz',
'rn4' => 'rattus_norvegicus/pep/Rattus_norvegicus.RGSC3.4.48.pep.all.fa.gz',
'sacCer1' => 'saccharomyces_cerevisiae/pep/Saccharomyces_cerevisiae.SGD1.48.pep.all.fa.gz',
'sorAra0' => 'sorex_araneus/pep/Sorex_araneus.COMMON_SHREW1.48.pep.all.fa.gz',
'speTri0' => 'spermophilus_tridecemlineatus/pep/Spermophilus_tridecemlineatus.SQUIRREL.48.pep.all.fa.gz',
'fr2' => 'takifugu_rubripes/pep/Takifugu_rubripes.FUGU4.48.pep.all.fa.gz',
'tetNig1' => 'tetraodon_nigroviridis/pep/Tetraodon_nigroviridis.TETRAODON7.48.pep.all.fa.gz',
'tupBel1' => 'tupaia_belangeri/pep/Tupaia_belangeri.TREESHREW.48.pep.all.fa.gz',
'xenTro2' => 'xenopus_tropicalis/pep/Xenopus_tropicalis.JGI4.1.48.pep.all.fa.gz',
);

#	directory name under release-48/mysql/ to find 'seq_region' and
#	'assembly' table copies for GeneScaffold coordinate conversions
my %ensGeneFtpMySqlFileNames_48 = (
'aedAeg0' => 'aedes_aegypti_core_48_1b',
'anoGam2' => 'anopheles_gambiae_core_48_3j',
'bosTau3' => 'bos_taurus_core_48_3e',
'ce5' => 'caenorhabditis_elegans_core_48_180a',
'canFam2' => 'canis_familiaris_core_48_2f',
'cavPor2' => 'cavia_porcellus_core_48_1c',
'ci2' => 'ciona_intestinalis_core_48_2h',
'cioSav2' => 'ciona_savignyi_core_48_2f',
'danRer5' => 'danio_rerio_core_48_7b',
'dasNov1' => 'dasypus_novemcinctus_core_48_1e',
'dm4' => 'drosophila_melanogaster_core_48_43b',
'echTel1' => 'echinops_telfairi_core_48_1e',
'eriEur1' => 'erinaceus_europaeus_core_48_1c',
'felCat3' => 'felis_catus_core_48_1c',
'galGal3' => 'gallus_gallus_core_48_2f',
'gasAcu1' => 'gasterosteus_aculeatus_core_48_1e',
'hg18' => 'homo_sapiens_core_48_36j',
'loxAfr1' => 'loxodonta_africana_core_48_1d',
'rheMac2' => 'macaca_mulatta_core_48_10g',
'micMur0' => 'microcebus_murinus_core_48_1',
'monDom5' => 'monodelphis_domestica_core_48_5c',
'mm9' => 'mus_musculus_core_48_37a',
'myoLuc0' => 'myotis_lucifugus_core_48_1d',
'ochPri0' => 'ochotona_princeps_core_48_1',
'ornAna1' => 'ornithorhynchus_anatinus_core_48_1e',
'oryCun1' => 'oryctolagus_cuniculus_core_48_1e',
'oryLat1' => 'oryzias_latipes_core_48_1d',
'otoGar1' => 'otolemur_garnettii_core_48_1b',
'panTro2' => 'pan_troglodytes_core_48_21g',
'rn4' => 'rattus_norvegicus_core_48_34r',
'sacCer1' => 'saccharomyces_cerevisiae_core_48_1h',
'sorAra0' => 'sorex_araneus_core_48_1b',
'speTri0' => 'spermophilus_tridecemlineatus_core_48_1d',
'fr2' => 'takifugu_rubripes_core_48_4h',
'tetNig1' => 'tetraodon_nigroviridis_core_48_1j',
'tupBel1' => 'tupaia_belangeri_core_48_1c',
'xenTro2' => 'xenopus_tropicalis_core_48_41h',
);

# key is UCSC db name, result is FTP file name under the gtf directory
my %ensGeneGtfFileNames_49 = (
'aedAeg0' => 'aedes_aegypti/Aedes_aegypti.AaegL1.49.gtf.gz',
'anoGam2' => 'anopheles_gambiae/Anopheles_gambiae.AgamP3.49.gtf.gz',
'bosTau3' => 'bos_taurus/Bos_taurus.Btau_3.1.49.gtf.gz',
'ce5' => 'caenorhabditis_elegans/Caenorhabditis_elegans.WS180.49.gtf.gz',
'canFam2' => 'canis_familiaris/Canis_familiaris.BROADD2.49.gtf.gz',
'cavPor2' => 'cavia_porcellus/Cavia_porcellus.GUINEAPIG.49.gtf.gz',
'ci2' => 'ciona_intestinalis/Ciona_intestinalis.JGI2.49.gtf.gz',
'cioSav2' => 'ciona_savignyi/Ciona_savignyi.CSAV2.0.49.gtf.gz',
'danRer5' => 'danio_rerio/Danio_rerio.ZFISH7.49.gtf.gz',
'dasNov1' => 'dasypus_novemcinctus/Dasypus_novemcinctus.ARMA.49.gtf.gz',
'dm5' => 'drosophila_melanogaster/Drosophila_melanogaster.BDGP5.4.49.gtf.gz',
'echTel1' => 'echinops_telfairi/Echinops_telfairi.TENREC.49.gtf.gz',
'equCab2' => 'equus_caballus/Equus_caballus.EquCab2.49.gtf.gz',
'eriEur1' => 'erinaceus_europaeus/Erinaceus_europaeus.HEDGEHOG.49.gtf.gz',
'felCat3' => 'felis_catus/Felis_catus.CAT.49.gtf.gz',
'galGal3' => 'gallus_gallus/Gallus_gallus.WASHUC2.49.gtf.gz',
'gasAcu1' => 'gasterosteus_aculeatus/Gasterosteus_aculeatus.BROADS1.49.gtf.gz',
'hg18' => 'homo_sapiens/Homo_sapiens.NCBI36.49.gtf.gz',
'loxAfr1' => 'loxodonta_africana/Loxodonta_africana.BROADE1.49.gtf.gz',
'rheMac2' => 'macaca_mulatta/Macaca_mulatta.MMUL_1.49.gtf.gz',
'micMur0' => 'microcebus_murinus/Microcebus_murinus.micMur1.49.gtf.gz',
'monDom5' => 'monodelphis_domestica/Monodelphis_domestica.BROADO5.49.gtf.gz',
'mm9' => 'mus_musculus/Mus_musculus.NCBIM37.49.gtf.gz',
'myoLuc1' => 'myotis_lucifugus/Myotis_lucifugus.MICROBAT1.49.gtf.gz',
'ochPri0' => 'ochotona_princeps/Ochotona_princeps.pika.49.gtf.gz',
'ornAna1' => 'ornithorhynchus_anatinus/Ornithorhynchus_anatinus.OANA5.49.gtf.gz',
'oryCun1' => 'oryctolagus_cuniculus/Oryctolagus_cuniculus.RABBIT.49.gtf.gz',
'oryLat1' => 'oryzias_latipes/Oryzias_latipes.MEDAKA1.49.gtf.gz',
'otoGar1' => 'otolemur_garnettii/Otolemur_garnettii.BUSHBABY1.49.gtf.gz',
'panTro2' => 'pan_troglodytes/Pan_troglodytes.CHIMP2.1.49.gtf.gz',
'ponAbe2' => 'pongo_pygmaeus/Pongo_pygmaeus.PPYG2.49.gtf.gz',
'rn4' => 'rattus_norvegicus/Rattus_norvegicus.RGSC3.4.49.gtf.gz',
'sacCer1' => 'saccharomyces_cerevisiae/Saccharomyces_cerevisiae.SGD1.01.49.gtf.gz',
'sorAra0' => 'sorex_araneus/Sorex_araneus.COMMON_SHREW1.49.gtf',
'speTri0' => 'spermophilus_tridecemlineatus/Spermophilus_tridecemlineatus.SQUIRREL.49.gtf.gz',
'fr2' => 'takifugu_rubripes/Takifugu_rubripes.FUGU4.49.gtf.gz',
'tetNig1' => 'tetraodon_nigroviridis/Tetraodon_nigroviridis.TETRAODON7.49.gtf.gz',
'tupBel1' => 'tupaia_belangeri/Tupaia_belangeri.TREESHREW.49.gtf.gz',
'xenTro2' => 'xenopus_tropicalis/Xenopus_tropicalis.JGI4.1.49.gtf.gz',
);

# key is UCSC db name, result is FTP file name under the fasta directory
my %ensGeneFtpPeptideFileNames_49 = (
'aedAeg0' => 'aedes_aegypti/pep/Aedes_aegypti.AaegL1.49.pep.all.fa.gz',
'anoGam2' => 'anopheles_gambiae/pep/Anopheles_gambiae.AgamP3.49.pep.all.fa.gz',
'bosTau3' => 'bos_taurus/pep/Bos_taurus.Btau_3.1.49.pep.all.fa.gz',
'ce5' => 'caenorhabditis_elegans/pep/Caenorhabditis_elegans.WS180.49.pep.all.fa.gz',
'canFam2' => 'canis_familiaris/pep/Canis_familiaris.BROADD2.49.pep.all.fa.gz',
'cavPor2' => 'cavia_porcellus/pep/Cavia_porcellus.GUINEAPIG.49.pep.all.fa.gz',
'ci2' => 'ciona_intestinalis/pep/Ciona_intestinalis.JGI2.49.pep.all.fa.gz',
'cioSav2' => 'ciona_savignyi/pep/Ciona_savignyi.CSAV2.0.49.pep.all.fa.gz',
'danRer5' => 'danio_rerio/pep/Danio_rerio.ZFISH7.49.pep.all.fa.gz',
'dasNov1' => 'dasypus_novemcinctus/pep/Dasypus_novemcinctus.ARMA.49.pep.all.fa.gz',
'dm5' => 'drosophila_melanogaster/pep/Drosophila_melanogaster.BDGP5.4.49.pep.all.fa.gz',
'echTel1' => 'echinops_telfairi/pep/Echinops_telfairi.TENREC.49.pep.all.fa.gz',
'equCab2' => 'equus_caballus/pep/Equus_caballus.EquCab2.49.pep.all.fa.gz',
'eriEur1' => 'erinaceus_europaeus/pep/Erinaceus_europaeus.HEDGEHOG.49.pep.all.fa.gz',
'felCat3' => 'felis_catus/pep/Felis_catus.CAT.49.pep.all.fa.gz',
'galGal3' => 'gallus_gallus/pep/Gallus_gallus.WASHUC2.49.pep.all.fa.gz',
'gasAcu1' => 'gasterosteus_aculeatus/pep/Gasterosteus_aculeatus.BROADS1.49.pep.all.fa.gz',
'hg18' => 'homo_sapiens/pep/Homo_sapiens.NCBI36.49.pep.all.fa.gz',
'loxAfr1' => 'loxodonta_africana/pep/Loxodonta_africana.BROADE1.49.pep.all.fa.gz',
'rheMac2' => 'macaca_mulatta/pep/Macaca_mulatta.MMUL_1.49.pep.all.fa.gz',
'micMur0' => 'microcebus_murinus/pep/Microcebus_murinus.micMur1.49.pep.all.fa.gz',
'monDom5' => 'monodelphis_domestica/pep/Monodelphis_domestica.BROADO5.49.pep.all.fa.gz',
'mm9' => 'mus_musculus/pep/Mus_musculus.NCBIM37.49.pep.all.fa.gz',
'myoLuc0' => 'myotis_lucifugus/pep/Myotis_lucifugus.MICROBAT1.49.pep.all.fa.gz',
'ochPri0' => 'ochotona_princeps/pep/Ochotona_princeps.pika.49.pep.all.fa.gz',
'ornAna1' => 'ornithorhynchus_anatinus/pep/Ornithorhynchus_anatinus.OANA5.49.pep.all.fa.gz',
'oryCun1' => 'oryctolagus_cuniculus/pep/Oryctolagus_cuniculus.RABBIT.49.pep.all.fa.gz',
'oryLat1' => 'oryzias_latipes/pep/Oryzias_latipes.MEDAKA1.49.pep.all.fa.gz',
'otoGar1' => 'otolemur_garnettii/pep/Otolemur_garnettii.BUSHBABY1.49.pep.all.fa.gz',
'panTro2' => 'pan_troglodytes/pep/Pan_troglodytes.CHIMP2.1.49.pep.all.fa.gz',
'ponAbe2' => 'pongo_pygmaeus/pep/Pongo_pygmaeus.PPYG2.49.pep.all.fa.gz',
'rn4' => 'rattus_norvegicus/pep/Rattus_norvegicus.RGSC3.4.49.pep.all.fa.gz',
'sacCer1' => 'saccharomyces_cerevisiae/pep/Saccharomyces_cerevisiae.SGD1.01.49.pep.all.fa.gz',
'sorAra0' => 'sorex_araneus/pep/Sorex_araneus.COMMON_SHREW1.49.pep.all.fa.gz',
'speTri0' => 'spermophilus_tridecemlineatus/pep/Spermophilus_tridecemlineatus.SQUIRREL.49.pep.all.fa.gz',
'fr2' => 'takifugu_rubripes/pep/Takifugu_rubripes.FUGU4.49.pep.all.fa.gz',
'tetNig1' => 'tetraodon_nigroviridis/pep/Tetraodon_nigroviridis.TETRAODON7.49.pep.all.fa.gz',
'tupBel1' => 'tupaia_belangeri/pep/Tupaia_belangeri.TREESHREW.49.pep.all.fa.gz',
'xenTro2' => 'xenopus_tropicalis/pep/Xenopus_tropicalis.JGI4.1.49.pep.all.fa.gz',
);

#	directory name under release-49/mysql/ to find 'seq_region' and
#	'assembly' table copies for GeneScaffold coordinate conversions
my %ensGeneFtpMySqlFileNames_49 = (
'aedAeg0' => 'aedes_aegypti_core_49_1b',
'anoGam2' => 'anopheles_gambiae_core_49_3j',
'bosTau3' => 'bos_taurus_core_49_3f',
'ce5' => 'caenorhabditis_elegans_core_49_180a',
'canFam2' => 'canis_familiaris_core_49_2g',
'cavPor2' => 'cavia_porcellus_core_49_1c',
'ci2' => 'ciona_intestinalis_core_49_2i',
'cioSav2' => 'ciona_savignyi_core_49_2f',
'danRer5' => 'danio_rerio_core_49_7c',
'dasNov1' => 'dasypus_novemcinctus_core_49_1f',
'dm5' => 'drosophila_melanogaster_core_49_54',
'echTel1' => 'echinops_telfairi_core_49_1e',
'equCab2' => 'equus_caballus_core_49_2',
'eriEur1' => 'erinaceus_europaeus_core_49_1c',
'felCat3' => 'felis_catus_core_49_1c',
'galGal3' => 'gallus_gallus_core_49_2g',
'gasAcu1' => 'gasterosteus_aculeatus_core_49_1f',
'hg18' => 'homo_sapiens_core_49_36k',
'loxAfr1' => 'loxodonta_africana_core_49_1d',
'rheMac2' => 'macaca_mulatta_core_49_10h',
'micMur0' => 'microcebus_murinus_core_49_1',
'monDom5' => 'monodelphis_domestica_core_49_5d',
'mm9' => 'mus_musculus_core_49_37b',
'myoLuc0' => 'myotis_lucifugus_core_49_1e',
'ochPri0' => 'ochotona_princeps_core_49_1',
'ornAna1' => 'ornithorhynchus_anatinus_core_49_1f',
'oryCun1' => 'oryctolagus_cuniculus_core_49_1f',
'oryLat1' => 'oryzias_latipes_core_49_1e',
'otoGar1' => 'otolemur_garnettii_core_49_1c',
'panTro2' => 'pan_troglodytes_core_49_21h',
'ponAbe2' => 'pongo_pygmaeus_core_49_1',
'rn4' => 'rattus_norvegicus_core_49_34s',
'sacCer1' => 'saccharomyces_cerevisiae_core_49_1h',
'sorAra0' => 'sorex_araneus_core_49_1b',
'speTri0' => 'spermophilus_tridecemlineatus_core_49_1e',
'fr2' => 'takifugu_rubripes_core_49_4i',
'tetNig1' => 'tetraodon_nigroviridis_core_49_1k',
'tupBel1' => 'tupaia_belangeri_core_49_1d',
'xenTro2' => 'xenopus_tropicalis_core_49_41i',
);

# key is UCSC db name, result is FTP file name under the gtf directory
my %ensGeneGtfFileNames_50 = (
'aedAeg0' => 'aedes_aegypti/Aedes_aegypti.AaegL1.50.gtf.gz',
'anoGam2' => 'anopheles_gambiae/Anopheles_gambiae.AgamP3.50.gtf.gz',
'bosTau4' => 'bos_taurus/Bos_taurus.Btau_4.0.50.gtf.gz',
'ce6' => 'caenorhabditis_elegans/Caenorhabditis_elegans.WS190.50.gtf.gz',
'canFam2' => 'canis_familiaris/Canis_familiaris.BROADD2.50.gtf.gz',
'cavPor2' => 'cavia_porcellus/Cavia_porcellus.GUINEAPIG.50.gtf.gz',
'ci2' => 'ciona_intestinalis/Ciona_intestinalis.JGI2.50.gtf.gz',
'cioSav2' => 'ciona_savignyi/Ciona_savignyi.CSAV2.0.50.gtf.gz',
'danRer5' => 'danio_rerio/Danio_rerio.ZFISH7.50.gtf.gz',
'dasNov1' => 'dasypus_novemcinctus/Dasypus_novemcinctus.ARMA.50.gtf.gz',
'dm5' => 'drosophila_melanogaster/Drosophila_melanogaster.BDGP5.4.50.gtf.gz',
'echTel1' => 'echinops_telfairi/Echinops_telfairi.TENREC.50.gtf.gz',
'equCab2' => 'equus_caballus/Equus_caballus.EquCab2.50.gtf.gz',
'eriEur1' => 'erinaceus_europaeus/Erinaceus_europaeus.HEDGEHOG.50.gtf.gz',
'felCat3' => 'felis_catus/Felis_catus.CAT.50.gtf.gz',
'galGal3' => 'gallus_gallus/Gallus_gallus.WASHUC2.50.gtf.gz',
'gasAcu1' => 'gasterosteus_aculeatus/Gasterosteus_aculeatus.BROADS1.50.gtf.gz',
'hg18' => 'homo_sapiens/Homo_sapiens.NCBI36.50.gtf.gz',
'loxAfr1' => 'loxodonta_africana/Loxodonta_africana.BROADE1.50.gtf.gz',
'rheMac2' => 'macaca_mulatta/Macaca_mulatta.MMUL_1.50.gtf.gz',
'micMur1' => 'microcebus_murinus/Microcebus_murinus.micMur1.50.gtf.gz',
'monDom5' => 'monodelphis_domestica/Monodelphis_domestica.BROADO5.50.gtf.gz',
'mm9' => 'mus_musculus/Mus_musculus.NCBIM37.50.gtf.gz',
'myoLuc1' => 'myotis_lucifugus/Myotis_lucifugus.MICROBAT1.50.gtf.gz',
'ochPri2' => 'ochotona_princeps/Ochotona_princeps.pika.50.gtf.gz',
'ornAna1' => 'ornithorhynchus_anatinus/Ornithorhynchus_anatinus.OANA5.50.gtf.gz',
'oryCun1' => 'oryctolagus_cuniculus/Oryctolagus_cuniculus.RABBIT.50.gtf.gz',
'oryLat2' => 'oryzias_latipes/Oryzias_latipes.MEDAKA1.50.gtf.gz',
'otoGar1' => 'otolemur_garnettii/Otolemur_garnettii.BUSHBABY1.50.gtf.gz',
'panTro2' => 'pan_troglodytes/Pan_troglodytes.CHIMP2.1.50.gtf.gz',
'ponAbe2' => 'pongo_pygmaeus/Pongo_pygmaeus.PPYG2.50.gtf.gz',
'rn4' => 'rattus_norvegicus/Rattus_norvegicus.RGSC3.4.50.gtf.gz',
'sacCer1' => 'saccharomyces_cerevisiae/Saccharomyces_cerevisiae.SGD1.01.50.gtf.gz',
'sorAra0' => 'sorex_araneus/Sorex_araneus.COMMON_SHREW1.50.gtf.gz',
'speTri1' => 'spermophilus_tridecemlineatus/Spermophilus_tridecemlineatus.SQUIRREL.50.gtf.gz',
'fr2' => 'takifugu_rubripes/Takifugu_rubripes.FUGU4.50.gtf.gz',
'tetNig2' => 'tetraodon_nigroviridis/Tetraodon_nigroviridis.TETRAODON8.50.gtf.gz',
'tupBel1' => 'tupaia_belangeri/Tupaia_belangeri.TREESHREW.50.gtf.gz',
'xenTro2' => 'xenopus_tropicalis/Xenopus_tropicalis.JGI4.1.50.gtf.gz',
);

# key is UCSC db name, result is FTP file name under the fasta directory
my %ensGeneFtpPeptideFileNames_50 = (
'aedAeg0' => 'aedes_aegypti/pep/Aedes_aegypti.AaegL1.50.pep.all.fa.gz',
'anoGam2' => 'anopheles_gambiae/pep/Anopheles_gambiae.AgamP3.50.pep.all.fa.gz',
'bosTau4' => 'bos_taurus/pep/Bos_taurus.Btau_4.0.50.pep.all.fa.gz',
'ce6' => 'caenorhabditis_elegans/pep/Caenorhabditis_elegans.WS190.50.pep.all.fa.gz',
'canFam2' => 'canis_familiaris/pep/Canis_familiaris.BROADD2.50.pep.all.fa.gz',
'cavPor2' => 'cavia_porcellus/pep/Cavia_porcellus.GUINEAPIG.50.pep.all.fa.gz',
'ci2' => 'ciona_intestinalis/pep/Ciona_intestinalis.JGI2.50.pep.all.fa.gz',
'cioSav2' => 'ciona_savignyi/pep/Ciona_savignyi.CSAV2.0.50.pep.all.fa.gz',
'danRer5' => 'danio_rerio/pep/Danio_rerio.ZFISH7.50.pep.all.fa.gz',
'dasNov1' => 'dasypus_novemcinctus/pep/Dasypus_novemcinctus.ARMA.50.pep.all.fa.gz',
'dm5' => 'drosophila_melanogaster/pep/Drosophila_melanogaster.BDGP5.4.50.pep.all.fa.gz',
'echTel1' => 'echinops_telfairi/pep/Echinops_telfairi.TENREC.50.pep.all.fa.gz',
'equCab2' => 'equus_caballus/pep/Equus_caballus.EquCab2.50.pep.all.fa.gz',
'eriEur1' => 'erinaceus_europaeus/pep/Erinaceus_europaeus.HEDGEHOG.50.pep.all.fa.gz',
'felCat3' => 'felis_catus/pep/Felis_catus.CAT.50.pep.all.fa.gz',
'galGal3' => 'gallus_gallus/pep/Gallus_gallus.WASHUC2.50.pep.all.fa.gz',
'gasAcu1' => 'gasterosteus_aculeatus/pep/Gasterosteus_aculeatus.BROADS1.50.pep.all.fa.gz',
'hg18' => 'homo_sapiens/pep/Homo_sapiens.NCBI36.50.pep.all.fa.gz',
'loxAfr1' => 'loxodonta_africana/pep/Loxodonta_africana.BROADE1.50.pep.all.fa.gz',
'rheMac2' => 'macaca_mulatta/pep/Macaca_mulatta.MMUL_1.50.pep.all.fa.gz',
'micMur1' => 'microcebus_murinus/pep/Microcebus_murinus.micMur1.50.pep.all.fa.gz',
'monDom5' => 'monodelphis_domestica/pep/Monodelphis_domestica.BROADO5.50.pep.all.fa.gz',
'mm9' => 'mus_musculus/pep/Mus_musculus.NCBIM37.50.pep.all.fa.gz',
'myoLuc1' => 'myotis_lucifugus/pep/Myotis_lucifugus.MICROBAT1.50.pep.all.fa.gz',
'ochPri2' => 'ochotona_princeps/pep/Ochotona_princeps.pika.50.pep.all.fa.gz',
'ornAna1' => 'ornithorhynchus_anatinus/pep/Ornithorhynchus_anatinus.OANA5.50.pep.all.fa.gz',
'oryCun1' => 'oryctolagus_cuniculus/pep/Oryctolagus_cuniculus.RABBIT.50.pep.all.fa.gz',
'oryLat2' => 'oryzias_latipes/pep/Oryzias_latipes.MEDAKA1.50.pep.all.fa.gz',
'otoGar1' => 'otolemur_garnettii/pep/Otolemur_garnettii.BUSHBABY1.50.pep.all.fa.gz',
'panTro2' => 'pan_troglodytes/pep/Pan_troglodytes.CHIMP2.1.50.pep.all.fa.gz',
'ponAbe2' => 'pongo_pygmaeus/pep/Pongo_pygmaeus.PPYG2.50.pep.all.fa.gz',
'rn4' => 'rattus_norvegicus/pep/Rattus_norvegicus.RGSC3.4.50.pep.all.fa.gz',
'sacCer1' => 'saccharomyces_cerevisiae/pep/Saccharomyces_cerevisiae.SGD1.01.50.pep.all.fa.gz',
'sorAra0' => 'sorex_araneus/pep/Sorex_araneus.COMMON_SHREW1.50.pep.all.fa.gz',
'speTri1' => 'spermophilus_tridecemlineatus/pep/Spermophilus_tridecemlineatus.SQUIRREL.50.pep.all.fa.gz',
'fr2' => 'takifugu_rubripes/pep/Takifugu_rubripes.FUGU4.50.pep.all.fa.gz',
'tetNig2' => 'tetraodon_nigroviridis/pep/Tetraodon_nigroviridis.TETRAODON8.50.pep.all.fa.gz',
'tupBel1' => 'tupaia_belangeri/pep/Tupaia_belangeri.TREESHREW.50.pep.all.fa.gz',
'xenTro2' => 'xenopus_tropicalis/pep/Xenopus_tropicalis.JGI4.1.50.pep.all.fa.gz',
);

#	directory name under release-50/mysql/ to find 'seq_region' and
#	'assembly' table copies for GeneScaffold coordinate conversions
my %ensGeneFtpMySqlFileNames_50 = (
'aedAeg0' => 'aedes_aegypti_core_50_1c',
'anoGam2' => 'anopheles_gambiae_core_50_3k',
'bosTau4' => 'bos_taurus_core_50_4',
'ce6' => 'caenorhabditis_elegans_core_50_190',
'canFam2' => 'canis_familiaris_core_50_2h',
'cavPor2' => 'cavia_porcellus_core_50_1d',
'ci2' => 'ciona_intestinalis_core_50_2j',
'cioSav2' => 'ciona_savignyi_core_50_2g',
'danRer5' => 'danio_rerio_core_50_7d',
'dasNov1' => 'dasypus_novemcinctus_core_50_1g',
'dm5' => 'drosophila_melanogaster_core_50_54a',
'echTel1' => 'echinops_telfairi_core_50_1f',
'equCab2' => 'equus_caballus_core_50_2a',
'eriEur1' => 'erinaceus_europaeus_core_50_1d',
'felCat3' => 'felis_catus_core_50_1d',
'galGal3' => 'gallus_gallus_core_50_2h',
'gasAcu1' => 'gasterosteus_aculeatus_core_50_1g',
'hg18' => 'homo_sapiens_core_50_36l',
'loxAfr1' => 'loxodonta_africana_core_50_1e',
'rheMac2' => 'macaca_mulatta_core_50_10i',
'micMur1' => 'microcebus_murinus_core_50_1a',
'monDom5' => 'monodelphis_domestica_core_50_5e',
'mm9' => 'mus_musculus_core_50_37c',
'myoLuc1' => 'myotis_lucifugus_core_50_1f',
'ochPri2' => 'ochotona_princeps_core_50_1a',
'ornAna1' => 'ornithorhynchus_anatinus_core_50_1g',
'oryCun1' => 'oryctolagus_cuniculus_core_50_1g',
'oryLat2' => 'oryzias_latipes_core_50_1f',
'otoGar1' => 'otolemur_garnettii_core_50_1d',
'panTro2' => 'pan_troglodytes_core_50_21i',
'ponAbe2' => 'pongo_pygmaeus_core_50_1a',
'rn4' => 'rattus_norvegicus_core_50_34t',
'sacCer1' => 'saccharomyces_cerevisiae_core_50_1i',
'sorAra0' => 'sorex_araneus_core_50_1d',
'speTri1' => 'spermophilus_tridecemlineatus_core_50_1f',
'fr2' => 'takifugu_rubripes_core_50_4j',
'tetNig2' => 'tetraodon_nigroviridis_core_50_8',
'tupBel1' => 'tupaia_belangeri_core_50_1e',
'xenTro2' => 'xenopus_tropicalis_core_50_41j',
);

# key is UCSC db name, result is FTP file name under the gtf directory
my %ensGeneGtfFileNames_51 = (
'aedAeg0' => 'aedes_aegypti/Aedes_aegypti.AaegL1.51.gtf.gz',
'anoGam2' => 'anopheles_gambiae/Anopheles_gambiae.AgamP3.51.gtf.gz',
'bosTau4' => 'bos_taurus/Bos_taurus.Btau_4.0.51.gtf.gz',
'ce6' => 'caenorhabditis_elegans/Caenorhabditis_elegans.WS190.51.gtf.gz',
'canFam2' => 'canis_familiaris/Canis_familiaris.BROADD2.51.gtf.gz',
'cavPor3' => 'cavia_porcellus/Cavia_porcellus.cavPor3.51.gtf.gz',
'ci2' => 'ciona_intestinalis/Ciona_intestinalis.JGI2.51.gtf.gz',
'cioSav2' => 'ciona_savignyi/Ciona_savignyi.CSAV2.0.51.gtf.gz',
'danRer5' => 'danio_rerio/Danio_rerio.ZFISH7.51.gtf.gz',
'dasNov1' => 'dasypus_novemcinctus/Dasypus_novemcinctus.ARMA.51.gtf.gz',
'dipOrd1' => 'dipodomys_ordii/Dipodomys_ordii.dipOrd1.51.gtf.gz',
'dm5' => 'drosophila_melanogaster/Drosophila_melanogaster.BDGP5.4.51.gtf.gz',
'echTel1' => 'echinops_telfairi/Echinops_telfairi.TENREC.51.gtf.gz',
'equCab2' => 'equus_caballus/Equus_caballus.EquCab2.51.gtf.gz',
'eriEur1' => 'erinaceus_europaeus/Erinaceus_europaeus.HEDGEHOG.51.gtf.gz',
'felCat3' => 'felis_catus/Felis_catus.CAT.51.gtf.gz',
'galGal3' => 'gallus_gallus/Gallus_gallus.WASHUC2.51.gtf.gz',
'gasAcu1' => 'gasterosteus_aculeatus/Gasterosteus_aculeatus.BROADS1.51.gtf.gz',
'hg18' => 'homo_sapiens/Homo_sapiens.NCBI36.51.gtf.gz',
'loxAfr1' => 'loxodonta_africana/Loxodonta_africana.BROADE1.51.gtf.gz',
'rheMac2' => 'macaca_mulatta/Macaca_mulatta.MMUL_1.51.gtf.gz',
'micMur1' => 'microcebus_murinus/Microcebus_murinus.micMur1.51.gtf.gz',
'monDom5' => 'monodelphis_domestica/Monodelphis_domestica.BROADO5.51.gtf.gz',
'mm9' => 'mus_musculus/Mus_musculus.NCBIM37.51.gtf.gz',
'myoLuc1' => 'myotis_lucifugus/Myotis_lucifugus.MICROBAT1.51.gtf.gz',
'ochPri2' => 'ochotona_princeps/Ochotona_princeps.pika.51.gtf.gz',
'ornAna1' => 'ornithorhynchus_anatinus/Ornithorhynchus_anatinus.OANA5.51.gtf.gz',
'oryCun1' => 'oryctolagus_cuniculus/Oryctolagus_cuniculus.RABBIT.51.gtf.gz',
'oryLat2' => 'oryzias_latipes/Oryzias_latipes.MEDAKA1.51.gtf.gz',
'otoGar1' => 'otolemur_garnettii/Otolemur_garnettii.BUSHBABY1.51.gtf.gz',
'panTro2' => 'pan_troglodytes/Pan_troglodytes.CHIMP2.1.51.gtf.gz',
'ponAbe2' => 'pongo_pygmaeus/Pongo_pygmaeus.PPYG2.51.gtf.gz',
'proCap1' => 'procavia_capensis/Procavia_capensis.proCap1.51.gtf.gz',
'pteVam1' => 'pteropus_vampyrus/Pteropus_vampyrus.pteVam1.51.gtf.gz',
'rn4' => 'rattus_norvegicus/Rattus_norvegicus.RGSC3.4.51.gtf.gz',
'sacCer1' => 'saccharomyces_cerevisiae/Saccharomyces_cerevisiae.SGD1.01.51.gtf.gz',
'sorAra1' => 'sorex_araneus/Sorex_araneus.COMMON_SHREW1.51.gtf.gz',
'speTri1' => 'spermophilus_tridecemlineatus/Spermophilus_tridecemlineatus.SQUIRREL.51.gtf.gz',
'fr2' => 'takifugu_rubripes/Takifugu_rubripes.FUGU4.51.gtf.gz',
'tarSyr1' => 'tarsius_syrichta/Tarsius_syrichta.tarSyr1.51.gtf.gz',
'tetNig2' => 'tetraodon_nigroviridis/Tetraodon_nigroviridis.TETRAODON8.51.gtf.gz',
'tupBel1' => 'tupaia_belangeri/Tupaia_belangeri.TREESHREW.51.gtf.gz',
'turTru1' => 'tursiops_truncatus/Tursiops_truncatus.turTru1.51.gtf.gz',
'vicPac1' => 'vicugna_pacos/Vicugna_pacos.vicPac1.51.gtf.gz',
'xenTro2' => 'xenopus_tropicalis/Xenopus_tropicalis.JGI4.1.51.gtf.gz',
);

# key is UCSC db name, result is FTP file name under the fasta directory
my %ensGeneFtpPeptideFileNames_51 = (
'aedAeg0' => 'aedes_aegypti/pep/Aedes_aegypti.AaegL1.51.pep.all.fa.gz',
'anoGam2' => 'anopheles_gambiae/pep/Anopheles_gambiae.AgamP3.51.pep.all.fa.gz',
'bosTau4' => 'bos_taurus/pep/Bos_taurus.Btau_4.0.51.pep.all.fa.gz',
'ce6' => 'caenorhabditis_elegans/pep/Caenorhabditis_elegans.WS190.51.pep.all.fa.gz',
'canFam2' => 'canis_familiaris/pep/Canis_familiaris.BROADD2.51.pep.all.fa.gz',
'cavPor3' => 'cavia_porcellus/pep/Cavia_porcellus.cavPor3.51.pep.all.fa.gz',
'ci2' => 'ciona_intestinalis/pep/Ciona_intestinalis.JGI2.51.pep.all.fa.gz',
'cioSav2' => 'ciona_savignyi/pep/Ciona_savignyi.CSAV2.0.51.pep.all.fa.gz',
'danRer5' => 'danio_rerio/pep/Danio_rerio.ZFISH7.51.pep.all.fa.gz',
'dasNov1' => 'dasypus_novemcinctus/pep/Dasypus_novemcinctus.ARMA.51.pep.all.fa.gz',
'dipOrd1' => 'dipodomys_ordii/pep/Dipodomys_ordii.dipOrd1.51.pep.all.fa.gz',
'dm5' => 'drosophila_melanogaster/pep/Drosophila_melanogaster.BDGP5.4.51.pep.all.fa.gz',
'echTel1' => 'echinops_telfairi/pep/Echinops_telfairi.TENREC.51.pep.all.fa.gz',
'equCab2' => 'equus_caballus/pep/Equus_caballus.EquCab2.51.pep.all.fa.gz',
'eriEur1' => 'erinaceus_europaeus/pep/Erinaceus_europaeus.HEDGEHOG.51.pep.all.fa.gz',
'felCat3' => 'felis_catus/pep/Felis_catus.CAT.51.pep.all.fa.gz',
'galGal3' => 'gallus_gallus/pep/Gallus_gallus.WASHUC2.51.pep.all.fa.gz',
'gasAcu1' => 'gasterosteus_aculeatus/pep/Gasterosteus_aculeatus.BROADS1.51.pep.all.fa.gz',
'hg18' => 'homo_sapiens/pep/Homo_sapiens.NCBI36.51.pep.all.fa.gz',
'loxAfr1' => 'loxodonta_africana/pep/Loxodonta_africana.BROADE1.51.pep.all.fa.gz',
'rheMac2' => 'macaca_mulatta/pep/Macaca_mulatta.MMUL_1.51.pep.all.fa.gz',
'micMur1' => 'microcebus_murinus/pep/Microcebus_murinus.micMur1.51.pep.all.fa.gz',
'monDom5' => 'monodelphis_domestica/pep/Monodelphis_domestica.BROADO5.51.pep.all.fa.gz',
'mm9' => 'mus_musculus/pep/Mus_musculus.NCBIM37.51.pep.all.fa.gz',
'myoLuc1' => 'myotis_lucifugus/pep/Myotis_lucifugus.MICROBAT1.51.pep.all.fa.gz',
'ochPri2' => 'ochotona_princeps/pep/Ochotona_princeps.pika.51.pep.all.fa.gz',
'ornAna1' => 'ornithorhynchus_anatinus/pep/Ornithorhynchus_anatinus.OANA5.51.pep.all.fa.gz',
'oryCun1' => 'oryctolagus_cuniculus/pep/Oryctolagus_cuniculus.RABBIT.51.pep.all.fa.gz',
'oryLat2' => 'oryzias_latipes/pep/Oryzias_latipes.MEDAKA1.51.pep.all.fa.gz',
'otoGar1' => 'otolemur_garnettii/pep/Otolemur_garnettii.BUSHBABY1.51.pep.all.fa.gz',
'panTro2' => 'pan_troglodytes/pep/Pan_troglodytes.CHIMP2.1.51.pep.all.fa.gz',
'ponAbe2' => 'pongo_pygmaeus/pep/Pongo_pygmaeus.PPYG2.51.pep.all.fa.gz',
'proCap1' => 'procavia_capensis/pep/Procavia_capensis.proCap1.51.pep.all.fa.gz',
'pteVam1' => 'pteropus_vampyrus/pep/Pteropus_vampyrus.pteVam1.51.pep.all.fa.gz',
'rn4' => 'rattus_norvegicus/pep/Rattus_norvegicus.RGSC3.4.51.pep.all.fa.gz',
'sacCer1' => 'saccharomyces_cerevisiae/pep/Saccharomyces_cerevisiae.SGD1.01.51.pep.all.fa.gz',
'sorAra1' => 'sorex_araneus/pep/Sorex_araneus.COMMON_SHREW1.51.pep.all.fa.gz',
'speTri1' => 'spermophilus_tridecemlineatus/pep/Spermophilus_tridecemlineatus.SQUIRREL.51.pep.all.fa.gz',
'fr2' => 'takifugu_rubripes/pep/Takifugu_rubripes.FUGU4.51.pep.all.fa.gz',
'tarSyr1' => 'tarsius_syrichta/pep/Tarsius_syrichta.tarSyr1.51.pep.all.fa.gz',
'tetNig2' => 'tetraodon_nigroviridis/pep/Tetraodon_nigroviridis.TETRAODON8.51.pep.all.fa.gz',
'tupBel1' => 'tupaia_belangeri/pep/Tupaia_belangeri.TREESHREW.51.pep.all.fa.gz',
'turTru1' => 'tursiops_truncatus/pep/Tursiops_truncatus.turTru1.51.pep.all.fa.gz',
'vicPac1' => 'vicugna_pacos/pep/Vicugna_pacos.vicPac1.51.pep.all.fa.gz',
'xenTro2' => 'xenopus_tropicalis/pep/Xenopus_tropicalis.JGI4.1.51.pep.all.fa.gz',
);

#	directory name under release-50/mysql/ to find 'seq_region' and
#	'assembly' table copies for GeneScaffold coordinate conversions
my %ensGeneFtpMySqlFileNames_51 = (
'aedAeg0' => 'aedes_aegypti_core_51_1c',
'anoGam2' => 'anopheles_gambiae_core_51_3k',
'bosTau4' => 'bos_taurus_core_51_4a',
'ce6' => 'caenorhabditis_elegans_core_51_190',
'canFam2' => 'canis_familiaris_core_51_2i',
'cavPor3' => 'cavia_porcellus_core_51_3',
'ci2' => 'ciona_intestinalis_core_51_2k',
'cioSav2' => 'ciona_savignyi_core_51_2g',
'danRer5' => 'danio_rerio_core_51_7d',
'dasNov1' => 'dasypus_novemcinctus_core_51_1g',
'dipOrd1' => 'dipodomys_ordii_core_51_1',
'dm5' => 'drosophila_melanogaster_core_51_54a',
'echTel1' => 'echinops_telfairi_core_51_1f',
'equCab2' => 'equus_caballus_core_51_2a',
'eriEur1' => 'erinaceus_europaeus_core_51_1d',
'felCat3' => 'felis_catus_core_51_1e',
'galGal3' => 'gallus_gallus_core_51_2i',
'gasAcu1' => 'gasterosteus_aculeatus_core_51_1h',
'hg18' => 'homo_sapiens_core_51_36m',
'loxAfr1' => 'loxodonta_africana_core_51_1f',
'rheMac2' => 'macaca_mulatta_core_51_10i',
'micMur1' => 'microcebus_murinus_core_51_1a',
'monDom5' => 'monodelphis_domestica_core_51_5f',
'mm9' => 'mus_musculus_core_51_37d',
'myoLuc1' => 'myotis_lucifugus_core_51_1f',
'ochPri2' => 'ochotona_princeps_core_51_1b',
'ornAna1' => 'ornithorhynchus_anatinus_core_51_1h',
'oryCun1' => 'oryctolagus_cuniculus_core_51_1g',
'oryLat2' => 'oryzias_latipes_core_51_1g',
'otoGar1' => 'otolemur_garnettii_core_51_1d',
'panTro2' => 'pan_troglodytes_core_51_21i',
'ponAbe2' => 'pongo_pygmaeus_core_51_1b',
'proCap1' => 'procavia_capensis_core_51_1',
'pteVam1' => 'pteropus_vampyrus_core_51_1',
'rn4' => 'rattus_norvegicus_core_51_34t',
'sacCer1' => 'saccharomyces_cerevisiae_core_51_1i',
'sorAra1' => 'sorex_araneus_core_51_1d',
'speTri1' => 'spermophilus_tridecemlineatus_core_51_1f',
'fr2' => 'takifugu_rubripes_core_51_4j',
'tarSyr1' => 'tarsius_syrichta_core_51_1',
'tetNig2' => 'tetraodon_nigroviridis_core_51_8',
'tupBel1' => 'tupaia_belangeri_core_51_1e',
'turTru1' => 'tursiops_truncatus_core_51_1',
'vicPac1' => 'vicugna_pacos_core_51_1',
'xenTro2' => 'xenopus_tropicalis_core_51_41j',
);

# key is UCSC db name, result is FTP file name under the gtf directory
my %ensGeneGtfFileNames_52 = (
'aedAeg0' => 'aedes_aegypti/Aedes_aegypti.AaegL1.52.gtf.gz',
'anoGam2' => 'anopheles_gambiae/Anopheles_gambiae.AgamP3.52.gtf.gz',
'bosTau4' => 'bos_taurus/Bos_taurus.Btau_4.0.52.gtf.gz',
'ce6' => 'caenorhabditis_elegans/Caenorhabditis_elegans.WS190.52.gtf.gz',
'canFam2' => 'canis_familiaris/Canis_familiaris.BROADD2.52.gtf.gz',
'cavPor3' => 'cavia_porcellus/Cavia_porcellus.cavPor3.52.gtf.gz',
'ci2' => 'ciona_intestinalis/Ciona_intestinalis.JGI2.52.gtf.gz',
'cioSav2' => 'ciona_savignyi/Ciona_savignyi.CSAV2.0.52.gtf.gz',
'danRer5' => 'danio_rerio/Danio_rerio.ZFISH7.52.gtf.gz',
'dasNov1' => 'dasypus_novemcinctus/Dasypus_novemcinctus.ARMA.52.gtf.gz',
'dipOrd1' => 'dipodomys_ordii/Dipodomys_ordii.dipOrd1.52.gtf.gz',
'dm5' => 'drosophila_melanogaster/Drosophila_melanogaster.BDGP5.4.52.gtf.gz',
'echTel1' => 'echinops_telfairi/Echinops_telfairi.TENREC.52.gtf.gz',
'equCab2' => 'equus_caballus/Equus_caballus.EquCab2.52.gtf.gz',
'eriEur1' => 'erinaceus_europaeus/Erinaceus_europaeus.HEDGEHOG.52.gtf.gz',
'felCat3' => 'felis_catus/Felis_catus.CAT.52.gtf.gz',
'galGal3' => 'gallus_gallus/Gallus_gallus.WASHUC2.52.gtf.gz',
'gasAcu1' => 'gasterosteus_aculeatus/Gasterosteus_aculeatus.BROADS1.52.gtf.gz',
'gorGor1' => 'gorilla_gorilla/Gorilla_gorilla.gorGor1.52.gtf.gz',
'hg18' => 'homo_sapiens/Homo_sapiens.NCBI36.52.gtf.gz',
'loxAfr1' => 'loxodonta_africana/Loxodonta_africana.BROADE1.52.gtf.gz',
'rheMac2' => 'macaca_mulatta/Macaca_mulatta.MMUL_1.52.gtf.gz',
'micMur1' => 'microcebus_murinus/Microcebus_murinus.micMur1.52.gtf.gz',
'monDom5' => 'monodelphis_domestica/Monodelphis_domestica.BROADO5.52.gtf.gz',
'mm9' => 'mus_musculus/Mus_musculus.NCBIM37.52.gtf.gz',
'myoLuc1' => 'myotis_lucifugus/Myotis_lucifugus.MICROBAT1.52.gtf.gz',
'ochPri2' => 'ochotona_princeps/Ochotona_princeps.pika.52.gtf.gz',
'ornAna1' => 'ornithorhynchus_anatinus/Ornithorhynchus_anatinus.OANA5.52.gtf.gz',
'oryCun1' => 'oryctolagus_cuniculus/Oryctolagus_cuniculus.RABBIT.52.gtf.gz',
'oryLat2' => 'oryzias_latipes/Oryzias_latipes.MEDAKA1.52.gtf.gz',
'otoGar1' => 'otolemur_garnettii/Otolemur_garnettii.BUSHBABY1.52.gtf.gz',
'panTro2' => 'pan_troglodytes/Pan_troglodytes.CHIMP2.1.52.gtf.gz',
'ponAbe2' => 'pongo_pygmaeus/Pongo_pygmaeus.PPYG2.52.gtf.gz',
'proCap1' => 'procavia_capensis/Procavia_capensis.proCap1.52.gtf.gz',
'pteVam1' => 'pteropus_vampyrus/Pteropus_vampyrus.pteVam1.52.gtf.gz',
'rn4' => 'rattus_norvegicus/Rattus_norvegicus.RGSC3.4.52.gtf.gz',
'sacCer2' => 'saccharomyces_cerevisiae/Saccharomyces_cerevisiae.SGD1.01.52.gtf.gz',
'sorAra1' => 'sorex_araneus/Sorex_araneus.COMMON_SHREW1.52.gtf.gz',
'speTri1' => 'spermophilus_tridecemlineatus/Spermophilus_tridecemlineatus.SQUIRREL.52.gtf.gz',
'fr2' => 'takifugu_rubripes/Takifugu_rubripes.FUGU4.52.gtf.gz',
'tarSyr1' => 'tarsius_syrichta/Tarsius_syrichta.tarSyr1.52.gtf.gz',
'tetNig2' => 'tetraodon_nigroviridis/Tetraodon_nigroviridis.TETRAODON8.52.gtf.gz',
'tupBel1' => 'tupaia_belangeri/Tupaia_belangeri.TREESHREW.52.gtf.gz',
'turTru1' => 'tursiops_truncatus/Tursiops_truncatus.turTru1.52.gtf.gz',
'vicPac1' => 'vicugna_pacos/Vicugna_pacos.vicPac1.52.gtf.gz',
'xenTro2' => 'xenopus_tropicalis/Xenopus_tropicalis.JGI4.1.52.gtf.gz',
);


# key is UCSC db name, result is FTP file name under the fasta directory
my %ensGeneFtpPeptideFileNames_52 = (
'aedAeg0' => 'aedes_aegypti/pep/Aedes_aegypti.AaegL1.52.pep.all.fa.gz',
'anoGam2' => 'anopheles_gambiae/pep/Anopheles_gambiae.AgamP3.52.pep.all.fa.gz',
'bosTau4' => 'bos_taurus/pep/Bos_taurus.Btau_4.0.52.pep.all.fa.gz',
'ce6' => 'caenorhabditis_elegans/pep/Caenorhabditis_elegans.WS190.52.pep.all.fa.gz',
'canFam2' => 'canis_familiaris/pep/Canis_familiaris.BROADD2.52.pep.all.fa.gz',
'cavPor3' => 'cavia_porcellus/pep/Cavia_porcellus.cavPor3.52.pep.all.fa.gz',
'ci2' => 'ciona_intestinalis/pep/Ciona_intestinalis.JGI2.52.pep.all.fa.gz',
'cioSav2' => 'ciona_savignyi/pep/Ciona_savignyi.CSAV2.0.52.pep.all.fa.gz',
'danRer5' => 'danio_rerio/pep/Danio_rerio.ZFISH7.52.pep.all.fa.gz',
'dasNov1' => 'dasypus_novemcinctus/pep/Dasypus_novemcinctus.ARMA.52.pep.all.fa.gz',
'dipOrd1' => 'dipodomys_ordii/pep/Dipodomys_ordii.dipOrd1.52.pep.all.fa.gz',
'dm5' => 'drosophila_melanogaster/pep/Drosophila_melanogaster.BDGP5.4.52.pep.all.fa.gz',
'echTel1' => 'echinops_telfairi/pep/Echinops_telfairi.TENREC.52.pep.all.fa.gz',
'equCab2' => 'equus_caballus/pep/Equus_caballus.EquCab2.52.pep.all.fa.gz',
'eriEur1' => 'erinaceus_europaeus/pep/Erinaceus_europaeus.HEDGEHOG.52.pep.all.fa.gz',
'felCat3' => 'felis_catus/pep/Felis_catus.CAT.52.pep.all.fa.gz',
'galGal3' => 'gallus_gallus/pep/Gallus_gallus.WASHUC2.52.pep.all.fa.gz',
'gasAcu1' => 'gasterosteus_aculeatus/pep/Gasterosteus_aculeatus.BROADS1.52.pep.all.fa.gz',
'gorGor1' => 'gorilla_gorilla/pep/Gorilla_gorilla.gorGor1.52.pep.all.fa.gz',
'hg18' => 'homo_sapiens/pep/Homo_sapiens.NCBI36.52.pep.all.fa.gz',
'loxAfr1' => 'loxodonta_africana/pep/Loxodonta_africana.BROADE1.52.pep.all.fa.gz',
'rheMac2' => 'macaca_mulatta/pep/Macaca_mulatta.MMUL_1.52.pep.all.fa.gz',
'micMur1' => 'microcebus_murinus/pep/Microcebus_murinus.micMur1.52.pep.all.fa.gz',
'monDom5' => 'monodelphis_domestica/pep/Monodelphis_domestica.BROADO5.52.pep.all.fa.gz',
'mm9' => 'mus_musculus/pep/Mus_musculus.NCBIM37.52.pep.all.fa.gz',
'myoLuc1' => 'myotis_lucifugus/pep/Myotis_lucifugus.MICROBAT1.52.pep.all.fa.gz',
'ochPri2' => 'ochotona_princeps/pep/Ochotona_princeps.pika.52.pep.all.fa.gz',
'ornAna1' => 'ornithorhynchus_anatinus/pep/Ornithorhynchus_anatinus.OANA5.52.pep.all.fa.gz',
'oryCun1' => 'oryctolagus_cuniculus/pep/Oryctolagus_cuniculus.RABBIT.52.pep.all.fa.gz',
'oryLat2' => 'oryzias_latipes/pep/Oryzias_latipes.MEDAKA1.52.pep.all.fa.gz',
'otoGar1' => 'otolemur_garnettii/pep/Otolemur_garnettii.BUSHBABY1.52.pep.all.fa.gz',
'panTro2' => 'pan_troglodytes/pep/Pan_troglodytes.CHIMP2.1.52.pep.all.fa.gz',
'ponAbe2' => 'pongo_pygmaeus/pep/Pongo_pygmaeus.PPYG2.52.pep.all.fa.gz',
'proCap1' => 'procavia_capensis/pep/Procavia_capensis.proCap1.52.pep.all.fa.gz',
'pteVam1' => 'pteropus_vampyrus/pep/Pteropus_vampyrus.pteVam1.52.pep.all.fa.gz',
'rn4' => 'rattus_norvegicus/pep/Rattus_norvegicus.RGSC3.4.52.pep.all.fa.gz',
'sacCer2' => 'saccharomyces_cerevisiae/pep/Saccharomyces_cerevisiae.SGD1.01.52.pep.all.fa.gz',
'sorAra1' => 'sorex_araneus/pep/Sorex_araneus.COMMON_SHREW1.52.pep.all.fa.gz',
'speTri1' => 'spermophilus_tridecemlineatus/pep/Spermophilus_tridecemlineatus.SQUIRREL.52.pep.all.fa.gz',
'fr2' => 'takifugu_rubripes/pep/Takifugu_rubripes.FUGU4.52.pep.all.fa.gz',
'tarSyr1' => 'tarsius_syrichta/pep/Tarsius_syrichta.tarSyr1.52.pep.all.fa.gz',
'tetNig2' => 'tetraodon_nigroviridis/pep/Tetraodon_nigroviridis.TETRAODON8.52.pep.all.fa.gz',
'tupBel1' => 'tupaia_belangeri/pep/Tupaia_belangeri.TREESHREW.52.pep.all.fa.gz',
'turTru1' => 'tursiops_truncatus/pep/Tursiops_truncatus.turTru1.52.pep.all.fa.gz',
'vicPac1' => 'vicugna_pacos/pep/Vicugna_pacos.vicPac1.52.pep.all.fa.gz',
'xenTro2' => 'xenopus_tropicalis/pep/Xenopus_tropicalis.JGI4.1.52.pep.all.fa.gz',
);

#	directory name under release-50/mysql/ to find 'seq_region' and
#	'assembly' table copies for GeneScaffold coordinate conversions
my %ensGeneFtpMySqlFileNames_52 = (
'aedAeg0' => 'aedes_aegypti_core_52_1d',
'anoGam2' => 'anopheles_gambiae_core_52_3k',
'bosTau4' => 'bos_taurus_core_52_4b',
'ce6' => 'caenorhabditis_elegans_core_52_190',
'canFam2' => 'canis_familiaris_core_52_2j',
'cavPor3' => 'cavia_porcellus_core_52_3a',
'ci2' => 'ciona_intestinalis_core_52_2l',
'cioSav2' => 'ciona_savignyi_core_52_2h',
'danRer5' => 'danio_rerio_core_52_7e',
'dasNov1' => 'dasypus_novemcinctus_core_52_1h',
'dipOrd1' => 'dipodomys_ordii_core_52_1a',
'dm5' => 'drosophila_melanogaster_core_52_54a',
'echTel1' => 'echinops_telfairi_core_52_1g',
'equCab2' => 'equus_caballus_core_52_2b',
'eriEur1' => 'erinaceus_europaeus_core_52_1e',
'felCat3' => 'felis_catus_core_52_1f',
'galGal3' => 'gallus_gallus_core_52_2j',
'gasAcu1' => 'gasterosteus_aculeatus_core_52_1i',
'gorGor1' => 'gorilla_gorilla_core_52_1',
'hg18' => 'homo_sapiens_core_52_36n',
'loxAfr1' => 'loxodonta_africana_core_52_1g',
'rheMac2' => 'macaca_mulatta_core_52_10j',
'micMur1' => 'microcebus_murinus_core_52_1b',
'monDom5' => 'monodelphis_domestica_core_52_5g',
'mm9' => 'mus_musculus_core_52_37e',
'myoLuc1' => 'myotis_lucifugus_core_52_1g',
'ochPri2' => 'ochotona_princeps_core_52_1c',
'ornAna1' => 'ornithorhynchus_anatinus_core_52_1i',
'oryCun1' => 'oryctolagus_cuniculus_core_52_1h',
'oryLat2' => 'oryzias_latipes_core_52_1h',
'otoGar1' => 'otolemur_garnettii_core_52_1e',
'panTro2' => 'pan_troglodytes_core_52_21j',
'ponAbe2' => 'pongo_pygmaeus_core_52_1c',
'proCap1' => 'procavia_capensis_core_52_1a',
'pteVam1' => 'pteropus_vampyrus_core_52_1a',
'rn4' => 'rattus_norvegicus_core_52_34u',
'sacCer2' => 'saccharomyces_cerevisiae_core_52_1i',
'sorAra1' => 'sorex_araneus_core_52_1e',
'speTri1' => 'spermophilus_tridecemlineatus_core_52_1g',
'fr2' => 'takifugu_rubripes_core_52_4k',
'tarSyr1' => 'tarsius_syrichta_core_52_1a',
'tetNig2' => 'tetraodon_nigroviridis_core_52_8b',
'tupBel1' => 'tupaia_belangeri_core_52_1f',
'turTru1' => 'tursiops_truncatus_core_52_1a',
'vicPac1' => 'vicugna_pacos_core_52_1a',
'xenTro2' => 'xenopus_tropicalis_core_52_41l',
);

# key is UCSC db name, result is FTP file name under the gtf directory
my %ensGeneGtfFileNames_55 = (
'aedAeg0' => 'aedes_aegypti/Aedes_aegypti.AaegL1.55.gtf.gz',
'anoCar1' => 'anolis_carolinensis/Anolis_carolinensis.AnoCar1.0.55.gtf.gz',
'anoGam2' => 'anopheles_gambiae/Anopheles_gambiae.AgamP3.55.gtf.gz',
'bosTau4' => 'bos_taurus/Bos_taurus.Btau_4.0.55.gtf.gz',
'ce7' => 'caenorhabditis_elegans/Caenorhabditis_elegans.WS200.55.gtf.gz',
'canFam2' => 'canis_familiaris/Canis_familiaris.BROADD2.55.gtf.gz',
'cavPor3' => 'cavia_porcellus/Cavia_porcellus.cavPor3.55.gtf.gz',
'choHof1' => 'choloepus_hoffmanni/Choloepus_hoffmanni.choHof1.55.gtf.gz',
'ci2' => 'ciona_intestinalis/Ciona_intestinalis.JGI2.55.gtf.gz',
'cioSav2' => 'ciona_savignyi/Ciona_savignyi.CSAV2.0.55.gtf.gz',
'danRer6' => 'danio_rerio/Danio_rerio.Zv8.55.gtf.gz',
'dasNov2' => 'dasypus_novemcinctus/Dasypus_novemcinctus.dasNov2.55.gtf.gz',
'dipOrd1' => 'dipodomys_ordii/Dipodomys_ordii.dipOrd1.55.gtf.gz',
'dm5' => 'drosophila_melanogaster/Drosophila_melanogaster.BDGP5.4.55.gtf.gz',
'echTel1' => 'echinops_telfairi/Echinops_telfairi.TENREC.55.gtf.gz',
'equCab2' => 'equus_caballus/Equus_caballus.EquCab2.55.gtf.gz',
'eriEur1' => 'erinaceus_europaeus/Erinaceus_europaeus.HEDGEHOG.55.gtf.gz',
'felCat3' => 'felis_catus/Felis_catus.CAT.55.gtf.gz',
'galGal3' => 'gallus_gallus/Gallus_gallus.WASHUC2.55.gtf.gz',
'gasAcu1' => 'gasterosteus_aculeatus/Gasterosteus_aculeatus.BROADS1.55.gtf.gz',
'gorGor1' => 'gorilla_gorilla/Gorilla_gorilla.gorGor1.55.gtf.gz',
'hg19' => 'homo_sapiens/Homo_sapiens.GRCh37.55.gtf.gz',
'loxAfr2' => 'loxodonta_africana/Loxodonta_africana.loxAfr2.55.gtf.gz',
'rheMac2' => 'macaca_mulatta/Macaca_mulatta.MMUL_1.55.gtf.gz',
'macEug1' => 'macropus_eugenii/Macropus_eugenii.Meug_1.0.55.gtf.gz',
'micMur1' => 'microcebus_murinus/Microcebus_murinus.micMur1.55.gtf.gz',
'monDom5' => 'monodelphis_domestica/Monodelphis_domestica.BROADO5.55.gtf.gz',
'mm9' => 'mus_musculus/Mus_musculus.NCBIM37.55.gtf.gz',
'myoLuc1' => 'myotis_lucifugus/Myotis_lucifugus.MICROBAT1.55.gtf.gz',
'ochPri2' => 'ochotona_princeps/Ochotona_princeps.pika.55.gtf.gz',
'ornAna1' => 'ornithorhynchus_anatinus/Ornithorhynchus_anatinus.OANA5.55.gtf.gz',
'oryCun1' => 'oryctolagus_cuniculus/Oryctolagus_cuniculus.RABBIT.55.gtf.gz',
'oryLat2' => 'oryzias_latipes/Oryzias_latipes.MEDAKA1.55.gtf.gz',
'otoGar1' => 'otolemur_garnettii/Otolemur_garnettii.BUSHBABY1.55.gtf.gz',
'panTro2' => 'pan_troglodytes/Pan_troglodytes.CHIMP2.1.55.gtf.gz',
'ponAbe2' => 'pongo_pygmaeus/Pongo_pygmaeus.PPYG2.55.gtf.gz',
'proCap1' => 'procavia_capensis/Procavia_capensis.proCap1.55.gtf.gz',
'pteVam1' => 'pteropus_vampyrus/Pteropus_vampyrus.pteVam1.55.gtf.gz',
'rn4' => 'rattus_norvegicus/Rattus_norvegicus.RGSC3.4.55.gtf.gz',
'sacCer2' => 'saccharomyces_cerevisiae/Saccharomyces_cerevisiae.SGD1.01.55.gtf.gz',
'sorAra1' => 'sorex_araneus/Sorex_araneus.COMMON_SHREW1.55.gtf.gz',
'speTri1' => 'spermophilus_tridecemlineatus/Spermophilus_tridecemlineatus.SQUIRREL.55.gtf.gz',
'taeGut1' => 'taeniopygia_guttata/Taeniopygia_guttata.taeGut3.2.4.55.gtf.gz',
'fr2' => 'takifugu_rubripes/Takifugu_rubripes.FUGU4.55.gtf.gz',
'tarSyr1' => 'tarsius_syrichta/Tarsius_syrichta.tarSyr1.55.gtf.gz',
'tetNig2' => 'tetraodon_nigroviridis/Tetraodon_nigroviridis.TETRAODON8.55.gtf.gz',
'tupBel1' => 'tupaia_belangeri/Tupaia_belangeri.TREESHREW.55.gtf.gz',
'turTru1' => 'tursiops_truncatus/Tursiops_truncatus.turTru1.55.gtf.gz',
'vicPac1' => 'vicugna_pacos/Vicugna_pacos.vicPac1.55.gtf.gz',
'xenTro2' => 'xenopus_tropicalis/Xenopus_tropicalis.JGI4.1.55.gtf.gz',
);

# key is UCSC db name, result is FTP file name under the fasta directory
my %ensGeneFtpPeptideFileNames_55 = (
'aedAeg0' => 'aedes_aegypti/pep/Aedes_aegypti.AaegL1.55.pep.all.fa.gz',
'anoCar1' => 'anolis_carolinensis/pep/Anolis_carolinensis.AnoCar1.0.55.pep.all.fa.gz',
'anoGam2' => 'anopheles_gambiae/pep/Anopheles_gambiae.AgamP3.55.pep.all.fa.gz',
'bosTau4' => 'bos_taurus/pep/Bos_taurus.Btau_4.0.55.pep.all.fa.gz',
'ce7' => 'caenorhabditis_elegans/pep/Caenorhabditis_elegans.WS200.55.pep.all.fa.gz',
'canFam2' => 'canis_familiaris/pep/Canis_familiaris.BROADD2.55.pep.all.fa.gz',
'cavPor3' => 'cavia_porcellus/pep/Cavia_porcellus.cavPor3.55.pep.all.fa.gz',
'choHof1' => 'choloepus_hoffmanni/pep/Choloepus_hoffmanni.choHof1.55.pep.all.fa.gz',
'ci2' => 'ciona_intestinalis/pep/Ciona_intestinalis.JGI2.55.pep.all.fa.gz',
'cioSav2' => 'ciona_savignyi/pep/Ciona_savignyi.CSAV2.0.55.pep.all.fa.gz',
'danRer6' => 'danio_rerio/pep/Danio_rerio.Zv8.55.pep.all.fa.gz',
'dasNov2' => 'dasypus_novemcinctus/pep/Dasypus_novemcinctus.dasNov2.55.pep.all.fa.gz',
'dipOrd1' => 'dipodomys_ordii/pep/Dipodomys_ordii.dipOrd1.55.pep.all.fa.gz',
'dm5' => 'drosophila_melanogaster/pep/Drosophila_melanogaster.BDGP5.4.55.pep.all.fa.gz',
'echTel1' => 'echinops_telfairi/pep/Echinops_telfairi.TENREC.55.pep.all.fa.gz',
'equCab2' => 'equus_caballus/pep/Equus_caballus.EquCab2.55.pep.all.fa.gz',
'eriEur1' => 'erinaceus_europaeus/pep/Erinaceus_europaeus.HEDGEHOG.55.pep.all.fa.gz',
'felCat3' => 'felis_catus/pep/Felis_catus.CAT.55.pep.all.fa.gz',
'galGal3' => 'gallus_gallus/pep/Gallus_gallus.WASHUC2.55.pep.all.fa.gz',
'gasAcu1' => 'gasterosteus_aculeatus/pep/Gasterosteus_aculeatus.BROADS1.55.pep.all.fa.gz',
'gorGor1' => 'gorilla_gorilla/pep/Gorilla_gorilla.gorGor1.55.pep.all.fa.gz',
'hg19' => 'homo_sapiens/pep/Homo_sapiens.GRCh37.55.pep.all.fa.gz',
'loxAfr2' => 'loxodonta_africana/pep/Loxodonta_africana.loxAfr2.55.pep.all.fa.gz',
'rheMac2' => 'macaca_mulatta/pep/Macaca_mulatta.MMUL_1.55.pep.all.fa.gz',
'macEug1' => 'macropus_eugenii/pep/Macropus_eugenii.Meug_1.0.55.pep.all.fa.gz',
'micMur1' => 'microcebus_murinus/pep/Microcebus_murinus.micMur1.55.pep.all.fa.gz',
'monDom5' => 'monodelphis_domestica/pep/Monodelphis_domestica.BROADO5.55.pep.all.fa.gz',
'mm9' => 'mus_musculus/pep/Mus_musculus.NCBIM37.55.pep.all.fa.gz',
'myoLuc1' => 'myotis_lucifugus/pep/Myotis_lucifugus.MICROBAT1.55.pep.all.fa.gz',
'ochPri2' => 'ochotona_princeps/pep/Ochotona_princeps.pika.55.pep.all.fa.gz',
'ornAna1' => 'ornithorhynchus_anatinus/pep/Ornithorhynchus_anatinus.OANA5.55.pep.all.fa.gz',
'oryCun1' => 'oryctolagus_cuniculus/pep/Oryctolagus_cuniculus.RABBIT.55.pep.all.fa.gz',
'oryLat2' => 'oryzias_latipes/pep/Oryzias_latipes.MEDAKA1.55.pep.all.fa.gz',
'otoGar1' => 'otolemur_garnettii/pep/Otolemur_garnettii.BUSHBABY1.55.pep.all.fa.gz',
'panTro2' => 'pan_troglodytes/pep/Pan_troglodytes.CHIMP2.1.55.pep.all.fa.gz',
'ponAbe2' => 'pongo_pygmaeus/pep/Pongo_pygmaeus.PPYG2.55.pep.all.fa.gz',
'proCap1' => 'procavia_capensis/pep/Procavia_capensis.proCap1.55.pep.all.fa.gz',
'pteVam1' => 'pteropus_vampyrus/pep/Pteropus_vampyrus.pteVam1.55.pep.all.fa.gz',
'rn4' => 'rattus_norvegicus/pep/Rattus_norvegicus.RGSC3.4.55.pep.all.fa.gz',
'sacCer2' => 'saccharomyces_cerevisiae/pep/Saccharomyces_cerevisiae.SGD1.01.55.pep.all.fa.gz',
'sorAra1' => 'sorex_araneus/pep/Sorex_araneus.COMMON_SHREW1.55.pep.all.fa.gz',
'speTri1' => 'spermophilus_tridecemlineatus/pep/Spermophilus_tridecemlineatus.SQUIRREL.55.pep.all.fa.gz',
'taeGut1' => 'taeniopygia_guttata/pep/Taeniopygia_guttata.taeGut3.2.4.55.pep.all.fa.gz',
'fr2' => 'takifugu_rubripes/pep/Takifugu_rubripes.FUGU4.55.pep.all.fa.gz',
'tarSyr1' => 'tarsius_syrichta/pep/Tarsius_syrichta.tarSyr1.55.pep.all.fa.gz',
'tetNig2' => 'tetraodon_nigroviridis/pep/Tetraodon_nigroviridis.TETRAODON8.55.pep.all.fa.gz',
'tupBel1' => 'tupaia_belangeri/pep/Tupaia_belangeri.TREESHREW.55.pep.all.fa.gz',
'turTru1' => 'tursiops_truncatus/pep/Tursiops_truncatus.turTru1.55.pep.all.fa.gz',
'vicPac1' => 'vicugna_pacos/pep/Vicugna_pacos.vicPac1.55.pep.all.fa.gz',
'xenTro2' => 'xenopus_tropicalis/pep/Xenopus_tropicalis.JGI4.1.55.pep.all.fa.gz',
);

#	directory name under release-50/mysql/ to find 'seq_region' and
#	'assembly' table copies for GeneScaffold coordinate conversions
my %ensGeneFtpMySqlFileNames_55 = (
'aedAeg0' => 'aedes_aegypti_core_55_1d',
'anoCar1' => 'anolis_carolinensis_core_55_1a',
'anoGam2' => 'anopheles_gambiae_core_55_3k',
'bosTau4' => 'bos_taurus_core_55_4e',
'ce7' => 'caenorhabditis_elegans_core_55_200',
'canFam2' => 'canis_familiaris_core_55_2m',
'cavPor3' => 'cavia_porcellus_core_55_3a',
'choHof1' => 'choloepus_hoffmanni_core_55_1a',
'ci2' => 'ciona_intestinalis_core_55_2m',
'cioSav2' => 'ciona_savignyi_core_55_2h',
'danRer6' => 'danio_rerio_core_55_8e',
'dasNov2' => 'dasypus_novemcinctus_core_55_2a',
'dipOrd1' => 'dipodomys_ordii_core_55_1c',
'dm5' => 'drosophila_melanogaster_core_55_54c',
'echTel1' => 'echinops_telfairi_core_55_1g',
'equCab2' => 'equus_caballus_core_55_2d',
'eriEur1' => 'erinaceus_europaeus_core_55_1e',
'felCat3' => 'felis_catus_core_55_1f',
'galGal3' => 'gallus_gallus_core_55_2m',
'gasAcu1' => 'gasterosteus_aculeatus_core_55_1j',
'gorGor1' => 'gorilla_gorilla_core_55_1',
'hg19' => 'homo_sapiens_core_55_37',
'loxAfr2' => 'loxodonta_africana_core_55_2a',
'rheMac2' => 'macaca_mulatta_core_55_10l',
'macEug1' => 'macropus_eugenii_core_55_1',
'micMur1' => 'microcebus_murinus_core_55_1b',
'monDom5' => 'monodelphis_domestica_core_55_5i',
'mm9' => 'mus_musculus_core_55_37h',
'myoLuc1' => 'myotis_lucifugus_core_55_1g',
'ochPri2' => 'ochotona_princeps_core_55_1c',
'ornAna1' => 'ornithorhynchus_anatinus_core_55_1k',
'oryCun1' => 'oryctolagus_cuniculus_core_55_1h',
'oryLat2' => 'oryzias_latipes_core_55_1i',
'otoGar1' => 'otolemur_garnettii_core_55_1e',
'panTro2' => 'pan_troglodytes_core_55_21l',
'ponAbe2' => 'pongo_pygmaeus_core_55_1c',
'proCap1' => 'procavia_capensis_core_55_1c',
'pteVam1' => 'pteropus_vampyrus_core_55_1c',
'rn4' => 'rattus_norvegicus_core_55_34w',
'sacCer2' => 'saccharomyces_cerevisiae_core_55_1j',
'sorAra1' => 'sorex_araneus_core_55_1e',
'speTri1' => 'spermophilus_tridecemlineatus_core_55_1g',
'taeGut1' => 'taeniopygia_guttata_core_55_1b',
'fr2' => 'takifugu_rubripes_core_55_4k',
'tarSyr1' => 'tarsius_syrichta_core_55_1c',
'tetNig2' => 'tetraodon_nigroviridis_core_55_8b',
'tupBel1' => 'tupaia_belangeri_core_55_1f',
'turTru1' => 'tursiops_truncatus_core_55_1c',
'vicPac1' => 'vicugna_pacos_core_55_1c',
'xenTro2' => 'xenopus_tropicalis_core_55_41n',
);

my @versionList = qw( 55 52 51 50 49 48 47 46 35 );

my @ensGtfReference;
$ensGtfReference[55] = \%ensGeneGtfFileNames_55;
$ensGtfReference[52] = \%ensGeneGtfFileNames_52;
$ensGtfReference[51] = \%ensGeneGtfFileNames_51;
$ensGtfReference[50] = \%ensGeneGtfFileNames_50;
$ensGtfReference[49] = \%ensGeneGtfFileNames_49;
$ensGtfReference[48] = \%ensGeneGtfFileNames_48;
$ensGtfReference[47] = \%ensGeneGtfFileNames_47;
$ensGtfReference[46] = \%ensGeneGtfFileNames_46;
$ensGtfReference[35] = \%ensGeneGtfFileNames_35;
my @ensPepReference;
$ensPepReference[55] = \%ensGeneFtpPeptideFileNames_55;
$ensPepReference[52] = \%ensGeneFtpPeptideFileNames_52;
$ensPepReference[51] = \%ensGeneFtpPeptideFileNames_51;
$ensPepReference[50] = \%ensGeneFtpPeptideFileNames_50;
$ensPepReference[49] = \%ensGeneFtpPeptideFileNames_49;
$ensPepReference[48] = \%ensGeneFtpPeptideFileNames_48;
$ensPepReference[47] = \%ensGeneFtpPeptideFileNames_47;
$ensPepReference[46] = \%ensGeneFtpPeptideFileNames_46;
$ensPepReference[35] = \%ensGeneFtpPeptideFileNames_35;
my @ensMySqlReference;
$ensMySqlReference[55] = \%ensGeneFtpMySqlFileNames_55;
$ensMySqlReference[52] = \%ensGeneFtpMySqlFileNames_52;
$ensMySqlReference[51] = \%ensGeneFtpMySqlFileNames_51;
$ensMySqlReference[50] = \%ensGeneFtpMySqlFileNames_50;
$ensMySqlReference[49] = \%ensGeneFtpMySqlFileNames_49;
$ensMySqlReference[48] = \%ensGeneFtpMySqlFileNames_48;
$ensMySqlReference[47] = \%ensGeneFtpMySqlFileNames_47;
$ensMySqlReference[46] = \%ensGeneFtpMySqlFileNames_46;
$ensMySqlReference[35] = \%ensGeneFtpMySqlFileNames_35;

sub ensVersionList() {
   return @versionList;
}

sub ensGeneVersioning($$) {
#  given a UCSC db name, and an Ensembl version number, return
#	FTP gtf file name, peptide file name, MySql core directory
#	and archive version string
  my ($ucscDb, $ensVersion) = @_;
  if (defined($ensGtfReference[$ensVersion]) &&
	defined($ensPepReference[$ensVersion])) {
    my $gtfReference = $ensGtfReference[$ensVersion];
    my $pepReference = $ensPepReference[$ensVersion];
    my $mySqlReference = $ensMySqlReference[$ensVersion];
    my $gtfDir = "release-$ensVersion/gtf/";
    my $pepDir = "release-$ensVersion/fasta/";
    my $mySqlDir = "release-$ensVersion/mysql/";
    if ($ensVersion < 47) {
	$gtfDir = "release-$ensVersion/";
	$pepDir = "release-$ensVersion/";
	$mySqlDir = "release-$ensVersion/";
    }
    if (exists($gtfReference->{$ucscDb}) &&
	exists($pepReference->{$ucscDb}) &&
	exists($mySqlReference->{$ucscDb}) ) {
	my $gtfName =  $ensemblFTP . $gtfDir . $gtfReference->{$ucscDb};
	my $pepName =  $ensemblFTP . $pepDir . $pepReference->{$ucscDb};
	my $mySqlName =  $ensemblFTP . $mySqlDir . $mySqlReference->{$ucscDb};
	return ($gtfName, $pepName, $mySqlName, $verToDate[$ensVersion]);
    }
  }
  return (undef, undef, undef);
}
