/*
   This file contains classes that can be used to validate goals or assign new goals to a selected position. 
   Validation of a goal is basically a thorough search of the search space and therefore the name Searcher. 
*/

#ifndef __GOLSEARCHER_HPP 
#define __GOLSEARCHER_HPP

#include "root.hpp"
#include "GOLDomain.hpp"

#include "px/orangol_globals.hpp"

OGWRAPPER(GOL_Searcher)
class ORANGOL_API TGOL_Searcher : public TOrange {
public:
  __REGISTER_ABSTRACT_CLASS

    /*
        The method evaluates whether it is possible in the given state to achieve one of the goals from the goal list. 
        Search starts from search state and final goals are compared to start state. The provided movelist bring you from start state to search state. 
        Goals should be achieved considering maximum search depth and possibly (not implemented yet) maximum
        allowed number of states to explore.
    */
    virtual PGOL_EvaluateResult evaluateGoals(PGOL_State & start, PGOL_MoveList &, const PGOL_GoalList &, const int & maxDepth, const bool & storeIDs) = 0; // example position, a list of goals, depth, max. num of states
    /* evaluates a set of goals in parallel */  
    virtual PGOL_EvaluateResultList evaluateGoalsSet(PGOL_State & start, PGOL_MoveList & moves, const PGOL_GoalList & gl, const int & maxDepth)=0;

    /* Returns a two sets of goals. The first set is a set of good goals - goals that lead to a decrease of DTM and the
            second is a set of unachievable goals. Unachievable goals are not actually returned but added to the provided goal list. */
    virtual PGOL_EvaluateResultList assignGoals(PGOL_State , const int &, const int &, const int &, PGOL_GoalList start, bool specialize, PGOL_GoalList unach) = 0; //example position, depth, max conditions
};

class ORANGOL_API TGOL_ANDORSearcher : public TGOL_Searcher {
public:
    __REGISTER_CLASS

    float depthpen;

    TGOL_ANDORSearcher(void);
    virtual PGOL_EvaluateResult evaluateGoals(PGOL_State & start, PGOL_MoveList &, const PGOL_GoalList &, const int & maxDepth, const bool & storeIDs);
    virtual PGOL_EvaluateResultList assignGoals(PGOL_State, const int &, const int &, const int &, PGOL_GoalList start, bool specialize, PGOL_GoalList unach);
    virtual PGOL_EvaluateResultList evaluateGoalsSet(PGOL_State & start, PGOL_MoveList & moves, const PGOL_GoalList & gl, const int & maxDepth);

    PGOL_EvaluateResult evaluateGoals(PGOL_State &, PGOL_State &, const PGOL_GoalList &, vector<string> & path, const vector<bool> & holding, const int & maxDepth, int depth, const bool storeIDs);

    /* The method evaluates a set of goals. It returns a list of TGOL_EvaluateResult objects in an arraylist, where objects
    correspond by index to goals in goalsToEvaluate. */
    PGOL_EvaluateResultList getAchievableGoals(PGOL_State start, PGOL_State current, PGOL_GoalList goalsToEvaluate, vector<string> & path, const int & maxDepth, int depth);
    /*
    Evaluates a set of goals in the current position.
    */
    PGOL_EvaluateResultList evaluateGoalsPosition(PGOL_State start, PGOL_State current, PGOL_GoalList goalsToEvaluate, const int & maxDepth, const int & depth);
};


#endif