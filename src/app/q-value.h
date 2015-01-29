#ifndef QVALUE_H
#define QVALUE_H

/**
 * \file q-value.h
 * AUTHOR: Barbara Frewen
 * CREATE DATE: November 24, 2008
 ****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "io/carp.h"
#include "util/crux-utils.h"
#include "objects.h"
#include "parameter.h"
#include "model/Protein.h"
#include "model/Peptide.h"
#include "model/Spectrum.h"
#include "util/parse_arguments.h" 
#include "io/SpectrumCollection.h"
#include "model/Scorer.h"
#include "model/Match.h"
#include "model/MatchCollection.h"
#include "io/OutputFiles.h"



FLOAT_T* compute_decoy_qvalues_tdc(
  FLOAT_T* target_scores,
  int      num_targets,
  FLOAT_T* decoy_scores,
  int      num_decoys,
  bool     reverse,
  FLOAT_T  pi_zero);

FLOAT_T* compute_qvalues_from_pvalues(
  FLOAT_T* pvalues, 
  int      num_pvals,
  FLOAT_T  pi_zero);

FLOAT_T estimate_pi0( FLOAT_T* target_scores,
  int      num_targets,
  FLOAT_T* decoy_scores,
  int      num_decoys,
  bool     ascending);
  
MatchCollection* run_qvalue(
  const std::string& psm_result_folder, 
  const std::string& fasta_file,
  OutputFiles& output,
  COMMAND_T command  );

#endif //QVALUE_H
