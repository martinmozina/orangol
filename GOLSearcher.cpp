#include "GOLSearcher.ppp"

string TGOL_EvaluateResult::toString(void)
{
    stringstream s;
    s << "Best execution: \n";
    s << "Eval: " << bestEval << "\n";
    if (bestPV)
    {
        s << "Moves: ";
        PITERATE(TGOL_MoveList, m, bestPV) {
            s << "  " << (*m)->toString() << ";";
        }
    }
    if (bestGoals)
    {
        s << "\nGoals: ";
        PITERATE(TGOL_GoalList, g, bestGoals) {
            s << "  " << (*g)->toString() << ";";
        }
    }
    s << "\nWorst execution: \n";
    s << "Eval: " << worstEval << "\n";
    if (worstPV)
    {
        s << "Moves: ";
        PITERATE(TGOL_MoveList, m, worstPV) {
            s << "  " << (*m)->toString() << ";";
        }
    }
    if (worstGoals)
    {
        s << "\nGoals: ";
        PITERATE(TGOL_GoalList, g, worstGoals) {
            s << "  " << (*g)->toString() << ";";
        }
    }
    s << "\nVisited nodes: " << visited << "\n";
    return s.str();
}

TGOL_ANDORSearcher::TGOL_ANDORSearcher(void)
{
    depthpen = 0.5f;
}

/*PGOL_EvaluateResult TGOL_ANDORSearcher::evaluateGoals(PGOL_State & example, const PGOL_GoalList & gl, const int & maxDepth)
{
    PGOL_State searchExample = example->deepCopy();
    vector<string> path;
    vector<bool> holding;
    return evaluateGoals(example, searchExample, gl, path, holding, maxDepth, 0);
}*/

PGOL_EvaluateResult TGOL_ANDORSearcher::evaluateGoals(PGOL_State & start, PGOL_MoveList & moves, const PGOL_GoalList & gl, const int & maxDepth, const bool & storeIDs)
{
    PGOL_State search = start->deepCopy();
    vector<string> path;
    PITERATE(TGOL_MoveList, m, moves)
    {
        path.push_back(search->id());
        search->doMove(*m);
    }
    vector<bool> holding;
    const_PITERATE(TGOL_GoalList, g, gl)
        holding.push_back(true);
    return evaluateGoals(start, search, gl, path, holding, maxDepth, moves->size(), storeIDs);
}

PGOL_EvaluateResultList TGOL_ANDORSearcher::evaluateGoalsSet(PGOL_State & start, PGOL_MoveList & moves, const PGOL_GoalList & gl, const int & maxDepth)
{
    PGOL_State search = start->deepCopy();
    vector<string> path;
    PITERATE(TGOL_MoveList, m, moves)
    {
        path.push_back(search->id());
        search->doMove(*m);
    }
    return getAchievableGoals(start, search, gl, path, maxDepth, moves->size());
}

// evaluates all goals in the current position
PGOL_EvaluateResultList TGOL_ANDORSearcher::evaluateGoalsPosition(PGOL_State start, PGOL_State current, PGOL_GoalList goalsToEvaluate, const int & maxDepth, const int & depth)
{
    TGOL_EvaluateResultList * erl = mlnew TGOL_EvaluateResultList();
    PGOL_EvaluateResultList wrl = erl;

    PITERATE(TGOL_GoalList, g, goalsToEvaluate)
    {
        // goal was already achieved or holding goals not satisfied
        if (!(*g))
            wrl->push_back(PGOL_EvaluateResult());
        else if (depth > 0 && (*g)->call(start, current))
        {
            TGOL_EvaluateResult * e = mlnew TGOL_EvaluateResult();
            // set eval
            float eval = current->evaluate(); // evaluate is not needed for achievableGoals, is here only for info
            e->bestEval = eval;
            e->worstEval = eval;
            // set empty variations
            TGOL_MoveList * tmp1 = mlnew TGOL_MoveList();
            e->bestPV = tmp1;
            TGOL_MoveList * tmp2 = mlnew TGOL_MoveList();
            e->worstPV = tmp2; 
            // set visited
            e->visited = 1; 
            e->achievable = true;
            PGOL_GoalList tmpl = new TGOL_GoalList();
            tmpl->push_back(*g);
            e->pruneProb = current->pruneProb(start, tmpl, maxDepth, depth);
            // best and worst goals are not needed here
            erl->push_back(e);
        }
        else // goal is not achievable
        {
            TGOL_EvaluateResult * e = mlnew TGOL_EvaluateResult();
            e->achievable = false;
            e->visited = 1;
            PGOL_GoalList tmpl = new TGOL_GoalList();
            tmpl->push_back(*g);
            e->pruneProb = current->pruneProb(start, tmpl, maxDepth, depth);
            wrl->push_back(e);
        }
    }
    return wrl;
}

