""" The implementation of the means-ends gol rule learning algorithm. 

The main algorithm is implemented in the GOLRuleLearner class, for documentation see  ...
"""

import random
import math
import Orange
import gol
import collections
import rulelearner
import goaltree
import orangol

EPS = 0.001

operators = [orangol.GOL_SubGoal.NoChange,
             orangol.GOL_SubGoal.Increase,
             orangol.GOL_SubGoal.Decrease,
             orangol.GOL_SubGoal.NonIncrease,
             orangol.GOL_SubGoal.NonDecrease,
             orangol.GOL_SubGoal.Equals,
             orangol.GOL_SubGoal.GreaterThan,
             orangol.GOL_SubGoal.LessThan,
             orangol.GOL_SubGoal.External]

oper_map = {Orange.data.filter.ValueFilter.Equal:orangol.GOL_SubGoal.Equals,
            Orange.data.filter.ValueFilter.Less:orangol.GOL_SubGoal.LessThan,
            Orange.data.filter.ValueFilter.LessEqual:orangol.GOL_SubGoal.LessThan,
            Orange.data.filter.ValueFilter.Greater:orangol.GOL_SubGoal.GreaterThan,
            Orange.data.filter.ValueFilter.GreaterEqual:orangol.GOL_SubGoal.GreaterThan }

class PrepareGoals(object):
    """ A class for manipulating goals, like creating goals from rules' conditions and similar. """
    def __init__(self, rule):
        self.rule = rule
        
    def add_monotone_subgoal(self, c, goal):
        # if conditions is less or less equal, goal is Decrease
        if c.oper == Orange.data.filter.ValueFilter.LessEqual or \
           c.oper == Orange.data.filter.ValueFilter.Less:
            oper_goal = orangol.GOL_SubGoal.Decrease
        else:
            oper_goal = orangol.GOL_SubGoal.Increase
        sub_goal =  orangol.GOL_SubGoal(c.position, oper_goal, 0, self.rule.filter.domain)
        goal.subGoals.append(sub_goal)

    def add_constraining_goals(self, actives, goal):
        # create holding goals and required conditions of rules learning from this example
        for c_holding in self.rule.filter.conditions:
            if actives and c_holding in actives:
                continue
            if isinstance(c_holding, Orange.data.filter.ValueFilterDiscrete): # TODO: doesnt work
                sub_goal = orangol.GOL_SubGoal(c_holding.position, orangol.GOL_SubGoal.Equals, 0, self.rule.filter.domain, c_holding.values)
            else:
                # determine operator
                oper = oper_map[c_holding.oper]
                # and value
                val = float(c_holding.ref)
                if c_holding.oper == Orange.data.filter.ValueFilter.LessEqual:
                    val += EPS
                if c_holding.oper == Orange.data.filter.ValueFilter.GreaterEqual:
                    val -= EPS
                vals = Orange.core.ValueList()
                vals.append(Orange.core.Value(self.rule.filter.domain.attributes[c_holding.position], float(val)))
                sub_goal =  orangol.GOL_SubGoal(c_holding.position, oper, 0, self.rule.filter.domain, vals)
            goal.subGoals.append(sub_goal)
            
    def get_holding_conditions(self, actives, goal):
        return [c_holding for c_holding in self.rule.filter.conditions if c_holding not in actives]            

            
    def prepare_possible_goals(self, actives):
        """ Takes rule's conditions and creates possible goals that 
        solve the active goal: one progressive and one normal. 
        """
        goals_with_conds = []
        # first, make a list of goals with decrease / increase
    
        # create a standard goal
        goal = orangol.GOL_Goal()
        goal.subGoals = orangol.GOL_SubGoalList()
        
        self.add_constraining_goals(None, goal)
        conditions = self.get_holding_conditions(actives, goal)
        unfin_conds = []
        goals_with_conds.append((goal, goal, conditions, unfin_conds))
    #===========================================================================
    #    if isinstance(active, Orange.data.filter.ValueFilterContinuous):
    #        # first create decrease / increase goals
    #        mgoal = orangol.GOL_Goal()
    #        mgoal.subGoals = orangol.GOL_SubGoalList()
    # 
    #        self.add_monotone_subgoal(active, mgoal)
    #        self.add_constraining_goals(active, mgoal)
    #        conditions = self.get_holding_conditions(active, mgoal)
    #        unfin_conds = [active]
    #        goals_with_conds.append((mgoal, goal, conditions, unfin_conds))
    #===========================================================================
        return goals_with_conds
    
