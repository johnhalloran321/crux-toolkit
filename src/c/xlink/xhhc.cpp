#include "xhhc.h"

extern "C" {
#include "peptide.h"
}

using namespace std;

FLOAT_T LinkedPeptide::linker_mass;

// a hack, works for EDC linker only
void get_linkable_peptides(set<string>& peptides, 
	DATABASE_PROTEIN_ITERATOR_T* protein_iterator,
	PEPTIDE_CONSTRAINT_T* peptide_constraint) 
{
  PROTEIN_PEPTIDE_ITERATOR_T* peptide_iterator = NULL;
  PROTEIN_T* protein;
  PEPTIDE_T* peptide;
  string sequence = "";
  string last_sequence = "zz";
  bool missed_cleavage = false;
  // keep track of whether the next peptide contains the previous one or not
  size_t index;
  while (database_protein_iterator_has_next(protein_iterator)) {
    protein = database_protein_iterator_next(protein_iterator);
    peptide_iterator = new_protein_peptide_iterator(protein, peptide_constraint);
    // missed_cleavages must be TRUE in protein.c for this to work
    prepare_protein_peptide_iterator(peptide_iterator); 
    while (protein_peptide_iterator_has_next(peptide_iterator)) {
      //peptide = database_peptide_iterator_next(peptide_iterator);
      peptide = protein_peptide_iterator_next(peptide_iterator);
      sequence = get_peptide_sequence(peptide); 
      index = sequence.find(last_sequence);
      // if doesn't contain last peptide
      if (sequence[0] == 'R' && sequence[1] != 'P') { continue;}
      if (index == string::npos || missed_cleavage) {
        missed_cleavage = !missed_cleavage;
        if (!missed_cleavage && last_sequence[last_sequence.size()-1] != 'K') {
          //cout << "skipping " << get_peptide_sequence(peptide) << endl;
	  continue;
	} 
        //cout << "peptide " << get_peptide_sequence(peptide) << endl;
        peptides.insert(string(get_peptide_sequence(peptide)));
      } else {
	//cout << "skipping " << get_peptide_sequence(peptide) << endl;
	missed_cleavage = false;
      }
      last_sequence = string(get_peptide_sequence(peptide));
    }
  } 
}

// creates an index of all linked peptides from a fasta file
void find_all_precursor_ions(vector<LinkedPeptide>& all_ions, 
			     char* links, 
			     char* missed_link_cleavage,
		             char* database_file,
			     int charge)
{
 
  DATABASE_T* db = new_database(database_file, FALSE);
  PEPTIDE_CONSTRAINT_T* peptide_constraint = new_peptide_constraint_from_parameters();
  // add 
  set_peptide_constraint_num_mis_cleavage(peptide_constraint, 1);
  //set_verbosity_level(CARP_INFO);
  //PROTEIN_T* protein = NULL;
  DATABASE_PROTEIN_ITERATOR_T* protein_iterator = new_database_protein_iterator(db);
  //PROTEIN_PEPTIDE_ITERATOR_T* peptide_iterator = NULL;
  string bonds_string = string(links);
  set<string> peptides;
  get_linkable_peptides(peptides, protein_iterator, peptide_constraint);
  add_linked_peptides(all_ions, peptides, bonds_string, charge);

  //sort by increasing mass.

  //LinkedPeptide::sortByMass(all_ions);

 
}



