/**
 * \file q-ranker.c
 */
/*
 * AUTHOR: Barbara Frewen and Marina Spivak
 * CREATE DATE: November 25, 2008
 * DESCRIPTION: Copied from match_analysis.c with only the percolator
 *         functionality kept.
 *         Given as input a directory containing binary psm files and
 *         a protein database, run q-ranker and return a txt file
 *         with results.
 *
 *         Handles at most 4 files (target and decoy).  Looks for .csm
 *         files in the input directory and for corresponding
 *         -decoy[123].csm files.  Multiple target files in the given
 *         directory are concatinated together and presumed to be
 *         non-overlaping parts of the same ms2 file. 
 * 
 * $Revision: 1.1.2.3 $
 ****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "carp.h"
#include "crux-utils.h"
#include "objects.h"
#include "parameter.h"
#include "q-ranker.h"
#include "protein.h"
#include "peptide.h"
#include "spectrum.h"
#include "parse_arguments.h" 
#include "spectrum_collection.h"
#include "generate_peptides_iterator.h"
#include "scorer.h"
#include "match.h"
#include "match_collection.h"
#include "QRankerCInterface.h"



#define NUM_QRANKER_OPTIONS 7
#define NUM_QRANKER_ARGUMENTS 1
/* 
 * Private function declarations.  Details below
 */

MATCH_COLLECTION_T* run_q(
  char* psm_result_folder, 
  char* fasta_file, 
  char* feature_file); 


void print_text_files( 
  MATCH_COLLECTION_T* match_collection
  );

  
/**
 * \brief crux-analyze-matches: takes in a directory containing binary
 * psm files and a protein index and analyzes the psms.
 */
int qranker_main(int argc, char** argv){


  /* Define command line arguments */
  int num_options = NUM_QRANKER_OPTIONS;
  const char* option_list[NUM_QRANKER_OPTIONS] = {
    "version",
    "verbosity",
    "parameter-file",
    "fileroot",
    "feature-file",
    "output-dir",
    "overwrite",
  };


  int num_arguments = NUM_QRANKER_ARGUMENTS;
  const char* argument_list[NUM_QRANKER_ARGUMENTS] = {
    "protein input",
  };

  /* for debugging handling of parameters*/
  set_verbosity_level(CARP_ERROR);

  /* Set up parameters and set defaults in parameter.c */
  initialize_parameters();

  /* Define optional and required arguments in parameter.c */
  select_cmd_line_options(option_list, num_options );
  select_cmd_line_arguments(argument_list, num_arguments);

  /* Parse the command line and optional paramter file
     does sytnax, type, and bounds checking and dies on error */
  parse_cmd_line_into_params_hash(argc, argv, "crux-analyze-matches");

  /* Get arguments */
  char* psm_dir = get_string_parameter("output-dir");
  char* protein_input_name = get_string_parameter("protein input");
  char* feature_file = get_string_parameter("feature-file");
  if (feature_file != NULL) {
    prefix_fileroot_to_name(&feature_file);
  }

  /* Get options */
  SCORER_TYPE_T scorer_type = QRANKER_SCORE;
  SCORER_TYPE_T second_scorer_type = QRANKER_Q_VALUE;
  MATCH_COLLECTION_T* match_collection = NULL;

  /* Perform the analysis */
  carp(CARP_INFO, "Running q-ranker");

  char* param_file_name = get_string_parameter("qranker-param-file");
  print_parameter_file(&param_file_name);
  free(param_file_name);

  match_collection = run_q(psm_dir,
                                    protein_input_name,
                                    feature_file);
  scorer_type = QRANKER_SCORE;
  second_scorer_type = QRANKER_Q_VALUE;
    
  carp(CARP_INFO, "Outputting matches.");
  print_text_files(match_collection);

  // MEMLEAK below causes seg fault (or used to)
  // free_match_collection(match_collection);

  // clean up
  free(psm_dir);
  free(protein_input_name);
  free(feature_file);


  carp(CARP_INFO, "crux q-ranker finished.");
  exit(0);


}


/*  ****************** Subroutines ****************/


/*
 */
void print_text_files(
  MATCH_COLLECTION_T* match_collection
  ){

  // get filename and open file
  char* out_dir = get_string_parameter("output-dir");
  char* tab_filename = get_string_parameter("qranker-tab-output-file");
  prefix_fileroot_to_name(&tab_filename);
  BOOLEAN_T overwrite = get_boolean_parameter("overwrite");
  FILE* tab_file = create_file_in_path( tab_filename, out_dir, overwrite );

  // print header
  print_tab_header(tab_file);

  // print matches to tab file
  print_matches_multi_spectra(match_collection, tab_file, NULL);

  free(tab_filename);
}

/**
 * \brief Analyze matches using the q-ranker algorithm
 * 
 * Runs Marina Spivak's q-ranker algorithm on the PSMs in the psm_result_folder
 * for a search against the sequence database fasta_file. Optionally 
 * puts the q-ranker PSM feature vectors into feature_file, if it is 
 * not NULL.
 * \returns a pointer to a MATCH_COLLECTION_T object
 * \callgraph
 */