class TheoryConceptualizer(object):
    """ A class that accepts a state space representing a domain of problems, 
    where each node is described with attributes. The implemented method conceptualizes
    a theory (a recipe) for solving problems in this domain. 
    
    Settings:
    * search_depth: maximum search depth in solving goals
    * active: using active learning? 
    * cache_goals: should solutions of goals be cached? caching = faster, 
                   however using more memory
    * att_alpha: significance of each condition in rule learning
    * min_coverage: minimum number of examples that a rule must cover
    * max_conditions: maximal conditions in a rule
    * max_rules: number of rules that a rule learner can learn 
                 (actually it is the number of alternative branches 
                 in the goal-oriented tree)
    * min_quality: minimal quality of learned rules
    * only_positive: should learn only positive conditions 
                     ("yes" for discrete and < (less) for continuous)
    * only_positive_implicit: should implicit conditions be only positive?
    * only_pure: learn only rules with "pure" class distribution 
                 (covering only positive class)
    * max_branches:  maximum number of branches alltogether; 
                     it is actually the number of alternative strategies
    * add_implicit_conditions: should implicit conditions be added at 
                               the end of rule learning?
    * min_traces: minimal number of covered traces
    * min_seed_traces: minimal number of covered seed traces
    * negative_step: number of negative examples added in each active step
    * cov_weight: weight given to coverage in rule learning
    """
    
    def __init__(self, search_depth = 5, active=True, cache_goals=True, m=2, 
                 att_alpha=1.0, min_coverage=1, max_conditions = 10, max_rules=3, 
                 min_quality = 0.0, only_positive=False, only_positive_implicit=False, 
                 only_pure=False, max_branches=5, add_implicit_conditions = False, 
                 min_traces = 0, min_seed_traces=0, negative_step = 20):
        self.search_depth = search_depth
        self.cache_goals = cache_goals
        self.cached_goals = {}
        self.active = active
        self.max_rules = max_rules
        self.max_branches = max_branches
        self.min_traces = min_traces
        self.add_implicit_conditions = add_implicit_conditions
        self.negative_step = negative_step
        self.rule_learner = rulelearner.GOLRuleLearner(
            m=m, min_coverage=min_coverage, max_conditions=max_conditions, 
            max_rules=max_rules, only_positive=only_positive, 
            only_positive_implicit = only_positive_implicit, 
            only_pure=only_pure, att_alpha = att_alpha, min_quality = min_quality, 
            min_traces = min_traces, min_seed_traces = min_seed_traces)
        
    def __call__(self, data, state):
        print "LEARNING PLANS, len(data)=", len(data)
        
        # assign initial goal = finish
        goal = orangol.GOL_Goal()
        goal.subGoals = orangol.GOL_SubGoalList()
        vals = Orange.core.ValueList()
        vals.append(Orange.core.Value(data.domain.attributes[-1], "yes"))
        goal.subGoals.append(orangol.GOL_SubGoal(len(data.domain.attributes)-1, 
                                                     orangol.GOL_SubGoal.Equals, 0, data.domain, vals))
        initial_goals = orangol.GOL_GoalList()
        initial_goals.append(goal)
        
        # prepare data with class
        cls = Orange.feature.Discrete("Goal", values=["yes","no"])
        dom = Orange.data.Domain(data.domain.attributes, cls)
        dom.addmetas(data.domain.getmetas())
        # depth
        depth = Orange.feature.Continuous("depth")
        newid = Orange.feature.Descriptor.new_meta_id()
        dom.addmeta(newid, depth)

        solved = Orange.feature.Continuous("solved")
        newid = Orange.feature.Descriptor.new_meta_id()
        dom.addmeta(newid, solved)
        
        data_with_class = Orange.data.Table(dom, data)
        # updating data: assigning correct class values
        data_with_class = self.update_rule_examples(
                                state, data_with_class, initial_goals)
        Orange.core.saveTabDelimited("data_prolog2.tab", data_with_class)
        
        # create a goal tree with the initial goal only
        self.goals = goaltree.GoalTree(None, None, initial_goals, 
                                       data_with_class, self.search_depth)
        
        selected = self.goals.get_not_expanded(data_with_class)
        while selected is not None:
            self.expand_goal(selected, state, data_with_class)
            selected.free_memory()
            selected = self.goals.get_not_expanded(data_with_class)
            count = self.goals.count_candidates()
            if count > self.max_branches:
                self.max_rules = 1
            print self.goals.print_plan("")
        #self.goals.prune_final_tree(data_with_class)
        return self.goals
    
    def expand_goal(self, goal_node, state, full_data_with_class):
        self.cached_goals = {}
        print "class dist: ", goal_node.distribution

        # start by learning rules
        print "learning rules"
        if self.active:
            rules = self.active_rule_learner(goal_node, state)
            #if self.add_implicit_conditions:
            #    rules = self.rule_learner.add_conditions_method(rules, 0, 0)
        else:
            rules = self.rule_learner(goal_node)
            if self.add_implicit_conditions:
                rules = self.rule_learner.add_conditions_method(rules, 0, 0)
        print "finished with learning rules"
        if not rules or rules[0].classDistribution.abs == len(goal_node.data):
            print "Found a final node"
            goal_node.final_node = True
            return

        # analyze rules, take only best self.max_rules 
        added = False
        for r in rules:
            if r.quality <= 0.0:
                print "quality too low"
                continue
            if len(r.filter.conditions) == 0:
                print "conditions equals zero"
                continue
            r, gls, data_right, data_right_distribution = self.evaluate_rule(r, goal_node.data, state)
            if not r:
                print "not r out of evaluate rule"
                continue
            new_plan = goaltree.GoalTree(goal_node, r, gls, 
                                         data_right, self.search_depth)
            
            goal_node.children.append(new_plan)
            added = True

        if not added: #added == 0:
            print "Found a final node"
            goal_node.final_node = True
            return    
            
    def evaluate_goal(self, state, goals, depth = None, storeIDs = False):
        if not depth:
            depth = self.search_depth
        """ Evaluating includes caching to prevent re-evaluation of goals. """
        moves = orangol.GOL_MoveList()
        if self.cache_goals:    
            key = (state.id(), str(goals), depth, storeIDs)
            if key not in self.cached_goals:
                self.cached_goals[key] = gol.evaluate(state, moves, 
                                                      goals, depth, storeIDs)
            return self.cached_goals[key]
        return gol.evaluate(state, moves, goals, depth, storeIDs)

    def update_rule_examples(self, state, data, goals):
        dom = data.domain
        new_data = Orange.data.Table(dom)
        ach_len = 0
        ach_set = set()
        print "goals: ", list(g for g in goals)
        for e in data:
            #print "dealing with data:", str(e["id"])
            state.set_state(str(e["id"]))
            # evaluate goal
            evaluation = self.evaluate_goal(state, goals, storeIDs = True)
            if evaluation:
                # add this example to new_data as "yes"
                new_example = self.prepare_example(state, dom)
                new_example[new_example.domain.classVar] = "yes"
                new_example["depth"] = len(evaluation.worstPV)
                new_data.append(new_example)
                #ach_set.add(str(evaluation.achievedIDs))
                #ach_len += 1
                #new_example["achieved"] = str(evaluation.achievedIDs)
            else:
                # add this example to new_data as "no"
                new_example = self.prepare_example(state, dom)
                new_example[new_example.domain.classVar] = "no"
                new_example["depth"] = 1
                new_data.append(new_example)
        #print "ach_stat", ach_len, len(ach_set)
        return new_data

    def push_one_step(self, goal_node, rule, rule_goals, state, original_data):
        """ This method is a part of active learning; 
        it finds all examples close to examples in rule. 
        it will fill up a rule with a certain percentage of its examples (50%). """
        left_goals = goal_node.goals
        generated = []
        new_data = []
        dom = rule.examples.domain
        visited = set()
        rule_cond = []
        negatives, added = 0, 0
        for c in rule.filter.conditions:
            nr = rule.clone()
            nr.filter.conditions = [c]
            rule_cond.append(nr)
        for e in rule.examples:
            visited.add(str(e["id"]))
            generated.append((0, str(e["id"])))
        # generate examples from following example
        # 1. prepare goals
        # 2. check from which examples can we achieve these goals
        for e in original_data:
            state.set_state(str(e["id"]))
            # evaluate goal
            evaluation = self.evaluate_goal(state, rule_goals, storeIDs = True)
            if evaluation:
                for id in evaluation.achievedIDs:
                    if id in visited:
                        continue
                    visited.add(id)
                    generated.append((0, id))
                    # prepare and evaluate example                   
                    state.set_state(id)
                    ex = self.prepare_example(state, dom)
                    ev2 = self.evaluate_goal(state, left_goals, self.search_depth)
                    # add example to new_data
                    added += 1
                    if not ev2:
                        ex[ex.domain.classVar] = "no"
                        ex["depth"] = 1
                        new_data.append(("?", ex))
                        negatives += 1
                    else:
                        ex[ex.domain.classVar] = "yes"
                        ex["depth"] = 1
                        new_data.append(("?", ex))     
            #if negatives > self.negative_step:
            #    break               

        expanded = 0
        while expanded < 1000 and negatives < self.negative_step: 
            # select best candidate
            cand = generated[0][1]
            generated = generated[1:]
            expanded += 1
            
            # prepare neighbours
            state.set_state(cand)
            moves = state.get_moves()
            for m in moves:
                # add neighbours if they are not in visited; 
                state.do_move(m)
                neigh = state.state
                if neigh not in visited:
                    visited.add(neigh)
                    ex = self.prepare_example(state, dom)
                    # compute heuristics
                    h = sum(not rc(ex) for rc in rule_cond)
                    generated.append((h, neigh))
                    if h == 0: # rule covers example
                        ev2 = self.evaluate_goal(state, left_goals, self.search_depth)
                        added += 1
                        if not ev2:
                            ex[ex.domain.classVar] = "no"
                            ex["depth"] = 1
                            new_data.append(("?", ex))
                            negatives += 1
                        else:
                            ex[ex.domain.classVar] = "yes"
                            ex["depth"] = 1
                            new_data.append(("?", ex))
                state.undo_move(m)
            # sort new generated
            generated.sort()
        return new_data
    
    def prepare_example(self, state, dom):
        """ Creates an Orange example from this state. """
        new_example = state.create_example()
        new_example = Orange.data.Instance(dom, new_example)
        return new_example
    
    def active_rule_learner(self, goal_node, state):
        """ Learn a set of goal oriented rules using active learning. """
        data_set = set(str(d["id"]) for d in goal_node.data) # will also added examples
        good_rules = []
        seen_rules = collections.defaultdict(int)
        rule_pos = collections.defaultdict(float)
        rule_neg = collections.defaultdict(float)
        while True:
            # learn rules
            rules = self.rule_learner(goal_node, n_rules = 1, old_rules = good_rules)
            print "*" * 10, "temp rules: ", "*" * 10
            for r in rules:
                print Orange.classification.rules.rule_to_string(r)
            # iterate trough learned rules and find examples to add
            examples_to_add = Orange.data.Table(goal_node.data.domain)
            for r in rules:
                if len(r.filter.conditions) == 0:
                    continue
                rule_repr = rulelearner.uni_repr(r)
                # if this rules was seen N(5) times, it should be a good rule 
                seen_rules[rule_repr] += 1
                # prepare goals from rule
                g, a, c1, c2 = PrepareGoals(r).prepare_possible_goals(r.filter.conditions[:])[0]
                gls = orangol.GOL_GoalList()
                gls.append(g)
                # the active learning step: get new positive and negative examples
                new_data = self.push_one_step(goal_node, r, gls, state, goal_node.original_data)
                added_negative = 0.
                added_positive = 0.
                for start_node, example in new_data:
                    if str(example["id"]) not in data_set:
                        names = [example.domain[m].name for m in example.domain.get_metas()]
                        if "trace" in names:                        
                            example["trace"] = ""
                        examples_to_add.append(example)
                        data_set.add(str(example["id"]))
                        added_negative += str(example.get_class()) == "no"
                        added_positive += str(example.get_class()) == "yes"
                rule_pos[rule_repr] += added_positive
                rule_neg[rule_repr] += added_negative
                print "added negative", added_negative
                # prevent too many new examples
                if added_negative == 0: 
                    good_rules.append(r)
                elif r.classDistribution[1] > 200 and seen_rules[rule_repr] >= 5:
                    good_rules.append(r)
                    r.quality = min(r.quality, rule_pos[rule_repr] / (rule_pos[rule_repr] + rule_neg[rule_repr])) 
                    
            # if an empty rule was learned                
            if not rules or len(rules[0].filter.conditions) == 0:
                goal_node.children = []
                if self.add_implicit_conditions:
                    good_rules = self.rule_learner.add_conditions_method(good_rules, 0, 0) 
                for r in good_rules: # remove all examples added through active learning
                    r.filterAndStore(goal_node.original_data, 0, 0)
                # restore data in goal_node
                goal_node.data = goal_node.original_data
                goal_node.distribution = goal_node.original_distribution
                return good_rules
            # add new examples to learning data
            goal_node.add_examples(examples_to_add)
    
    def evaluate_rule(self, rule, data_with_class, state):
        if len(rule.filter.conditions) == 0:
            return None, None, None, None
        # prepare goals from rule
        pos_goals = PrepareGoals(rule).prepare_possible_goals(rule.filter.conditions[:])
        g, a, c1, c2 = pos_goals[0]          
        gls = orangol.GOL_GoalList()
        gls.append(g)
        print "testing rule: ", Orange.classification.rules.rule_to_string(rule)
        
        # select examples that are not covered by rule (tmp_c): these 
        # examples will be used to learn the remaining of the goal tree
        # important: do not add more of particular instances that were in the original data set
        #cont_counter = collections.Counter()
        #data_right = Orange.data.Table(data_with_class.domain)
        #for d in data_with_class:
        #    if not rule(d): # and str(d.getclass()) == "no":
        #        data_right.append(d)
        #print "data right len == ", len(data_right)
        # compute classes in data_right
        data_right = self.update_rule_examples(state, data_with_class, gls) 
        data_right_distribution = Orange.statistics.distribution.Distribution(
                                    data_right.domain.classVar, data_right)
        return rule, gls, data_right, data_right_distribution       
        
    def determine_p_value(self, data, state, p=0.5):
        """ Implements a randomization technique that determines LRS-value given p for a single attribute. 
        From returned LRS you can compute alpha and set that alpha to validators in learning. """
        
        # compute data (goal = initial goal)
        # average distribution of learning is determined by the distribution of achievability of the initial goal 
        goal = orangol.GOL_Goal()
        goal.subGoals = orangol.GOL_SubGoalList()
        vals = Orange.core.ValueList()
        vals.append(Orange.core.Value(data.domain.attributes[-1], "yes"))
        goal.subGoals.append(orangol.GOL_SubGoal(len(data.domain.attributes)-1, 
                                                     orangol.GOL_SubGoal.Equals, 
                                                     0, data.domain, vals))
        initial_goals = orangol.GOL_GoalList()
        initial_goals.append(goal)
        # prepare data with class
        cls = Orange.feature.Discrete("Goal", values=["yes","no"])
        dom = Orange.data.Domain(data.domain.attributes, cls)
        dom.addmetas(data.domain.getmetas())
        data_with_class = Orange.data.Table(dom, data)
                
        # updating data: assigning correct class values
        data_with_class = self.update_rule_examples(state, data_with_class, initial_goals)
        
        print "dist", Orange.statistics.distribution.Distribution(
                        data_with_class.domain.classVar, data_with_class)
        
        # set rule's settings (at the end of the method, these values are reset)
        mc = self.rule_learner.rule_finder.validator.max_conditions
        at_sig = self.rule_learner.rule_finder.validator.att_val.alpha
        mq = self.rule_learner.rule_finder.validator.min_quality
        self.rule_learner.rule_finder.validator.max_conditions = 1
        self.rule_learner.rule_finder.rule_stoppingValidator.max_conditions = 1
        self.rule_learner.rule_finder.validator.att_val.alpha = 1.0
        self.rule_learner.rule_finder.rule_stoppingValidator.att_val.alpha = 1.0
        self.rule_learner.rule_finder.validator.min_quality = 0
        self.rule_learner.rule_finder.rule_stoppingValidator.min_quality = 0
        lrs_evaluator = Orange.core.RuleEvaluator_LRS()
        
        # the actual algorithm:
        # runs 100 times, randomizes class, computes lrs' and adds them to a list
        lrs_values = []
        for i in range(100):
            # permute classes in data_with_class
            newData = Orange.data.Table(data_with_class)
            cl_num = newData.toNumpy("C")
            random.shuffle(cl_num[0][:, 0])
            clData = Orange.data.Table(Orange.data.Domain([newData.domain.classVar]), 
                                       cl_num[0])
            for d_i, d in enumerate(newData):
                d[newData.domain.classVar] = clData[d_i][newData.domain.classVar]
                    
            # learn rules
            rules = self.rule_learner(newData, full_distr = False)
            print "Learned rules: "
                
            # compute LRS statistics
            if len(rules) <= 1: # only if TRUE was learned
                lrs_values.append(0)
            else:
                for r in rules:
                    if len(r.filter.conditions) == 0:
                        continue
                    lrs_value = lrs_evaluator(r, r.parentRule.examples, 0, 
                                              0, r.parentRule.classDistribution)
                    # add to list of lrs values
                    lrs_values.append(lrs_value)
            lrs_values.sort(reverse = True)
            print "Finished iteration {}, current lrs value = {}".format(i, lrs_values[int(p * len(lrs_values))])
            
        # reset rule's settings
        self.rule_learner.rule_finder.validator.max_conditions = mc
        self.rule_learner.rule_finder.rule_stoppingValidator.max_conditions = mc
        self.rule_learner.rule_finder.validator.att_val.alpha = at_sig
        self.rule_learner.rule_finder.rule_stoppingValidator.att_val.alpha = at_sig
        self.rule_learner.rule_finder.validator.min_quality = mq
        self.rule_learner.rule_finder.rule_stoppingValidator.min_quality = mq
            
        # return the appropriate lrs value (given p)
        lrs_values.sort(reverse = True)
        return lrs_values[int(p * len(lrs_values))]
        

        
        
        

        
        