// modified version of crux's estimate_weibull_parameters_from_xcorrs
BOOLEAN_T hhc_estimate_weibull_parameters_from_xcorrs(
  FLOAT_T* scores,
  int num_scores,
  FLOAT_T* eta,
  FLOAT_T* beta,
  FLOAT_T* shift,
  FLOAT_T* correlation,
  SPECTRUM_T* spectrum,
  int charge
  ){
/*
  if( match_collection == NULL || spectrum == NULL ){
    carp(CARP_ERROR, "Cannot estimate parameters from null inputs.");
    return FALSE;
  }
*/
  // check that we have the minimum number of matches
  if( num_scores < MIN_WEIBULL_MATCHES ){
    carp(CARP_DETAILED_INFO, "Too few psms (%i) to estimate "
         "p-value parameters for spectrum %i, charge %i",
         num_scores, get_spectrum_first_scan(spectrum), charge);
    // set eta, beta, and shift to something???
    return FALSE;
  }

  // randomly sample n from the list by shuffling and taking first n 
  shuffle_floats(scores, num_scores);
  int num_samples = num_scores;
  //int num_samples = PARAM_ESTIMATION_SAMPLE_COUNT;
  //if(num_samples > num_scores){ num_samples = num_scores; }

  //int num_samples = num_scores;
  // reverse sort the first num_samples of them
  qsort(scores, num_samples, sizeof(float), compare_floats_descending);

  // use only a fraction of the samples, the high-scoring tail
  // this parameter is hidden from the user
  double fraction_to_fit = get_double_parameter("fraction-top-scores-to-fit");
  assert( fraction_to_fit >= 0 && fraction_to_fit <= 1 );
  int num_tail_samples = (int)(num_samples * fraction_to_fit);
  carp(CARP_INFO, "Estimating Weibull params with %d psms (%.2f of %i)",
       num_tail_samples, fraction_to_fit, num_samples);

  // do the estimation
  fit_three_parameter_weibull(scores, 
			      num_tail_samples, 
			      num_samples,
			      MIN_XCORR_SHIFT, 
			      MAX_XCORR_SHIFT, 
			      XCORR_SHIFT,
			      0,  /*CORR_THRESHOLD*/
			      eta, 
			      beta, 
			      shift, 
			      correlation);

  carp(CARP_DETAILED_DEBUG,
      "Correlation: %.6f\nEta: %.6f\nBeta: %.6f\nShift: %.6f\n",
      correlation, *eta, *beta, *shift);

  return TRUE;
}

// helper function for get_linkable_peptides
// creates single and cross-linked peptides, considering every pair
// of peptides and every possible link site

void add_linked_peptides(vector<LinkedPeptide>& all_ions, set<string>& peptides, string links, int charge) {
  //if (peptides.size() < 2) return;
  BondMap bonds; 
  vector<LinkedPeptide> ions;
  for (size_t i = 0; i < links.length() - 2; i += 4) {
     bonds[links[i+2]].insert(links[i]);
     bonds[links[i]].insert(links[i+2]);
  }

  // iterate over both sequences, adding linked peptides with correct links
  for (set<string>::iterator pepA = peptides.begin(); pepA != peptides.end(); ++pepA) {
    char* sequenceA = (char*) pepA->c_str();
    // add unlinked precursor
    LinkedPeptide lp = LinkedPeptide(charge);
    Peptide p = Peptide(sequenceA);
    lp.add_peptide(p);
    // don't add non-cross linked peptides?
    //ions.push_back(lp);
    for (size_t i = 0; i < pepA->length(); ++i) {
      BondMap::iterator bond = bonds.find(pepA->at(i));
      // if a link aa and doesn't end in K
      if (bond != bonds.end() && i != pepA->length()-1) {
	if (i == pepA->length()-1 && pepA->at(i) == 'K') continue;
        // add dead end
	//TODO: make these options.
	//ions.push_back(LinkedPeptide(sequenceA, NULL, i, -1, charge));
        // add self loop
	/*
	for (size_t j = i+1; j < pepA->length(); ++j) {
          if (bond->second.find(pepA->at(j)) != bond->second.end()) { 
	    //skip if linked to a K at the end
	    if (j == pepA->length()-1 && pepA->at(j) == 'K') continue;
	    ions.push_back(LinkedPeptide(sequenceA, NULL, i, j, charge));
	  }
	}
	*/
        // add linked precursor
        for (set<string>::iterator pepB = pepA; pepB != peptides.end(); ++pepB) {
          char* sequenceB = (char*) pepB->c_str();
          for (size_t j = 0; j < pepB->length(); ++j) {
	    // if a link aa and doesn't end in K
	    if (bond->second.find(pepB->at(j)) != bond->second.end()) { 
	      // skip if link to K at end
	      if (j == pepB->length()-1 && pepB->at(j) == 'K') continue;
	      //if (strcmp(sequenceA, sequenceB) != 0)
	      ions.push_back(LinkedPeptide(sequenceA, sequenceB, i, j, charge));
	    }
          }
	} // get next pepB 
      }
    }
  } // get next pepA 
   all_ions.insert(all_ions.end(), ions.begin(), ions.end());
}

