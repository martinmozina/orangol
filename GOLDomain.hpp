/*
   Main two classes used to describe search space.
*/

#ifndef __GOLDOMAIN_HPP 
#define __GOLDOMAIN_HPP

#include <string>
#include <sstream>
using namespace std;

#include "root.hpp"
#include "orvector.hpp"
#include "values.hpp"
#include "domain.hpp"

#include "px/orangol_globals.hpp"

OGWRAPPER(GOL_Move)
#define TGOL_MoveList TOrangeVector<PGOL_Move>
OGVWRAPPER(GOL_MoveList)

OGWRAPPER(GOL_SubGoal)
OGWRAPPER(GOL_Goal)
#define TGOL_SubGoalList TOrangeVector<PGOL_SubGoal>
OGVWRAPPER(GOL_SubGoalList)
#define TGOL_GoalList TOrangeVector<PGOL_Goal>
OGVWRAPPER(GOL_GoalList)
OGWRAPPER(GOL_EvaluateResult)
#define TGOL_EvaluateResultList TOrangeVector<PGOL_EvaluateResult>
OGVWRAPPER(GOL_EvaluateResultList)

class ORANGOL_API TGOL_Move : public TOrange {
public:
    __REGISTER_CLASS

    TGOL_Move(void);
    ~TGOL_Move(void);

    virtual string toString(void) const;
};

OGWRAPPER(GOL_State)
class ORANGOL_API TGOL_State : public TOrange {
public:
    __REGISTER_ABSTRACT_CLASS
    PDomain domain;  //P domain of all attributes representing a state.

    virtual PGOL_MoveList getMoves() const = 0;
    virtual void doMove(const PGOL_Move & move) = 0;
    virtual void undoMove(const PGOL_Move & move) = 0;
    virtual double evaluate() const = 0;
    virtual double pruneProb(const PGOL_State &, const PGOL_GoalList &, const int & maxDepth, const int & depth) const = 0;
    virtual bool orNode() const = 0;
    virtual TValue & getAttributeValue(const int & position) const = 0;
    virtual string id(void) const = 0;
    virtual PGOL_State deepCopy() const;
    /*
        This method returns a set of possible subgoals in the given domain. It considers "only" the following operators:
        a)EnumVariable: NoChange and Equals,
        b)FloatVariable: Increase, Decrease, NonIncrease, NonDecrease

        If other operators are needed or domain contains other types of variables, 
        you should implement a new "subGoals" method in the inherited state class. 
    */
    virtual PGOL_SubGoalList subGoals() const;
    virtual PGOL_GoalList expandGoals(PGOL_GoalList parents, PGOL_SubGoalList specializations) const;
};

/* Results of evaluating goals in a particular state.*/
class ORANGOL_API TGOL_EvaluateResult : public TOrange {
public:
    __REGISTER_CLASS
    float bestEval; //P evaluation of the best execution
    float worstEval; //P evaluation of the worst execution
    PGOL_MoveList bestPV; //P PV of the best execution
    PGOL_MoveList worstPV; //P PV of the worst execution
    float visited; //P number of visited nodes while searching for this goal
    PGOL_GoalList bestGoals; //P achieved goals in the best execution
    PGOL_GoalList worstGoals; //P achieved goals in the worst execution
    int bestGoalIndex; //P index of first achieved goal in best goals
    int worstGoalIndex; //P index of first achieved goal in worst goals
    float pruneProb; //P the probability that a human (player) will prune this branch 
    
    bool achievable; //P flag whether the goal can be achieved.
    PGOL_Goal goal; //P Goal

    PStringList achievedIDs; //P ids of all states where goal was achieved

    TGOL_EvaluateResult(void);
    ~TGOL_EvaluateResult(void);
    string toString();
};

class ORANGOL_API TGOL_Goal : public TOrange {
public:
    __REGISTER_CLASS

    PGOL_SubGoalList subGoals; //P a list of subgoals
    PGOL_Goal parent; //P parent goal from which this goal was derived
    PGOL_EvaluateResult eval; //P Evaluation of goal (needed in the method for goal assignment)

    TGOL_Goal(void);
    TGOL_Goal(PGOL_SubGoalList subGoals, PGOL_Goal parent);
    ~TGOL_Goal(void);

    // Returns true when goal is achieved, otherwise false.
    bool operator()(const PGOL_State & start, const PGOL_State & end) const;
    // Returns true if all holding goals are satisfies
    bool checkHolding(const PGOL_State & start, const PGOL_State & end) const;

    string toString(void) const;
    int size(void) const;
};

/*
    Class subgoal deals with a single sub-goal. Subgoals consistos of an attribute and its change.
*/
class ORANGOL_API TGOL_SubGoal : public TOrange {
public:
  __REGISTER_CLASS
    CLASSCONSTANTS(Operator) enum Operator { NoChange, Increase, Decrease, NonIncrease, NonDecrease, Equals, GreaterThan, LessThan, External};

    int oper; //P Subgoal's operator
    int position; //P position of variable to compare
    bool holdingGoal; //P holding goal: if set to true a goal must be kept through the whole solution of the goal, otherwise only at the end.
    PValueList values; //P accepted values used with Equals, LargerThan and LessThan operators
    PDomain domain; //P orange domain of attributes

    TGOL_SubGoal(void);
    TGOL_SubGoal(int, TGOL_SubGoal::Operator, bool, PDomain, PValueList = PValueList());
    ~TGOL_SubGoal(void);

    // Is goal achieved in given start and end positions?
    bool operator()(const PGOL_State & start, const PGOL_State & end) const;

    // special external operator if needed for certain domains
    virtual bool externalCompare(const PGOL_State & start, const PGOL_State & end) const;

    // a goal is progressive when it is not true in the original state
    bool isProgressive(PGOL_State start) const;

    // is this goal a holding goal?
    bool isHoldingGoal() const;

    string toString(void) const;

    const string operatorString(int o) const;
};

#endif