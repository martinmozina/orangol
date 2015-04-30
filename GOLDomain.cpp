#include "ppp/GOLDomain.ppp"
#include "px/externs.px"
#include <iostream>

DEFINE_TOrangeVector_classDescription(PGOL_SubGoal, "TGOL_SubGoalList", true, ORANGOL_API)
DEFINE_TOrangeVector_classDescription(PGOL_Goal, "TGOL_GoalList", true, ORANGOL_API)
DEFINE_TOrangeVector_classDescription(PGOL_Move, "TGOL_MoveList", true, ORANGOL_API)
DEFINE_TOrangeVector_classDescription(PGOL_EvaluateResult, "TGOL_EvaluateResultList", true, ORANGOL_API)

TGOL_Move::TGOL_Move(void)
{}

TGOL_Move::~TGOL_Move(void)
{}

string TGOL_Move::toString(void) const
{
    raiseWarning("toString not implemented");
    return "";
}

PGOL_State TGOL_State::deepCopy() const
{
  raiseWarning("Deep copy not implemented.");
  return PGOL_State();
}

PGOL_SubGoalList TGOL_State::subGoals() const
{
    TGOL_SubGoalList * t_sglist = mlnew TGOL_SubGoalList();
    PGOL_SubGoalList sglist = t_sglist;

    // loop over all attributes
    const int size = domain->attributes->size();
    for (int ai=0; ai<size; ai++)
    {
        // loop over all possible changes 
        for (int operi = 0; operi < TGOL_SubGoal::External; operi++)
        {
            //const TGOL_SubGoal::Operator oper = (TGOL_SubGoal::Operator) operi;
            const TFloatVariable *fvar = (domain->attributes->at(ai)).AS(TFloatVariable);
            const TEnumVariable *evar = (domain->attributes->at(ai)).AS(TEnumVariable);
            if (fvar) // dealing with a float variable;
            {
                if (operi == TGOL_SubGoal::Increase ||
                    operi == TGOL_SubGoal::Decrease ||
                    operi == TGOL_SubGoal::NonIncrease ||
                    operi == TGOL_SubGoal::NonDecrease)
                {   
                    TGOL_SubGoal * sg = mlnew TGOL_SubGoal(ai, operi, false, domain);
                    sglist->push_back(sg);
                    if ( operi == TGOL_SubGoal::NonIncrease ||
                         operi == TGOL_SubGoal::NonDecrease)
                    {
                        sg = mlnew TGOL_SubGoal(ai, operi, true, domain);
                        sglist->push_back(sg);
                    }
                }

            }
            else if (evar)  // dealing with a discrete variable
            {
                if (operi == TGOL_SubGoal::NoChange)
                {   TGOL_SubGoal * sg = mlnew TGOL_SubGoal(ai, operi, false, domain);
                    sglist->push_back(sg);
                    sg = mlnew TGOL_SubGoal(ai, operi, true, domain);
                    sglist->push_back(sg);
                }
                else if (operi == TGOL_SubGoal::Equals)
                {
                    // iterate over attribute's values
                    const int values_size = evar->values->size();
                    for (int vi=0; vi<values_size; vi++)
                    {
                        TValueList *values = mlnew TValueList();
                        PValueList wValues = values;
                        wValues->push_back(TValue(vi));

                        TGOL_SubGoal * sg = mlnew TGOL_SubGoal(ai, operi, false, domain, wValues);
                        sglist->push_back(sg);
                    }
                }
            }
            else
            {
                raiseWarning("Unknown variable type.");
                return PGOL_SubGoalList();
            }
        }
    }
    return sglist;
}