// append one shuffled decoy to decoys vector
void add_decoys(vector<LinkedPeptide>& decoys, LinkedPeptide& lp) {
  vector<Peptide> peptides = lp.peptides();
  LinkedPeptide decoy = LinkedPeptide(lp.charge()); 
  Peptide pepA_shuffled = shuffle(peptides[0]);
  decoy.add_peptide(pepA_shuffled);
  if (lp.size() == 2) {
    Peptide pepB_shuffled = shuffle(peptides[1]);
    decoy.add_peptide(pepB_shuffled);
  }
  //cout << "decoy " << decoy << " from " << endl << "peptd " << lp << endl;
  //cout << "decoy\ttarget" << endl;
  //cout << decoy.get_mz() << "\t" << lp.get_mz() << endl;
  //cout << decoy.mass() << "\t" << lp.mass() << endl;
  decoy.set_decoy();
  decoy.calculate_mass();
  decoys.push_back(decoy);
}

// return a shuffled peptide, preserving any links
Peptide shuffle(Peptide peptide) {
  string shuffled = string(peptide.sequence());
  //cout << shuffled << " before shuffle " << endl;
  Peptide shuffled_peptide = Peptide(peptide.sequence());
  for (size_t i = 0; i < shuffled.length(); ++i) {
    if (peptide.has_link_at(i)) shuffled_peptide.add_link(i);
  }

  int start_idx = 1;
  int end_idx = peptide.length() - 2;
  int switch_idx = 0;
  char temp_char = 0;
  while(start_idx <= end_idx){
    //cout << shuffled << endl;
    switch_idx = get_random_number_interval(start_idx, end_idx);
    //cout << "start index " << start_idx << " switch index " << switch_idx << endl;
    temp_char = shuffled[start_idx];
    shuffled[start_idx] = shuffled[switch_idx];
    shuffled[switch_idx] = temp_char;
    if (shuffled_peptide.has_link_at(switch_idx)) {
      //if not a self loop
      if (!shuffled_peptide.has_link_at(start_idx)) {
        //cout << "there is link at " << switch_idx << " to a " << shuffled[switch_idx] << endl;
        shuffled_peptide.remove_link(switch_idx);
        shuffled_peptide.add_link(start_idx);
      }
    } else if (shuffled_peptide.has_link_at(start_idx)) {
      //cout << "there is link at " << start_idx << " to a " << shuffled[start_idx] << endl;
      shuffled_peptide.remove_link(start_idx);
      shuffled_peptide.add_link(switch_idx);
    }
    ++start_idx;
  }
  //cout << shuffled << " after shuffle " << endl;
  shuffled_peptide.set_sequence(shuffled);
  return shuffled_peptide;
}


//
//
// Peptide and LinkedPeptide class method definitions below
//
//////////////////////////////////////////////////////////


void Peptide::set_sequence(string sequence) {
  sequence_ = sequence;
  length_ = sequence.length();  
}

void Peptide::remove_link(int index) {
  links[index] = false;
  num_links--;
}

bool LinkedPeptide::is_single() {
  return (peptides_.size() == 1 && peptides_[0].link_site() == -1);
} 

bool LinkedPeptide::is_dead_end() {
  return (peptides_.size() == 1 && peptides_[0].link_site() >= 0);
}

LinkedPeptide::LinkedPeptide(char* peptide_A, char* peptide_B, int posA, int posB, int charge) {
  mass_calculated = false;
  charge_ = charge;
  decoy_ = false;
  type_ = (ION_TYPE_T)NULL;
  Peptide pepA = Peptide(peptide_A);
  // if a self link or dead end
  if (peptide_B == NULL) {
     pepA.add_link(posA);
    if (posB >= 0)
      pepA.add_link(posB);
    peptides_.push_back(pepA);
  } else {
    //cout << "adding links at " << posA << " and " << posB << endl;
    Peptide pepB = Peptide(peptide_B);
    pepA.add_link(posA);
    pepB.add_link(posB);
    peptides_.push_back(pepA);
    peptides_.push_back(pepB);
  }
}




int Peptide::link_site() {
  for (int i = 0; i < length_; ++i) {
    if (has_link_at(i)) 
      return i;
  }
  return -1;
}

FLOAT_T Peptide::mass() {
  if (mass_calculated) 
    return mass_;
  else {
    mass_ = calc_sequence_mass((char*)sequence_.c_str(), MONO);
    mass_calculated = true;
    return mass_;
  }
}

// calculates mass of linked peptide,
// remove H2O from mass if it's a b-ion

FLOAT_T LinkedPeptide::mass() {
  if (!mass_calculated)
    calculate_mass();
  return mass_;
   
}

