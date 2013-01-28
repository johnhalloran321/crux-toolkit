/*************************************************************************//**
 * \file AbstractMatch.cpp
 * AUTHOR: Sean McIlwain
 * CREATE DATE: 1/26 2013
 * \brief Object for holding match scores.
 ****************************************************************************/
#include "AbstractMatch.h"
#include "carp.h"
  
/**
 * \returns a new memory allocated abstract match
 */
AbstractMatch::AbstractMatch() {
}

/**
 * default destructor
 */
AbstractMatch::~AbstractMatch() {
}


/**
 * \returns the match score for a particular score type
 */
FLOAT_T AbstractMatch::getScore(
  SCORER_TYPE_T type ///< score type desired
  ) {

  if (scores_.find(type) == scores_.end()) {
    carp(CARP_WARNING, "Score not set!");
    return 0.0;
  }
  return scores_[type];

}

/**
 * sets the match score for particular score type
 */
void AbstractMatch::setScore(
  SCORER_TYPE_T type, ///< score to set
  FLOAT_T score ///< score value
  ) {

  scores_[type] = score;  

}

/**
 * \returns whether the match has a particular score type assigned
 */
bool AbstractMatch::hasScore(
  SCORER_TYPE_T type ///< score to test
  ) {

  return (scores_.find(type) != scores_.end());
}

/**
 * \returns whether the match is a decoy or not (default false)
 */
bool AbstractMatch::isDecoy() {

  return false;
}
  
/**
 * \returns the beginning iterator for all set scores in the match
 */
ScoreMapIterator AbstractMatch::scoresBegin() {
  return scores_.begin();
}

/**
 * \returns the end iterator for all set scores in the match
 */
ScoreMapIterator AbstractMatch::scoresEnd() {
  return scores_.end();
}


/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 2
 * End:
 */
/*************************************************************************//**
 * \file AbstractMatch.cpp
 * AUTHOR: Sean McIlwain
 * CREATE DATE: 1/26 2013
 * \brief Object for holding match scores.
 ****************************************************************************/
#include "AbstractMatch.h"
#include "carp.h"
  
/**
 * \returns a new memory allocated abstract match
 */
AbstractMatch::AbstractMatch() {
}

/**
 * default destructor
 */
AbstractMatch::~AbstractMatch() {
}


/**
 * \returns the match score for a particular score type
 */
FLOAT_T AbstractMatch::getScore(
  SCORER_TYPE_T type ///< score type desired
  ) {

  if (scores_.find(type) == scores_.end()) {
    carp(CARP_WARNING, "Score not set!");
    return 0.0;
  }
  return scores_[type];

}

/**
 * sets the match score for particular score type
 */
void AbstractMatch::setScore(
  SCORER_TYPE_T type, ///< score to set
  FLOAT_T score ///< score value
  ) {

  scores_[type] = score;  

}

/**
 * \returns whether the match has a particular score type assigned
 */
bool AbstractMatch::hasScore(
  SCORER_TYPE_T type ///< score to test
  ) {

  return (scores_.find(type) != scores_.end());
}

/**
 * \returns whether the match is a decoy or not (default false)
 */
bool AbstractMatch::isDecoy() {

  return false;
}
  
/**
 * \returns the beginning iterator for all set scores in the match
 */
ScoreMapIterator AbstractMatch::scoresBegin() {
  return scores_.begin();
}

/**
 * \returns the end iterator for all set scores in the match
 */
ScoreMapIterator AbstractMatch::scoresEnd() {
  return scores_.end();
}


/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 2
 * End:
 */