PGOL_GoalList TGOL_State::expandGoals(PGOL_GoalList parents, PGOL_SubGoalList specializations) const
{
    // create an empty list of goals
    TGOL_GoalList * ng = mlnew TGOL_GoalList();
    PGOL_GoalList newGoals = ng;

    // get current state
    PGOL_State w = const_cast<TGOL_State *>(this);

    // deprecated: if not parents, select only progressive goals
    // used to be like that, now it adds all goals
    if (!parents)
    {
        PITERATE(TGOL_SubGoalList, s, specializations)
        {
            // now prepare a new goal having only s as subgoal
            TGOL_SubGoalList * ts = mlnew TGOL_SubGoalList();
            PGOL_SubGoalList ws = ts;
            ws->push_back(*s);
            TGOL_Goal * g = mlnew TGOL_Goal(ws, NULL);
            // add it to newGoals
            newGoals->push_back(g);
        }
        return newGoals;
    }
    // iterate through all parents
    PITERATE(TGOL_GoalList, p, parents)
    {
        // iterate through specializations
        PITERATE(TGOL_SubGoalList, s, specializations)
        {
            if (!(*s)->isProgressive(w))
            {
                // combine parent's subgoals and the subgoal s
                TGOL_SubGoalList * psg = mlnew TGOL_SubGoalList();
                PGOL_SubGoalList  ws = psg;
                PITERATE(TGOL_SubGoalList, sg, (*p)->subGoals)
                {
                    ws->push_back(*sg);
                }
                ws->push_back(*s);
                TGOL_Goal * g = mlnew TGOL_Goal(ws, *p);
                // add it to newGoals
                newGoals->push_back(g);
            }
        }
    }
    return newGoals;
}

TGOL_EvaluateResult::TGOL_EvaluateResult(void)
: achievable(false)
{
}

TGOL_EvaluateResult::~TGOL_EvaluateResult(void)
{
}

TGOL_Goal::TGOL_Goal(void)
: subGoals(NULL),
  parent(NULL),
  eval(NULL)
{
}

TGOL_Goal::TGOL_Goal(PGOL_SubGoalList subGoals, PGOL_Goal parent)
: subGoals(subGoals),
  parent(parent),
  eval(NULL)
{
}

TGOL_Goal::~TGOL_Goal(void)
{
}

string TGOL_Goal::toString(void) const
{
    printf("in to string ... \n");
    stringstream s;
    int size = subGoals->size();
    cout << "size: " << size;
    for (int i = 0; i < size; i++) {
        s << subGoals->at(i)->toString();
        if (i < size-1)
            s << ",";
    }
    cout << "out of string\n";
    cout << s.str() << "\n";
    return s.str();
}

int TGOL_Goal::size(void) const
{
    return subGoals->size();
}

bool TGOL_Goal::operator()(const PGOL_State & start, const PGOL_State & end) const
{
    const_PITERATE(TGOL_SubGoalList, sg, subGoals)
        if (!(*sg)->call(start, end))
            return false;
/*    int size = subGoals->size();
    for (int i = 0; i < size; i++) {
        if (!subGoals->at(i)->call(start, end))
            return false;
    }*/
    return true;
}

bool TGOL_Goal::checkHolding(const PGOL_State & start, const PGOL_State & end) const
{
    const_PITERATE(TGOL_SubGoalList, sg, subGoals)
    {
        if ((*sg)->isHoldingGoal() && !(*sg)->call(start, end))
            return false;
    }
    return true;
}


TGOL_SubGoal::TGOL_SubGoal(void)
: position(-1),
  values(NULL),
  oper(0),
  holdingGoal(false),
  domain(NULL)
{}

TGOL_SubGoal::TGOL_SubGoal(int p, int op, bool hg, PDomain domain, PValueList vl)
: position(p),
  values(vl),
  oper(op),
  holdingGoal(hg),
  domain(domain)
{}

bool TGOL_SubGoal::operator()(const PGOL_State & start, const PGOL_State & end) const
{
    TValue e = end->getAttributeValue(position);
    TValue s = start->getAttributeValue(position);
    bool found = false;
    switch (oper) {
        case Equals:      
            for (int i=values->size()-1; i>=0; i--)
                if (e.compare(values->at(i)) == 0)
                    found = true;
            return found;
        case NoChange:    return e.compare(s) == 0;
        case Increase:    return e.compare(s) > 0;
        case Decrease:    return e.compare(s) < 0;
        case NonIncrease: return e.compare(s) <= 0;
        case NonDecrease: return e.compare(s) >= 0;
        case GreaterThan: return e.compare(values->at(0)) > 0;
        case LessThan:    return e.compare(values->at(0)) < 0;
        case External:    return externalCompare(start, end);
        default:  return false;
    }
}

