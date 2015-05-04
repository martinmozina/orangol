""" class GoalTree implements a tree-based representation 
of goal hierarchy used in goal-oriented learning. 
"""

import random
import Orange
import gol

def get_indent(s):
    position = len(s) - len(s.lstrip("\t"))
    return position, s[position:]

def create_goal_tree_from_file(file_handler, search_depth, state):
    """ Reads and creates a goal-tree  from file. """
    root = GoalTree(None, None, None, None, search_depth)
    node = root
    position = 0
    for line in file_handler:
        indent, rule_line = get_indent(line)
        rule_line = rule_line.strip()
        rule_line, quality, dist = rule_line.split(";")
        rule = gol.read_rule_line(state, rule_line)
        rule.classDistribution = eval(dist)
        rule.quality = float(quality)
        goals = rule.goals
        if goals == None:
            print "none goals: ", Orange.classification.rules.rule_to_string(rule)
        
        if indent > position:
            print "error, should never happen"
        while indent < position:
            node = node.parent_plan
            position -= 1

        new_node = GoalTree(node, rule, None, None, search_depth)
        if len(rule.filter.conditions) == 0:
            node.classDistribution = eval(dist)
        node.children.append(new_node)
        node.goals = goals
        node = new_node
        position += 1
    return root

class GoalTree(object):
    def __init__(self, parent_plan, parent_rule, goals, data, search_depth):
        """ A constructor for a goal-tree node. 
        
        Every parent rule should have two attributes:
        * covered_traces: are all traces covered by this rule
        * seed_traces: are traces specific only to this rule
        
        We store covered_traces and seed_traces (copy from parent_rule), 
        but then, we also compute attribute *to_solve*, which are examples 
        that are achievable (positive = "yes") and at the same time
        were not solved by its parents already.
        """
        
        self.parent_plan = parent_plan # a pointer to the parent node in the goal tree
        self.parent_rule = parent_rule # a pointer to the rule from the parent node

        # are traces used in learning?
        names = [data.domain[m].name for m in data.domain.get_metas()]
        if "trace" in names:
            learn_trace = True
        else:
            learn_trace = False
        
        # compute active traces
        self.covered_traces = set()
        self.seed_traces = set()
        if self.parent_rule:
            if hasattr(self.parent_rule, "covered_traces"):
                self.covered_traces = self.parent_rule.covered_traces
                self.seed_traces = self.parent_rule.seed_traces
        elif data and learn_trace:
            for e in data:
                if str(e.getclass()) == "yes":
                    traces = set(str(e["trace"]).strip().split(";"))
                    self.covered_traces |= traces
                    self.seed_traces |= traces
        
        if data:
            # get all positive examples that were not solved already by parents.
            # 1. get all examples solved by parents (all examples where 
            #    goal is achievable or covered by a rule)
            pp = self.parent_plan
            # 2. get all positive examples in this data that are not 
            #    solved by parents already
            self.to_solve = Orange.data.Table(data.domain)
            to_remove = set()
            self.already_solved = set(pp.already_solved) if pp else set()
            if parent_rule:
                self.already_solved |= set(str(d["id"]) 
                                           for d in parent_rule.examples)
            print "already solved: ", self.already_solved
            print "seed traces: ", self.seed_traces 
            for d in data:
                if (str(d.getclass()) == "yes" and learn_trace and
                    str(d["id"]) not in self.already_solved):
                    # a candidate!
                    traces = set(str(d["trace"]).strip().split(";"))
                    if traces & self.seed_traces:
                        self.to_solve.append(d)
                elif (str(d.getclass()) == "yes" and not learn_trace and
                    str(d["id"]) not in self.already_solved):
                    # a candidate!
                    self.to_solve.append(d)
                
                if (str(d.getclass()) == "no" and 
                    str(d["id"]) in self.already_solved):
                    # examples to be removed from self.data
                    to_remove.add(str(d["id"]))
            #self.already_solved |= set(str(d["id"]) 
            #                           for d in data 
            #                           if str(d.getclass()) == "yes")
            # prepare learning data         
            self.data = Orange.data.Table(data.domain)
            for d in data:
                if str(d["id"]) not in to_remove:
                    self.data.append(d)
            self.distribution = Orange.statistics.distribution.Distribution(
                                                    self.data.domain.classVar, 
                                                    self.data)
        else:
            self.data = None
            self.distribution = None
            
        self.original_data = self.data
        self.original_distribution = self.distribution
        self.goals = goals
        # children plans: a list of nodes
        self.children = []
        # final node : a leaf node (without children) and can not be further expanded
        self.final_node = False
        self.search_depth = search_depth
        # compute active traces covered by this node
                
    def copy(self):
        cpy = GoalTree(self.parent_plan, self.parent_rule, 
                       self.goals, self.data, self.search_depth)
        cpy.children = self.children
        cpy.final_node = self.final_node
        cpy.covered_traces = self.covered_traces
        cpy.seed_traces = self.seed_traces
        return cpy
        
    def add_examples(self, new_examples):
        self.data = Orange.data.Table(self.data, False)
        for e in new_examples:
            self.data.append(e)
        self.distribution = Orange.statistics.distribution.Distribution(
                                   self.data.domain.classVar, self.data)
        
    def free_memory(self):
        self.data = None
        self.original_data = None
        if self.parent_rule:
            self.parent_rule.examples = None
        
    def save_to_file(self, file_handler, indent = ""):
        # first try to save only the first rule
        if not self.children and self.final_node:
            file_handler.write(indent + "-->"+ gol.goal_to_str(self.goals) + 
                               ";0;" + str(list(self.distribution)) + "\n")
        if self.children:
            for plan in self.children:
                r = plan.parent_rule
                r.setattr("goals", self.goals)
                file_handler.write(indent + gol.rule_to_str(r) + ";" + 
                                   str(r.quality) + ";" + 
                                   str(list(r.classDistribution)) + "\n")
                plan.save_to_file(file_handler, indent+"\t") 
    
    def get_not_expanded(self, orig_data):
        """ find first node (breadth first search) that 
        is not final and has no children. 
        """
        toexplore = [self]
        for te in toexplore:
            if not te.final_node and not te.children:
                return te
            toexplore.extend(te.children)
        return None
    
    def count_candidates(self):
        """ Counts number of branches. """
        ncands = 0
        toexplore = [self]
        for te in toexplore:
            if not te.final_node and not te.children:
                ncands += 1
            toexplore.extend(te.children)
        return ncands

    def print_plan(self, indent=""):
        if not self.children and self.final_node:
            print indent + "IF TRUE THEN " + str(self.goals) + ", quality = " + \
                  str(self.distribution)  + ", seed traces = " + str(self.seed_traces)
            return
        for c in self.children:
            print indent + Orange.classification.rules.rule_to_string(c.parent_rule)+ \
                  ", quality = " + str(c.parent_rule.quality) + ";" + str(self.goals) + \
                  ", seed traces = " + str(self.seed_traces)
            if c:
                c.print_plan(indent + "  ")
        
    def evaluate(self):
        """ Evaluation of a branch: now it simply takes the minimimu accuracy of rules in a branch. """
        if not self.parent_rule:
            return 1.0
        return min(self.parent_rule.quality, self.parent_plan.evaluate())
        
    def get_best_goal(self, state, depth):
        """ Find best goal according to evaluation function. Return goal and evaluation of goal. """
        
        # check if goal is achievable? - return it
        moves = orangol.GOL_MoveList()
        # first check if goals are already achieved
        if not self.goals or gol.evaluate(state, moves, self.goals, 0, False):
            return None, (None, 0), 0
        
        # can goals be achieved?
        evaluation = gol.evaluate(state, moves, self.goals, depth, True)
        if evaluation:
            # new state is?
            new_state = state.deep_copy()
            new_state.state = random.choice(evaluation.achievedIDs)
            #for move in reversed(evaluation.worstPV):
            #    new_state.do_move(move)
            return self, (new_state, len(evaluation.worstPV)), self.evaluate()
        
        # otherwise search through children
        best_goal, best_state, best_eval = None, (None, 0), 0
        for plan in self.children:
            goal, new_state, ev = plan.get_best_goal(state, depth)
            if goal and ev > best_eval:
                best_eval = ev
                best_goal = goal
                best_state = new_state
        return best_goal, best_state, best_eval

    def count_relevant(self, best_plan, relevant_branches, tuple_repr, active):
        act = active
        if self == best_plan:
            act = True

        if self.children == []:
            if act:
                # store relevant branches
                relevant_branches[tuple_repr] = 1
            return
        
        for ci, c in enumerate(self.children):
            c.count_relevant(best_plan, relevant_branches, tuple_repr + (ci,), act)
    
    def get_best_branch(self, state, depth):
        """ Selection of best branch. """
        # first find best subplan
        best_plan, ns, ev = self.get_best_goal(state, depth)
        if not best_plan:
            return None, {}
        relevant_branches = {}
        self.count_relevant(best_plan, relevant_branches, tuple(), False)
        print best_plan
        # create branch from this subplan
        new_plan = best_plan.copy()
        new_plan.children = []
        new_plan.final_node = True
        
        while new_plan.parent_plan:
            child_plan = new_plan
            new_plan = new_plan.parent_plan.copy()
            new_plan.children = [child_plan]
        return new_plan, relevant_branches
    
    def select_branch_by_indices(self, indices):
        """ Find the selected (using indices) branch. """
        bp = self
        for i in indices:
            bp = bp.children[i]
        
        np = bp.copy()
        np.final_node = True
        
        while np.parent_plan:
            child_plan = np
            np = np.parent_plan.copy()
            np.children = [child_plan]
        return np