// Returns all achievable goals from goalsToEvaluate in start state.
PGOL_EvaluateResultList TGOL_ANDORSearcher::getAchievableGoals(PGOL_State start, PGOL_State current, PGOL_GoalList goalsToEvaluate, vector<string> & path, const int & maxDepth, int depth)
{
        // evaluate solved goals in this position (and set holding goals)
        PGOL_EvaluateResultList goalsHereResult = evaluateGoalsPosition(start, current, goalsToEvaluate, maxDepth, depth);

        // if depth exhaustes, stop
        if (depth == maxDepth)
            return goalsHereResult;

        // get moves and determine type of node
        PGOL_MoveList moves = current->getMoves();
        if (moves->size() == 0)
            return goalsHereResult;

        // if cycle, stop searching
        const string id = current->id();
        for (vector<string>::iterator pi = path.begin(); pi != path.end(); pi++)
            if (id.compare(*pi) == 0)
                return goalsHereResult;

        bool orNode = current->orNode();

        // set achieved goals to null (to prevent further evaluation)
        TGOL_GoalList * tmpList = mlnew TGOL_GoalList();
        TGOL_GoalList::const_iterator gli(goalsToEvaluate->begin());
        TGOL_EvaluateResultList::const_iterator rli(goalsHereResult->begin());
        // if all goals are null (all goals are either solved or their holding goals are not achieved), stop searching
        bool allNull = true;
        for (; gli != goalsToEvaluate->end(); gli++, rli++)
        {
            // continue evaluating goal if it is not achievable and holding goals hold
            if ((*rli) && !(*rli)->achievable && (*gli)->checkHolding(start, current))
            {
                tmpList->push_back((*gli));
                allNull = false;
            }
            else
                tmpList->push_back(PGOL_GoalList());
        }
        if (allNull)
            return goalsHereResult;

        goalsToEvaluate = tmpList;

        // iterate through moves and store results
        vector<PGOL_EvaluateResultList> results_subtree;
        PITERATE(TGOL_MoveList, mi, moves) {
            float ceval = current->evaluate();
            current->doMove(*mi);
            path.push_back(id);
            PGOL_EvaluateResultList found = getAchievableGoals(start, current, goalsToEvaluate, path, maxDepth, depth+1);
            if (found)
            {
                results_subtree.push_back(found);
                // add the move to each result
                PITERATE(TGOL_EvaluateResultList, rl, found)
                {
                    if ((*rl) && (*rl)->achievable)
                    {
                        (*rl)->bestPV->push_back(*mi);
                        (*rl)->worstPV->push_back(*mi);
                    }
                }
            }
            path.pop_back();
            current->undoMove(*mi);
        }

        // Combine results in goalsHere if orNode
        if (orNode)
        {
            // we can select whatever move
            // iterate through all goals that are not achievable here
            TGOL_GoalList::const_iterator gi(goalsToEvaluate->begin());
            TGOL_EvaluateResultList::iterator ri(goalsHereResult->begin());
            for (int goalIndex=0; gi != goalsToEvaluate->end(); gi++, ri++, goalIndex++)
            {
                if (!(*gi) || !(*ri))
                    continue;

                // determine goal's complexity
                // au = average weight of unachievable branches
                // aa = average weight of achievable branches
                // A = sum of weights of achievable branches
                // U = sum of weights of unachievable branches
                // complexity = U / (A + au) * au + aa

                // how to evaluate best and worst evaluations (best and worst PVs are simply corresponding PVs)
                // among all achievable goals simply select the one with highest and the  one with lowest 
                // evaluations
                float au, aa, A=0.0f, U=0.0f, nu=0.0f, na=0.0f;
                float bestEval=-1, worstEval=-1;
                int bestEvalSize = -1;
                int bestEvalIndex=-1, worstEvalIndex=-1;
                vector<PGOL_EvaluateResultList>::const_iterator it = results_subtree.begin();
                vector<PGOL_EvaluateResultList>::const_iterator end = results_subtree.end();
                for (int moveIndex = 0; it != end; it++, moveIndex++)
                {
                    const PGOL_EvaluateResult r = (*it)->at(goalIndex);
                    if (r->achievable)
                    {
                        const float pen = (1-r->pruneProb) * pow(depthpen, r->bestPV->size());
                        na+=pen;
                        A+=r->visited*pen;
                        // find best and worst eval
                        if (bestEvalIndex == -1 || r->bestPV->size() < bestEvalSize || (r->bestPV->size() == bestEvalSize && r->bestEval < bestEval))
                        {
                            bestEval = r->bestEval;
                            bestEvalIndex = moveIndex;
                            bestEvalSize = r->bestPV->size();
                        }
                        if (worstEvalIndex == -1 || r->worstEval > worstEval)
                        {
                            worstEval = r->worstEval;
                            worstEvalIndex = moveIndex;
                        }
                    }
                    else
                    {
                        const float pen = (1-r->pruneProb) * pow(depthpen, maxDepth - depth);
                        nu+=pen;
                        U+=r->visited*pen;
                    }
                }
                if (na < 1e-6)
                    (*ri)->visited = 1 + U;
                else if (nu < 1e-6)
                    (*ri)->visited = 1 + A/na;
                else
                {
                    au = U/nu;
                    aa = A/na;
                    (*ri)->visited = 1 + U / (A + au) * au + aa;
                }
                //
                (*ri)->achievable = (bestEvalIndex==-1) ? false : true;
                if (!(*ri)->achievable)
                    continue;
                (*ri)->bestEval = bestEval;
                (*ri)->bestPV = results_subtree.at(bestEvalIndex)->at(goalIndex)->bestPV;
                (*ri)->worstEval = worstEval;
                (*ri)->worstPV = results_subtree.at(worstEvalIndex)->at(goalIndex)->worstPV;
            }
        }
        else // AND Node
        {
            // the opponent will select the move with worst realization of the goal (or the one where goal is not achievable)
            // therefore: find move that bring worst bestGoal and move that brings worst worst goal.

            // how to compute visited, which corresponds to complexity?
            // if goal is achievable, complexity is the sum of all complexity (you need to check all possible opponent moves)
            // if goal is not achievable then you need to find one move where goal is not achievable.
            // therefore: the procedure is exactly the same as for OR node, only roles are different.
            TGOL_GoalList::const_iterator gi(goalsToEvaluate->begin());
            TGOL_EvaluateResultList::iterator ri(goalsHereResult->begin());
            for (int goalIndex=0; gi != goalsToEvaluate->end(); gi++, ri++, goalIndex++)
            {
                if (!(*gi) || !(*ri))
                    continue;
                float au, aa, A=0.0f, U=0.0f, nu=0.0f, na=0.0f;
                float bestEval=-1, worstEval=-1;
                int bestEvalIndex=0, worstEvalIndex=0;
                vector<PGOL_EvaluateResultList>::const_iterator it = results_subtree.begin();
                vector<PGOL_EvaluateResultList>::const_iterator end = results_subtree.end();
                for (int moveIndex=0; it != end; it++, moveIndex++)
                {
                    const PGOL_EvaluateResult r = (*it)->at(goalIndex);
                    if (r->achievable)
                    {
                        const float pen = (1-r->pruneProb) * pow(depthpen, r->bestPV->size());
                        na+=pen;
                        A+=r->visited*pen;
                        // find best and worst eval
                        if (bestEvalIndex == -1)
                            continue;
                        if (bestEval == -1 || r->bestEval > bestEval)
                        {
                            bestEval = r->bestEval;
                            bestEvalIndex = moveIndex;
                        }
                        if (worstEval == -1 || r->worstEval > worstEval)
                        {
                            worstEval = r->worstEval;
                            worstEvalIndex = moveIndex;
                        }
                    }
                    else
                    {
                        const float pen = (1-r->pruneProb) * pow(depthpen, maxDepth - depth);
                        nu+=pen;
                        U+=r->visited*pen;
                        bestEvalIndex = -1;
                    }
                }
                if (nu < 1e-6)
                    (*ri)->visited = 1 + A;
                else if (na < 1e-6)
                    (*ri)->visited = 1 + U/nu;
                else
                {
                    au = U/nu;
                    aa = A/na;
                    (*ri)->visited = 1 + A / (U + aa) * aa + au;
                }

                //
                (*ri)->achievable = (bestEvalIndex==-1) ? false : true;
                if (!(*ri)->achievable)
                    continue;
                (*ri)->bestEval = bestEval;
                (*ri)->bestPV = results_subtree.at(bestEvalIndex)->at(goalIndex)->bestPV;
                (*ri)->worstEval = worstEval;
                (*ri)->worstPV = results_subtree.at(worstEvalIndex)->at(goalIndex)->worstPV;
            }
        }
        return goalsHereResult;
}