bool TGOL_SubGoal::externalCompare(const PGOL_State & start, const PGOL_State & end) const
{
  raiseWarning("External compare not implemented.");
  return true;
}

TGOL_SubGoal::~TGOL_SubGoal(void)
{
}

const string TGOL_SubGoal::operatorString(int o) const
{
    switch (o) {
        case Equals:      return "Equals";
        case NoChange:    return "NoChange";
        case Increase:    return "Increase";
        case Decrease:    return "Decrease";
        case NonIncrease: return "NonIncrease";
        case NonDecrease: return "NonDecrease";
        case GreaterThan: return "GreaterThan";
        case LessThan:    return "LessThan";
        case External:    return "External";
    }
    return "";
}


string TGOL_SubGoal::toString(void) const
{
    string hold = "";
    if (holdingGoal)
        hold = "(holding)";
    stringstream strvals;
    stringstream shortDesc;
    shortDesc << position << "|" << oper << "|" << holdingGoal;
    cout << position << "|" << oper << "|" << holdingGoal;

    if (values)
    {
        strvals << " ";
        int vi = 0;
        cout << "in values " << strvals.str() << " \n";
        const_PITERATE(TValueList, v, values)
        {
            if (vi > 0)
                strvals << "/";
            shortDesc << "|";
            cout << "1";
            cout << " i " << vi << "\n";
            cout << strvals.str() << "\n";
            if ((*v).varType == (*v).INTVAR)
            {
                PVariable tmpv = domain->attributes->at(position);
                TEnumVariable * ev = dynamic_cast<TEnumVariable *>(tmpv.getUnwrappedPtr());
                cout << "vals " << ev->values->at((*v).intV) << "\n";
                strvals << ev->values->at((*v).intV);
                cout << strvals.str() << "\n";
                shortDesc << (*v).intV;
            }
            if ((*v).varType == (*v).FLOATVAR)
            {
                strvals << (*v).floatV;
                shortDesc << (*v).floatV;
            }
            vi+=1;
        }
    }
    else 
        strvals << "";

                cout << strvals.str() << "\n";
    cout << "short " <<  shortDesc.str() << "\n";
                cout << strvals.str() << "\n";
    cout << "oper " <<  oper << "\n";
                cout << strvals.str() << "\n";
    cout << "operator str " <<  operatorString(oper) << "\n";
                cout << strvals.str() << "\n";
    cout << "name  " <<  domain->attributes->at(position)->get_name() << "\n";
    cout << "kr neki prej prej prej \n";
    cout << "kr neki prej prej prej \n";
    cout << "kr neki prej prej prej \n";
    cout << "kr neki prej prej prej \n";
    cout << "kr neki prej prej prej \n";
    cout << "kr neki prej prej prej \n";
    cout << "str vals  " <<  strvals.str() << "\n";
    cout << "kr neki \n";
    cout << "kr neki \n";
    cout << "kr neki \n";
    cout << "kr neki \n";
    cout << "kr neki \n";
    cout << "kr neki \n";
    cout << "kr neki \n";
    cout << "kr neki \n";
    cout << "kr neki \n";
    cout << flush;
    stringstream s;
    s << "[" << shortDesc.str() << "]" << domain->attributes->at(position)->get_name() << " " << operatorString(oper) << hold << strvals.str();
    cout << "s" << s << "\n";
    cout << flush;
    return s.str();
}

bool TGOL_SubGoal::isProgressive(PGOL_State start) const
{
    return !(this->call(start,start));
}

bool TGOL_SubGoal::isHoldingGoal() const
{
    return holdingGoal;
}