Feature: tide-index / tide-search
  tide-index should create an index for all peptides in a fasta file, for use in
    subsequent calls to tide-search
  tide-search should search a collection of spectra against a sequence database,
    returning a collection of peptide-spectrum matches (PSMs)

Scenario Outline: User runs tide-index / tide-search
  Given the path to Crux is ../../src/crux
  And I want to run a test named <test_name>
  And I pass the arguments --overwrite T --seed 7 <index_args> <fasta> <index>
  When I run tide-index as an intermediate step
  Then the return value should be 0
  And I pass the arguments --overwrite T --file-column F <search_args> <spectra> <index>
  When I run tide-search
  Then the return value should be 0
  And crux-output/<actual_output> should contain the same lines as good_results/<expected_output>

Examples:
  |test_name      |index_args                                                   |search_args                                             |fasta            |index          |spectra |actual_output         |expected_output    |
  |tide-default   |                                                             |--precursor-window 3 --precursor-window-type mass --mz-bin-width 1.0005079|small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-default.txt   |
  # Tests that vary tide-index options
  |tide-peplen    |--min-length 5 --max-length 10                               |--precursor-window 3 --precursor-window-type mass --mz-bin-width 1.0005079|small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-peplen.txt    |
  |tide-pepmass   |--min-mass 1000 --max-mass 2000                              |--precursor-window 3 --precursor-window-type mass --mz-bin-width 1.0005079|small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-pepmass.txt   |
  |tide-avgmass   |--isotopic-mass average                                      |--precursor-window 3 --precursor-window-type mass --mz-bin-width 1.0005079|small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-avgmass.txt   |
  |tide-clipn     |--clip-nterm-methionine T                                    |--precursor-window 3 --precursor-window-type mass --mz-bin-width 1.0005079|small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-clipn.txt     |
  |tide-mods1     |--mods-spec C+57.02146,2M+15.9949,1STY+79.966331             |--precursor-window 3 --precursor-window-type mass --mz-bin-width 1.0005079|small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-mods1.txt     |
  |tide-mods1limit|--mods-spec C+57.02146,2M+15.9949,1STY+79.966331 --max-mods 1|--precursor-window 3 --precursor-window-type mass --mz-bin-width 1.0005079|small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-mods1limit.txt|
  |tide-mods1min  |--mods-spec C+57.02146,2M+15.9949,1STY+79.966331 --min-mods 1|--precursor-window 3 --precursor-window-type mass --mz-bin-width 1.0005079|small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-mods1min.txt  |
  |tide-modsn     |--nterm-peptide-mods-spec 1E-18.0106,C-17.0265               |--precursor-window 3 --precursor-window-type mass --mz-bin-width 1.0005079|small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-modsn.txt     |
  |tide-modsc     |--cterm-peptide-mods-spec X+21.9819                          |--precursor-window 3 --precursor-window-type mass --mz-bin-width 1.0005079|small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-modsc.txt     |
  |tide-chymo     |--enzyme chymotrypsin                                        |--precursor-window 3 --precursor-window-type mass --mz-bin-width 1.0005079|small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-chymo.txt     |
  |tide-partial   |--digestion partial-digest                                   |--precursor-window 3 --precursor-window-type mass --mz-bin-width 1.0005079|small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-partial.txt   |
  |tide-misscleave|--missed-cleavages 2                                         |--precursor-window 3 --precursor-window-type mass --mz-bin-width 1.0005079|small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-misscleave.txt|
  |tide-reverse   |--decoy-format peptide-reverse                               |--precursor-window 3 --precursor-window-type mass --mz-bin-width 1.0005079|small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-reverse.txt   |
  |tide-multidecoy|--num-decoys-per-target 5                                    |--precursor-window 3 --precursor-window-type mass --mz-bin-width 1.0005079|small-yeast.fasta|tide_test_index|demo.ms2|tide-search.decoy.txt |tide-5decoys.txt   |

  # Tests that vary tide-search options
  |tide-masswin   |                                                             |--precursor-window 5 --precursor-window-type mass --mz-bin-width 1.0005079     |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-masswin.txt   |
  |tide-mzwin     |                                                             |--precursor-window 5 --precursor-window-type mz   --mz-bin-width 1.0005079     |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-mzwin.txt     |
  |tide-ppmwin    |                                                             |--precursor-window 5 --precursor-window-type ppm  --mz-bin-width 1.0005079     |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-ppmwin.txt    |
  |tide-computesp |                                                             |--precursor-window 3 --precursor-window-type mass --compute-sp T --mz-bin-width 1.0005079                                         |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-computesp.txt |
  |tide-specmz    |                                                             |--precursor-window 3 --precursor-window-type mass --spectrum-min-mz 800 --spectrum-max-mz 900 --mz-bin-width 1.0005079            |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-specmz.txt    |
  |tide-minpeaks  |                                                             |--precursor-window 3 --precursor-window-type mass --min-peaks 100 --mz-bin-width 1.0005079                                        |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-minpeaks.txt  |
  |tide-speccharge|                                                             |--precursor-window 3 --precursor-window-type mass --spectrum-charge 3 --mz-bin-width 1.0005079                                    |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-speccharge.txt|
  |tide-scannums  |                                                             |--precursor-window 3 --precursor-window-type mass --scan-number 30-36 --mz-bin-width 1.0005079                                    |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-scannums.txt  |
  |tide-rempeaks  |                                                             |--precursor-window 3 --precursor-window-type mass --remove-precursor-peak T --remove-precursor-tolerance 3 --mz-bin-width 1.0005079|small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-rempeaks.txt  |
  |tide-useflank  |                                                             |--precursor-window 3 --precursor-window-type mass --use-flanking-peaks T --mz-bin-width 1.0005079                                 |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-useflank.txt  |
  |tide-usenl     |                                                             |--precursor-window 3 --precursor-window-type mass --use-neutral-loss-peaks T --mz-bin-width 1.0005079                             |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-usenl.txt     |
  |tide-mzbins    |                                                             |--precursor-window 3 --precursor-window-type mass --mz-bin-width 0.02 --mz-bin-offset 0.34                |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-mzbins.txt    |
  |tide-exact-pval|                                                             |--precursor-window 3 --precursor-window-type mass --exact-p-value T --mz-bin-width 1.0005079                                      |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-exact-pval.txt|
  |tide-1thread   |                                                             |--precursor-window 3 --precursor-window-type mass --num-threads 1 --mz-bin-width 1.0005079                                        |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-default.txt   |
  |tide-7thread   |                                                             |--precursor-window 3 --precursor-window-type mass --num-threads 7 --mz-bin-width 1.0005079                                        |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-default.txt   |
  |tide-exact-pval-1thread|                                                     |--precursor-window 3 --precursor-window-type mass --exact-p-value T --num-threads 1 --mz-bin-width 1.0005079                      |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-exact-pval.txt|
  |tide-exact-pval-7thread|                                                     |--precursor-window 3 --precursor-window-type mass --exact-p-value T --num-threads 7 --mz-bin-width 1.0005079                      |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-exact-pval.txt|
  |tide-concat    |                                                             |--precursor-window 3 --precursor-window-type mass --concat T --mz-bin-width 1.0005079                                             |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.txt       |tide-concat.txt    |
  |tide-isoerr    |                                                             |--precursor-window 3 --precursor-window-type mass --isotope-error 1,2,3 --mz-bin-width 1.0005079                                  |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-isoerr.txt    |
  |tide-isoerrpval|                                                             |--precursor-window 3 --precursor-window-type mass --isotope-error 1,2,3 --exact-p-value T --mz-bin-width 1.0005079                |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-isoerrpval.txt|
  |tide-combine-pval-1thread|                                                   |--precursor-window 3 --precursor-window-type mass --score-function both --exact-p-value T --num-threads 1 --mz-bin-width 1.0005079 |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-combine-1.txt   |
  |tide-combine-pval-7thread|                                                   |--precursor-window 3 --precursor-window-type mass --score-function both --exact-p-value T --num-threads 7 --mz-bin-width 1.0005079 |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-combine-7.txt   |
  |tide-resEv-pval-1thread|                                                     |--precursor-window 3 --precursor-window-type mass --score-function residue-evidence --exact-p-value T --num-threads 1 --use-neutral-loss-peaks F --mz-bin-width 1.0005079|small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-resEv.txt   |
  |tide-resEv-pval-7thread|                                                     |--precursor-window 3 --precursor-window-type mass --score-function residue-evidence --exact-p-value T --num-threads 7 --use-neutral-loss-peaks F --mz-bin-width 1.0005079|small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-resEv.txt   |
  |tide-deiso     |                                                             |--precursor-window 3 --precursor-window-type mass --deisotope 10 --mz-bin-width 1.0005079                                         |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-deiso.txt     |
  |tide-deiso-pval|                                                             |--precursor-window 3 --precursor-window-type mass --deisotope 10 --exact-p-value t --mz-bin-width 1.0005079                       |small-yeast.fasta|tide_test_index|demo.ms2|tide-search.target.txt|tide-deiso-pval.txt|