// compare improvements of best executions
bool bestExec(const PGOL_Goal &g1, const PGOL_Goal &g2)
{ return  g1->eval->bestEval >= g2->eval->bestEval; }

PGOL_EvaluateResultList TGOL_ANDORSearcher::assignGoals(PGOL_State example, const int & depth, const int & maxLength, const int & K, PGOL_GoalList start, bool specialize, PGOL_GoalList unachGoals)
{
    // create empty good goals and clear unachievalbe goals
    TGOL_EvaluateResultList * gg = mlnew TGOL_EvaluateResultList();
    PGOL_EvaluateResultList goodGoals = gg;
    unachGoals->clear();
    goodGoals->clear();

    // get all subgoals
    PGOL_SubGoalList subGoals = example->subGoals();

    // create a set of initial goals to evaluate
    // and set goalsToEvaluate = initialGoals
    PGOL_GoalList goalsToEvaluate;
    if (!start)
        goalsToEvaluate = example->expandGoals(NULL, subGoals);
    else 
    {
        goalsToEvaluate = mlnew TGOL_GoalList();
        const_PITERATE(TGOL_GoalList, s, start)
            goalsToEvaluate->push_back(*s);
    }
    // 
    float startEval = example->evaluate();

    // while goalsToEvaluate and number of subgoals in goalsToEvaluate is less or equal maxLength
    while (goalsToEvaluate->size() > 0 && goalsToEvaluate->at(0)->size() < maxLength)
    {
/*        printf("goals to evaluate \n");
        PITERATE(TGOL_GoalList, g, goalsToEvaluate)
        {
            printf("goal: %s\n", (*g)->toString().c_str());
        } */
        // find achievable goals
        PGOL_State searchExample = example->deepCopy();
        // create a vector that will contain path
        vector<string> path;
        PGOL_EvaluateResultList achievable = getAchievableGoals(example, searchExample, goalsToEvaluate, path, depth, 0);

        // select unachievable goals = goals in goalsToEvaluate but not in achievable goals
        // besically all goals that have values NULL
        // also store good goals and 
        // out of achievable goals select potentially good goals, where:
        // a) worst execution of this goal is better than parents worst execution (check this!)
        // b) best execution should improve state's value
        // c) worst execution does not improve state's value (of'course, otherwise it would be a good goal already)
        TGOL_GoalList * pg = mlnew TGOL_GoalList();
        PGOL_GoalList potentiallyGood = pg;
        TGOL_GoalList::iterator g(goalsToEvaluate->begin()), g_end(goalsToEvaluate->end());
        TGOL_EvaluateResultList::iterator r(achievable->begin());

        for (; g!=g_end; g++,r++)
        {
            if ((*r)->achievable == false)
            {
                unachGoals->push_back(*g);
               // printf("unachievable goal: %s\n", (*g)->toString().c_str());
            }
            // this worst realization must be better that that of the parent
            else if ((*g)->parent && (*g)->parent->eval && (*g)->parent->eval->worstEval <= (*r)->worstEval)
                continue;
            else if ((*r)->worstEval < example->evaluate())
            {
                (*r)->goal = *g;
                goodGoals->push_back(*r);
              //  printf("good goal: %s\n", (*g)->toString().c_str());
            }
            else if ((*r)->bestEval < example->evaluate())
            {
                // change best eval to (startEval-bestEval)/len(moves)
                (*r)->bestEval = (startEval-(*r)->bestEval)/(*r)->bestPV->size();
                (*g)->eval = *r;
                potentiallyGood->push_back(*g);
               // printf("potential goal: %s\n", (*g)->toString().c_str());
            }
        }

        // sort potentially good goals according to their best executions
        sort(potentiallyGood->begin(), potentiallyGood->end(), bestExec);

        // take only best K potentially good goals, discard others (they are likely to be bad)
        // for each potentially good goal store its worst realization (needed for further pruning)
        goalsToEvaluate->clear();
        TGOL_GoalList::const_iterator first(potentiallyGood->begin());
        TGOL_GoalList::const_iterator end(potentiallyGood->end());
        int nGoals = 0;
        while (nGoals < K && first != end)
        {
            goalsToEvaluate->push_back(*first);
            first ++;
            nGoals ++;
        }

        // to each potentially good goal, add all possible subgoals and set them as goalsToEvaluate
        goalsToEvaluate = example->expandGoals(goalsToEvaluate, subGoals);
        // do not further specialize, therefore stop
        if (!specialize)
            break;
    }
    // return good goals
   // printf("finished");
    return goodGoals;
}