void LinkedPeptide::calculate_mass() {
  mass_ = calc_sequence_mass((char*)peptides_[0].sequence().c_str(), MONO);   
  if (peptides_[0].get_num_links() > 0) {
    mass_ += linker_mass;
  }

  if (size() == 2) {
    mass_ += calc_sequence_mass((char*)peptides_[1].sequence().c_str(), MONO);
  }
  //mass += MASS_H_MONO*charge_;
  if (type_ == B_ION) {
    mass_ = mass_ - MASS_H2O_MONO;
  } 
  mass_calculated = true;
}

// calculate mass to charge 
FLOAT_T LinkedPeptide::get_mz() {
  //if (mz < 5)
  //mz = mass_ / charge_; 
  mz = ((mass_ + MASS_H_MONO*charge_) / charge_);
  return mz;
}

void Peptide::add_link(int index) {
  //cout << "adding link " << endl;
  links[index] = true;
  num_links++;
}

// split between every amino acid on every peptide in the
// linked peptide.
void LinkedPeptide::split(vector<pair<LinkedPeptide, LinkedPeptide> >& ion_pairs) {
  bool is_loop = false;
  Peptide peptideA = peptides_[0];  
  Peptide peptideB = peptides_[0];
  // dead end
  if (is_dead_end()) {
   peptideB.set_sequence("");
  }
  // if a loop
  if (peptideA.get_num_links() == 2) {
    is_loop = true;
  } else if (is_linked()) {
    peptideB = peptides_[1];
  }
  for (int i = 1; i < peptideA.length(); ++i) {
    peptideA.split_at(i, ion_pairs, charge_, peptideB, is_loop);
  }
 
  if (is_linked()) {
    for (int i = 1; i < peptideB.length(); ++i) {
      peptideB.split_at(i, ion_pairs, charge_, peptideA, is_loop);
    }
  } 
}

// temporary
std::ostream &operator<< (std::ostream& os, LinkedPeptide& lp) {
  vector<Peptide> peptides = lp.peptides();
  ostringstream link_positions;
  link_positions << "(";
  for (int i = 0; i < peptides[0].length(); ++i) {
	if (peptides[0].has_link_at(i)) link_positions << i << "," ;
  }
  if (peptides.size() == 2) {
    for (int i = 0; i < peptides[1].length(); ++i) {
	if (peptides[1].has_link_at(i)) link_positions << i << ")";
    }
    return os << peptides[0].sequence() << ", " << peptides[1].sequence() << " " << link_positions.str();// << " +" << lp.charge();
  }
  string final = link_positions.str();
  if (final.length() > 1) final.erase(final.end()-1);
  return os << peptides[0].sequence() << " " << final << ")";// +" << lp.charge();
}

// this needs to change
void Peptide::split_at(int index, vector<pair<LinkedPeptide, LinkedPeptide> >& pairs, int charge, Peptide& other, bool is_loop) {
  bool self_flag = false;
  // for every charge state
  for (int c = 0; c <= charge; ++c) {
    Peptide pepA = Peptide(sequence_.substr(0, index));
    Peptide pepB = Peptide(sequence_.substr(index, length_ - index));
    LinkedPeptide linkedA = LinkedPeptide(c);
    LinkedPeptide linkedB = LinkedPeptide(charge - c);
    self_flag = true;
    // for every position on pepA
    for (int i = 0; i < index; i++) {
      if (has_link_at(i)) {
        pepA.add_link(i); 
        if (!other.empty() && !is_loop) linkedA.add_peptide(other);
        // if a loop, skip cleavages in between link sites (same mass as precursor)
	if (is_loop) self_flag = !self_flag;
      }
    }
    if (!self_flag) continue;
    // for every position on pepB
    for (int i = index; i < length_; i++) {
      if (has_link_at(i)) {
        pepB.add_link(i - index);
	if (!other.empty() && !is_loop) linkedB.add_peptide(other);
	//else (self_flag = !self_flag);
      }
    } 
    linkedA.add_peptide(pepA);
    linkedB.add_peptide(pepB);
    pairs.push_back(pair<LinkedPeptide, LinkedPeptide> (linkedA, linkedB));
  }
}


bool compareMass(const LinkedPeptide& lp1, const LinkedPeptide& lp2) {
  return lp1.mass_ < lp2.mass_;
}

void LinkedPeptide::sortByMass(std::vector<LinkedPeptide>& linked_peptides) {
  sort(linked_peptides.begin(), linked_peptides.end(), compareMass);
}
