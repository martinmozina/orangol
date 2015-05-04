""" A python library for working with GOL (goal oriented learning) in Orange.

    Author: Martin Mozina
    Ljubljana, 13.11.2012
"""
import random
import collections
import warnings

import Orange
import re
from orngGOL import *
import orangol


def create_data_from_all_positions(state):
    """ Create data from all position
    in the problem space. State representation
    should contain a method *get_all_states()* 
    return all possible states.
    """
    attributes = state.domain.features
    all_states = state.get_all_states()

    # create domain from attributes    
    domain = Orange.data.Domain(state.domain)
    # create empty table
    tab = Orange.data.Table(domain)
    counter = 0
    for s in all_states:
        state.set_state(s)
        example = Orange.data.Instance(domain)
        example["id"] = str(state.id())
        example["eval"] = state.evaluate()
        for ati, at in enumerate(state.domain.features):
            example[ati] = state.get_attribute_value(ati)
        tab.append(example)                
    # return table
    return tab

def create_data_from_start_positions(start_positions, nrandom=0):
    """ Creates an Orange data set. An example is a state on the path
    from start_posititions to the goal nodes (path is found by following
    best moves). For each state it stores attributes, state id and
    its evaluation. It also adds meta attributes to store good goals and unachievable goals. 

    If nrandom is set to a number greater than zero (nrandom>0), the method adds
    nrandom random moves on each best move. 
    """
    
    # create two variables that will be used as class
    # and combine with other variables
    attributes = start_positions[0].domain.features

    # create domain from attributes    
    domain = Orange.data.Domain(start_positions[0].domain)
    domain.add_meta(Orange.core.newmetaid(), GOLVariable("GoodGoal"))
    domain.add_meta(Orange.core.newmetaid(), GOLVariable("Unachievable"))
    domain.add_meta(Orange.core.newmetaid(), Orange.feature.String("traceID"))
    domain.add_meta(Orange.core.newmetaid(), Orange.feature.Continuous("depth"))

    # create empty table
    tab = Orange.data.Table(domain)
    traces = []
    
    counter = 0
    trace = 0
    for ist, start in enumerate(start_positions):
        traces.append([])
        trace += 1
        eval = start.evaluate()
        last_good = start.id()
    ##    input("something")
        while eval > 0:
            if start.or_node():
                example = Orange.data.Instance(domain)
                example["id"] = start.id()
                example["eval"] = eval
                example["traceID"] = str(trace)
                for ati, at in enumerate(start.domain.features):
                    example[ati] = start.get_attribute_value(ati)
                tab.append(example)
                traces[-1].append(start.id())

            if nrandom == 0 or counter % nrandom == 0:
                start.set_state(last_good)
                eval = start.evaluate()
                # get best move
                moves = start.get_moves()
                old_eval = eval
                cands = []
                for m in moves:
                    start.do_move(m)
                    tmp_eval = start.evaluate()
                    if tmp_eval < eval:
                        cands.append((m,tmp_eval))
                    start.undo_move(m)

                # select move
                if start.or_node():
                    mval = min(e for m,e in cands)
                else:
                    mval = max(e for m,e in cands)
                cands = [m for m,e in cands if e == mval]
                move = random.choice(cands)
                start.do_move(move)
                eval = start.evaluate()
                # store last good
                last_good = start.id()
            else:
                move = random.choice(start.get_moves())
                start.do_move(move)
                eval = start.evaluate()
            counter += 1

    # return table
    return tab, traces



def add_goals(state, data, depth, max_sub_goals, K, goals=None, 
              specialize = True, goals_to_dict = False):
    """
    Computes goals for positions given in an existing Orange data set.

    If goals is given, only these goals will be specialized.
    Set specialize to False if specialization is not needed.
    If dict_goals is true, goal evaluations are stored in a 
    dictionary (instead of writing them in the Orange data set.) 
    When dict_goals is true then goals should not be None.
    """
    
    search = orangol.GOL_ANDORSearcher()
    if goals_to_dict:
        goals_dict = collections.defaultdict(list)
    for instance in data:
        print str(instance["id"])
        state.set_state(str(instance["id"]))
        print "before"
        if goals:
            best, unach = search.assignGoals(state, depth, max_sub_goals, 
                                             K, goals, specialize)
        else:
            best, unach = search.assignGoals(state, depth, max_sub_goals, K)
        print "after"
        best_goals = [b.goal for b in best]
        if not goals_to_dict:
            instance["GoodGoal"] = [(g.goal, g.visited) for g in best] 
            instance["Unachievable"] = [(g, -1) for g in unach] 
        else:
            for g in goals:
                if g in best_goals:
                    goals_dict[str(g)].append(1)
                elif g in unach:
                    goals_dict[str(g)].append(0)
                else:
                    goals_dict[str(g)].append(-1)
    print "done"
    if goals_to_dict:
        return data, goals_dict
    return data

def read_rule_line(state, line):
    tmp = GOLVariable("")
    ex = Orange.data.Instance(state.domain)
    
    conditions, goal = line.strip().split("-->")
    goals_ = tmp.filestr2val(goal, example=ex)
    goals = orangol.GOL_GoalList()
    for g, c in goals_:
        goals.append(g)
    
    rule = Orange.core.Rule()
    rule.filter = Orange.data.filter.Values(domain = state.domain)
    # add conditions
    for cond in conditions.split(","):
        if cond:
            params = re.search(r"\((.*)\)", cond).groups()[0].strip().split("|")
            position = int(params[0])
            if isinstance(state.domain[position], Orange.feature.Discrete):
                if int(params[1]) == 1:
                    negate = False
                else:
                    negate = True
                values = [Orange.data.Value(state.domain[int(params[0])], int(p)) 
                          for p in params[2:]]
                rule.filter.conditions.append(
                    Orange.data.filter.ValueFilterDiscrete(
                    position=position,
                    values=values,
                    negate=negate))                    
            elif isinstance(state.domain[position], Orange.feature.Continuous):
                oper = int(params[1])
                ref = float(params[2])
                rule.filter.conditions.append(
                    Orange.data.filter.ValueFilterContinuous(
                    position=position,
                    oper=oper,
                    ref=ref))                    
                
    # set goal
    rule.setattr("goals", goals)
    return rule      

def rule_to_str(rule):
    # convert conds
    conds = []
    for c in rule.filter.conditions:
        # get position
        pos = c.position
        if isinstance(rule.filter.domain[pos], Orange.feature.Discrete):
            neg = int(c.negate)
            vals = [str(int(v)) for v in c.values]
            conds.append("(%d|%d|%s)"%(pos,neg,"|".join(vals)))
        else:
            oper = c.oper
            ref = c.ref
            conds.append("(%d|%d|%f)"%(pos,oper,ref))
            
    # convert goals
    subgoals = str(rule.goals).strip().split(",")
    goals = [re.search("\[.*\]", s).group() for s in subgoals] 

    return ",".join(conds)+"-->"+",".join(goals)
         

def evaluate(state, moves, goals, depth, storeIDs):
    """ Evaluates wheter a goal is achievable. """
    search = orangol.GOL_ANDORSearcher()
    results = search.evaluateGoals(state, moves, goals, depth, storeIDs)
    return results

def evaluate_set(state, moves, goals, depth):
    """ Evaluates wheter a set of goals are achievable. """
    search = orangol.GOL_ANDORSearcher()
    results = search.evaluateGoalsSet(state, moves, goals, depth)
    return results

    
    
    
        
        