PGOL_GoalList & mergeGoals(PGOL_GoalList & goals1, const PGOL_GoalList & goals2)
{
    const_PITERATE(TGOL_GoalList, g2, goals2) {
        // if g2 not in goals 1, add it to goals 1
        bool goalIn = false;
        PITERATE(TGOL_GoalList, g1, goals1)
            if (*g1 == *g2)
            {
                goalIn = true;
                break;
            }
        if (!goalIn)
            goals1->push_back(*g2);
    }
    return goals1;
}

PGOL_EvaluateResult TGOL_ANDORSearcher::evaluateGoals(PGOL_State & startExample, PGOL_State & searchExample, const PGOL_GoalList & gl, vector<string> & path, const vector<bool> & holding, const int & maxDepth, int depth, bool storeIDs)
{
    // if cycle, return NULL
    const string id = searchExample->id();
    for (vector<string>::iterator pi = path.begin(); pi != path.end(); pi++)
        if (id.compare(*pi) == 0)
            return NULL;

    // change good enough for this set of goals?
    // there you should also check if the new example is a valid position
    const float pruneProb = searchExample->pruneProb(searchExample, gl, maxDepth, depth);
    if (pruneProb >= 1)
        return NULL;

    vector<bool> newHolding;
    int goalCounter=0;
    for (TGOL_GoalList::const_iterator gi(gl->begin()), ge(gl->end()); gi != ge; gi++, goalCounter++)
    {
        // check also holding goals: if not satisfied, set holding to false
        if (!holding.at(goalCounter))
            newHolding.push_back(false);
        else
            newHolding.push_back((*gi)->checkHolding(startExample, searchExample));
        //if (depth>0 && newHolding.back() && (*gi)->call(startExample, searchExample))
        if (newHolding.back() && (*gi)->call(startExample, searchExample))
        {
            // Goal achieved!
            TGOL_EvaluateResult * er = mlnew TGOL_EvaluateResult();
            PGOL_EvaluateResult wer = er;
            float eval = searchExample->evaluate(); // evaluate is not needed for achievableGoals, is here only for info
            wer->bestEval = eval;
            wer->worstEval = eval;
            TGOL_MoveList * tmp1 = mlnew TGOL_MoveList();
            wer->bestPV = tmp1;
            TGOL_MoveList * tmp2 = mlnew TGOL_MoveList();
            wer->worstPV = tmp2; 
            TGOL_GoalList * tmp3 = mlnew TGOL_GoalList();
            wer->bestGoals = tmp3;
            wer->bestGoals->push_back(*gi);
            TGOL_GoalList * tmp4 = mlnew TGOL_GoalList();
            wer->worstGoals = tmp4;
            wer->worstGoals->push_back(*gi); 
            wer->pruneProb = pruneProb;
            wer->bestGoalIndex = goalCounter;
            wer->worstGoalIndex = goalCounter;
            if (storeIDs)
            {
                TStringList * tsl = mlnew TStringList();
                wer->achievedIDs = tsl;
                wer->achievedIDs->push_back(searchExample->id());
            }
            // number of visited states not relevant in evaluate goals --> therefore omitted
            return wer;
        }
    }

    // depth = maxDepth, end of search
    if (depth == maxDepth)
        return NULL;

    // get moves
    PGOL_MoveList moves = searchExample->getMoves();
    if (!moves)
        return NULL;

    bool orNode = searchExample->orNode();
    vector<PGOL_EvaluateResult> results;
    PITERATE(TGOL_MoveList, mi, moves) {
        searchExample->doMove(*mi);
        path.push_back(id);
        PGOL_EvaluateResult found = evaluateGoals(startExample, searchExample, gl, path, newHolding, maxDepth, depth+1, storeIDs);
        if (found)
        {
            results.push_back(found);
            // add the move to result
            found->bestPV->push_back(*mi);
            found->worstPV->push_back(*mi);
        }
        path.pop_back();
        searchExample->undoMove(*mi);

        // first prune if possible (possible only when we are dealing with an "and" state)
        if (!orNode && !found)
            return NULL;
    }
    // if none of the continuations lead to goal, return NULL
    if (results.size() == 0)
        return NULL;

    PGOL_EvaluateResult result = results.at(0);

    // If And State: 
    // 1) make an union of all bestGoals all worstGoals
    // 2) Always select worst eval (for bestGoals and for worstGoals) and it doesnt matter which goals are being pursued.
    if (!orNode)
    {
        vector<PGOL_EvaluateResult>::const_iterator it = results.begin();
        vector<PGOL_EvaluateResult>::const_iterator end = results.end();
        for (; it != end; it++)
        {
            if ((*it)->worstEval > result->worstEval)
            {
                result->worstEval = (*it)->worstEval;
                result->worstPV = (*it)->worstPV;
                result->worstGoalIndex = (*it)->worstGoalIndex;
            }
            if ((*it)->bestEval > result->bestEval)
            {
                result->bestEval = (*it)->bestEval;
                result->bestPV = (*it)->bestPV;
                result->bestGoalIndex = (*it)->bestGoalIndex;
            }
            // merge goals
            result->bestGoals = mergeGoals(result->bestGoals, (*it)->bestGoals);
            result->worstGoals = mergeGoals(result->worstGoals, (*it)->worstGoals);
        }
    }
    // If Or State:
    // 1) select variant with lowest worst goal index
    // 2) if several possibilities, select the one with the worst worstEval
    else
    {
        vector<PGOL_EvaluateResult>::const_iterator it = results.begin();
        vector<PGOL_EvaluateResult>::const_iterator end = results.end();
        for (; it != end; it++)
        {
            // best goal: 
            // best goal in row or
            // shorted solution path for a goal (if it is the same goal) or
            // best evaluation (if it is the same goal at the same depth)
            if ((*it)->bestGoalIndex < result->bestGoalIndex ||
                (*it)->bestGoalIndex == result->bestGoalIndex && (*it)->bestPV->size() < result->bestPV->size() ||
                (*it)->bestGoalIndex == result->bestGoalIndex && (*it)->bestPV->size() == result->bestPV->size() && (*it)->bestEval < result->bestEval)
            {
                result->bestEval = (*it)->bestEval;
                result->bestPV = (*it)->bestPV;
                result->bestGoalIndex = (*it)->bestGoalIndex;
                result->bestGoals = (*it)->bestGoals;
                // result->bestVisited = (*it)->bestVisited;
            }
            if ((*it)->worstGoalIndex > result->worstGoalIndex ||
                (*it)->worstGoalIndex == result->worstGoalIndex && (*it)->worstEval > result->worstEval)
            {
                result->worstEval = (*it)->worstEval;
                result->worstPV = (*it)->worstPV;
                result->worstGoalIndex = (*it)->worstGoalIndex;
                result->worstGoals = (*it)->worstGoals;
            }
            if (storeIDs)
                const_PITERATE(TStringList, id, (*it)->achievedIDs) {
                    bool containsValue = false;
                    PITERATE(TStringList, rid, result->achievedIDs)
                        if ((*rid).compare(*id) == 0) {
                            containsValue = true;
                            break;
                        }
                    if (!containsValue)
                        result->achievedIDs->push_back(*id);
            }
        }
    }
    result->pruneProb = pruneProb;
    return result; // should create a valid return object
}