MATCH_COLLECTION_T* run_q(
  char* psm_result_folder, 
  char* fasta_file, 
  char* feature_file){ 

  unsigned int number_features = 20;
  double* features = NULL;    
  double* results_q = NULL;
  double* results_score = NULL;
  double pi0 = get_double_parameter("pi0");
  char** feature_names = generate_feature_name_array();
  MATCH_ITERATOR_T* match_iterator = NULL;
  MATCH_COLLECTION_T* match_collection = NULL;
  MATCH_COLLECTION_T* target_match_collection = NULL;
  MATCH_T* match = NULL;
  FILE* feature_fh = NULL;
  int set_idx = 0;
  
  // optional feature_file
  if(feature_file != NULL){  
    if((feature_fh = fopen(feature_file, "w")) == NULL){
      carp(CARP_FATAL, "Problem opening output file %s", feature_file);
      return NULL;
    }
  }

  carp(CARP_DETAILED_DEBUG, "Created feature file");

  
  // create MATCH_COLLECTION_ITERATOR_T object
  // which will read in the serialized output PSM results and return
  // first the match_collection of TARGET followed by 
  // the DECOY* match_collections.
  int num_decoys = 0;
  MATCH_COLLECTION_ITERATOR_T* match_collection_iterator =
    new_match_collection_iterator(psm_result_folder, fasta_file, &num_decoys);

  if( match_collection_iterator == NULL ){
    carp(CARP_FATAL, "Failed to create a match collection iterator");
    exit(1);
  }
  carp(CARP_DETAILED_DEBUG, "Created the match collection iterator");

  
  // iterate over each, TARGET, DECOY 1..3 match_collection sets
  int iterations = 0;
  while(match_collection_iterator_has_next(match_collection_iterator)){
    
    carp(CARP_DEBUG, "Match collection iteration: %i" , iterations++);

    // get the next match_collection
    match_collection = 
      match_collection_iterator_next(match_collection_iterator);
    
    // intialize q-ranker, using information from first match_collection
    if(set_idx == 0){
      // the first match_collection is the target_match_collection
      target_match_collection = match_collection;
      
      // result array that stores the algorithm scores
      results_q = (double*)mycalloc(
				    get_match_collection_match_total(match_collection), sizeof(double));
      results_score = (double*)mycalloc(
					get_match_collection_match_total(match_collection), sizeof(double));
          
      // Call that initiates q-ranker
      qcInitiate(
		 (NSet)get_match_collection_iterator_number_collections(
									match_collection_iterator), 
		 number_features, 
		 get_match_collection_match_total(match_collection), 
		 feature_names, 
		 pi0);
      
      
      // Call that sets verbosity level
      // 0 is quiet, 2 is default, 5 is more than you want
      if(verbosity < CARP_ERROR){
	qcSetVerbosity(0);
      }    
      else if(verbosity < CARP_INFO){
	qcSetVerbosity(1);
      }
      else{
	qcSetVerbosity(5);
      }
    }

    // create iterator, to register each PSM feature to q-ranker
    match_iterator = new_match_iterator(match_collection, XCORR, FALSE);
    
    while(match_iterator_has_next(match_iterator)){
      match = match_iterator_next(match_iterator);
      // Register PSM with features to q-ranker    
      features = get_match_percolator_features(match, match_collection);

      if (feature_fh != NULL){
        
        fprintf(feature_fh, "%i\t",
            get_spectrum_first_scan(get_match_spectrum(match))
            );
        if (get_match_null_peptide(match) == FALSE){
          fprintf(feature_fh, "1\t");
        } else { 
          fprintf(feature_fh, "-1\t");
        };

        unsigned int feature_idx;
        for (feature_idx = 0; feature_idx < number_features; feature_idx++){
          if (feature_idx < number_features - 1){
            fprintf(feature_fh, "%.4f\t", features[feature_idx]);
          } else {
            fprintf(feature_fh, "%.4f\n", features[feature_idx]);
          }
        }
      }
      
      //qcRegisterPSM((SetType)set_idx, NULL, features);
      qcRegisterPSM((SetType)set_idx, get_match_sequence_sqt(match), features);
      
      free(features);
    }

    // ok free & update for next set
    // MEMLEAK 
    free_match_iterator(match_iterator);

    // don't free the target_match_collection
    if(set_idx != 0){
      free_match_collection(match_collection);
    }

    ++set_idx;
  } // end iteratation over each, TARGET, DECOY 1..3 match_collection sets

  if (feature_fh != NULL){
    fclose(feature_fh);
  }
  
  /***** Q-RANKER run *********/

    carp(CARP_DETAILED_DEBUG, "got to here");
    
    // Start processing
  qcExecute(); 
  
  /* Retrieving target scores and qvalues after 
   * processing, the array should be numSpectra long and will be filled in 
   * the same order as the features were inserted */
  qcGetScores(results_score, results_q); 
       
  // fill results for QRANKER_Q_VALUE
  fill_result_to_match_collection(
      target_match_collection, results_q, QRANKER_Q_VALUE, TRUE);
  
  // fill results for QRANKER_SCORE
  fill_result_to_match_collection(
      target_match_collection, results_score, QRANKER_SCORE, FALSE);
   
  // Function that should be called after processing finished
  qcCleanUp();
  
  // TODO put free back in. took out because claimed it was double free
  // free names
  unsigned int name_idx;
  for(name_idx=0; name_idx < number_features; ++name_idx){
    free(feature_names[name_idx]);
  }
  free(feature_names);
  

  free(results_q);
  free(results_score);
  free_match_collection_iterator(match_collection_iterator);

  // TODO put free back in. took out because glibc claimed it was corrupted
  // double linked list
  // free_parameters();
  return target_match_collection;
}



/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 2
 * End:
 */
