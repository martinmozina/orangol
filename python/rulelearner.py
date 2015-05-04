""" A rule learner for goal-oriented rule learning. """

import warnings
import Orange

RuleList = Orange.core.RuleList
RuleLearner = Orange.core.RuleLearner
BeamFinder = Orange.core.RuleBeamFinder
BeamFilter_Width = Orange.core.RuleBeamFilter_Width
Validator_LRS = Orange.core.RuleValidator_LRS
DataStoppingCriteria_NoPositives = Orange.core.RuleDataStoppingCriteria_NoPositives

def select_sign(oper):
    if oper == Orange.data.filter.ValueFilter.Less:
        return "<"
    elif oper == Orange.data.filter.ValueFilter.LessEqual:
        return "<="
    elif oper == Orange.data.filter.ValueFilter.Greater:
        return ">"
    elif oper == Orange.data.filter.ValueFilter.GreaterEqual:
        return ">="
    else: return "="
        
def uni_repr(rule):
    domain = rule.filter.domain
    r = set()
    for c in rule.filter.conditions:
        if isinstance(c, Orange.data.filter.ValueFilterDiscrete):
            r.add(domain[c.position].name + "=" + 
                  str([domain[c.position].values[int(v)] 
                       for v in sorted(c.values)]))
        elif isinstance(c, Orange.data.filter.ValueFilterContinuous):
            r.add(domain[c.position].name + select_sign(c.oper) + str(c.ref))  
    return frozenset(r)      

def rules_equal(rule1, rule2):
    if len(rule1.filter.conditions) != len(rule2.filter.conditions):
        return False
    for c1 in rule1.filter.conditions:
        found = False # find the same condition in the other rule
        for c2 in rule2.filter.conditions:
            try:
                if not c1.position == c2.position: continue # same feature?
                if not type(c1) == type(c2): continue # same type of condition
                if type(c1) == Orange.core.ValueFilter_discrete:
                    if not type(c1.values[0]) == type(c2.values[0]): continue
                    if not c1.values[0] == c2.values[0]: continue # same value?
                if type(c1) == Orange.core.ValueFilter_continuous:
                    if not c1.oper == c2.oper: continue # same operator?
                    if not c1.ref == c2.ref: continue #same threshold?
                found = True
                break
            except:
                pass
        if not found:
            return False
    return True
    
    
class AllContinuousValues(Orange.core.RuleBeamRefiner):
    """
    Selector adder, this class is a rule refiner:
      - it creates a condition for every continuous value.
      - it works with rule algorithms that do not use covering 
        (then you need to set self.must_cover)
    """
    def set_initial_cover(self, all_cover, seed_traces, 
                          covered_traces, parent_examples):
        self.all_cover = all_cover
        self.must_cover = all_cover
        self.seed_traces = seed_traces
        self.covered_traces = covered_traces
        self.parent_examples = parent_examples
        
        if not seed_traces:
            self.use_traces = False
        else:
            self.use_traces = True
        
    def remove_cover(self, rule):
        """ Compute traces covered by rule.
        
        We have to types of active traces:
        * seed_traces: are traces that are covered by this rule only. 
        * covered_traces: are all active traces covered by the rule; 
          these traces are ... from the root node. 
        """
        
        """straces = set()
        for d in rule(self.must_cover):
            straces |= set(str(d["trace"]).strip().split(";")) & self.seed_traces
        rule.setattr("traces", traces)"""    
        
        if self.use_traces:
            straces = set()
            new_to_cover = Orange.data.Table(self.must_cover.domain)
            for d in self.must_cover:
                tr = set(str(d["trace"]).strip().split(";")) & self.seed_traces
                if (tr - rule.seed_traces):
                    new_to_cover.append(d)
                    straces |= (tr - rule.seed_traces)
            self.must_cover = new_to_cover
            self.seed_traces = straces
        else:
            self.must_cover = rule.filter(self.must_cover, negate=True)
            
         
    def compute_traces(self, rule):            
        rule_mc = rule(self.must_cover)
        if len(rule_mc) == 0:
            return False
        
        if self.use_traces:
            # update which traces were covered (from active examples)
            straces, ctraces = set(), set()
            for d in rule_mc:
                straces |= set(str(d["trace"]).strip().split(";")) & self.seed_traces
            for d in rule.examples:
                ctraces |= set(str(d["trace"]).strip().split(";")) & self.covered_traces
            rule.setattr("seed_traces", straces)                    
            rule.setattr("covered_traces", ctraces)
        # recompute coverage; needed in WeightedMEstimateEvaluator
        rule.setattr("coverage", float(len(rule(self.all_cover))) / len(self.all_cover))
        # compute coverage of parent rule
        if self.parent_examples:
            pcoverage = rule(self.parent_examples)
            pdist = Orange.statistics.distribution.Distribution(
                       pcoverage.domain.classVar, pcoverage)
            rule.setattr("pdist", pdist)
        return True
        
    def __call__(self, oldRule, data, weight_id, target_class= -1):
        new_rules = Orange.core.RuleList()
        for i, at in enumerate(data.domain.attributes):
            # discrete attribute
            if at.varType == Orange.feature.Type.Discrete:
                for v in at.values:
                    tempRule = oldRule.clone()
                    tempRule.parentRule = oldRule
                    tempRule.filter.conditions.append(
                        Orange.data.filter.ValueFilterDiscrete(
                            position=i,
                            values=[Orange.data.Value(at, v)],
                            acceptSpecial=0))
                    tempRule.filterAndStore(oldRule.examples, 
                                            weight_id, target_class)
                    if self.must_cover and not self.compute_traces(tempRule):
                        continue
                    if len(tempRule.examples) > 0:
                        new_rules.append(tempRule)
            # continuous
            elif at.varType == Orange.feature.Type.Continuous:
                # find all possible example values
                values = set()
                for e in oldRule.examples:
                    if not e[at].isSpecial():
                        values.add(float(e[at]))
                # for each value make a condition
                for v in values:
                    # create a condition with Less or Equal
                    tempRule = oldRule.clone()
                    tempRule.parentRule = oldRule
                    tempRule.filter.conditions.append(
                        Orange.data.filter.ValueFilterContinuous(
                            position=i, oper=Orange.data.filter.ValueFilter.Less,
                            ref=v, acceptSpecial=0))
                    tempRule.filterAndStore(oldRule.examples, 
                                            weight_id, target_class)
                    if (len(tempRule.examples) > 0 and 
                        (not self.must_cover or self.compute_traces(tempRule))):
                        new_rules.append(tempRule)
                    # create a condition with Greater
                    tempRule = oldRule.clone()
                    tempRule.parentRule = oldRule
                    tempRule.filter.conditions.append(
                        Orange.data.filter.ValueFilterContinuous(
                            position=i, oper=Orange.data.filter.ValueFilter.Greater,
                            ref=v, acceptSpecial=0))
                    tempRule.filterAndStore(oldRule.examples, weight_id, target_class)
                    if (len(tempRule.examples) > 0 and 
                        (not self.must_cover or self.compute_traces(tempRule))):
                        new_rules.append(tempRule)
        return new_rules    

class AttValidator(Orange.core.RuleValidator):
    """ A class that validates whether a rule can be selected or not. """
    def __init__(self, att_alpha=1.0, only_positive=False, min_coverage=20, max_conditions=10, 
                 min_quality=0, only_pure = False, min_traces=0, min_seed_traces=0):
        self.only_positive = only_positive
        self.min_coverage = min_coverage
        self.max_conditions = max_conditions
        self.only_pure = only_pure
        self.min_quality = min_quality
        self.min_traces = min_traces
        self.min_seed_traces = min_seed_traces
        self.att_val = Orange.core.RuleValidator_LRS(alpha=att_alpha)
        self.all_traces = set()
         
    def __call__(self, rule, data, weight_id, target_class, apriori):
        # is the rule too long?
        if len(rule.filter.conditions) > self.max_conditions:
            return False
        # should we learn only rules with pure distributions?
        if self.only_pure and rule.classDistribution[target_class] < rule.classDistribution.abs:
            return False
        # is accuracy not better than min quality
        if rule.classDistribution[target_class] / rule.classDistribution.abs < self.min_quality:
            return False
        # is rule coverage too low?
        if rule.classDistribution.abs < self.min_coverage:
            return False
        # should rule have only positive conditions?
        if self.only_positive:
            for c in rule.filter.conditions:
                if isinstance(c, Orange.data.filter.ValueFilterDiscrete):
                    for v in c.values:
                        if str(rule.examples.domain[c.position].values[int(v)]) != "yes":
                            return False        
                if isinstance(c, Orange.data.filter.ValueFilterContinuous):
                    if (c.oper != Orange.data.filter.ValueFilter.Less and 
                        c.oper != Orange.data.filter.ValueFilter.LessEqual):
                        return False
        # must cover at least min_traces and at least one active trace
        if self.min_traces > 1 and len(rule.covered_traces) < self.min_traces:
            return False
        # must cover at least min seed traces
        if self.min_seed_traces > 1 and len(rule.seed_traces) < self.min_seed_traces:
            return False        
        # check significance of rule
        if (self.att_val.alpha < 1.0 and rule.parentRule is not None and 
            not self.att_val(rule, rule.parentRule.examples, weight_id, 0, rule.parentRule.classDistribution)):
            return False
        # otherwise return true
        return True
    
class GOLRuleLearner(RuleLearner):
    def __init__(self, m=2, att_alpha=1.0, min_coverage=20, max_conditions=10, 
                 max_rules=3, beam_width=5, min_quality = 0.0, only_positive=False, 
                 only_positive_implicit=False, min_traces = 0, min_seed_traces = 0, 
                 only_pure = False):
        self.rule_finder = BeamFinder()
        self.cover_and_remove = Orange.core.RuleCovererAndRemover_Default()
        self.rule_finder.ruleFilter = BeamFilter_Width(width=beam_width)
        self.rule_finder.evaluator = Orange.classification.rules.MEstimateEvaluator(m=m)
        self.rule_finder.validator = AttValidator(
                         att_alpha=att_alpha, min_coverage=min_coverage, 
                         max_conditions=max_conditions, only_positive=only_positive, 
                         only_pure = only_pure, min_quality = min_quality, 
                         min_traces = min_traces, min_seed_traces = min_seed_traces)
        self.rule_finder.rule_stoppingValidator = AttValidator(
                         att_alpha=att_alpha, min_coverage=min_coverage, 
                         max_conditions=max_conditions, only_positive=only_positive, 
                         only_pure = False, min_traces = min_traces, 
                         min_seed_traces = min_seed_traces)
        self.rule_finder.refiner = AllContinuousValues()
        self.max_rules = max_rules
        self.only_positive_implicit = only_positive_implicit

    def __call__(self, goal_node, weight_id=0, old_rules = None, n_rules = -1):
        # set conditions
        instances = goal_node.data
        # if has parent rule, then remove all examples covered by parent rule
        if goal_node.parent_rule:
            parent_examples = goal_node.parent_rule(instances)
            instances = goal_node.parent_rule.filter(instances, negate=1)
        else:
            parent_examples = None
            
        all_instances = instances
        rules = RuleList()
        self.target_class = 0
        # set examples that need to be covered (for the refiner)
        to_cover = Orange.data.Table(goal_node.to_solve)
        self.rule_finder.refiner.set_initial_cover(to_cover, goal_node.seed_traces, 
                                                   goal_node.covered_traces, 
                                                   parent_examples)
        if old_rules:
            for r in old_rules:
                self.rule_finder.refiner.remove_cover(r)
                instances, weight = self.cover_and_remove(r, instances, 
                                                          weight_id, 
                                                          self.target_class)
        while (self.rule_finder.refiner.must_cover and 
               (n_rules < 0 or n_rules > len(rules)) and 
               (self.max_rules < 0 or self.max_rules > len(rules))):
            # learn a rule
            rule = self.rule_finder(instances, weight_id, 
                                    self.target_class, 
                                    Orange.core.RuleList())
            if len(rule.filter.conditions) == 0:
                break
            print "learned rule:"
            apriori = Orange.statistics.distribution.Distribution(
                                    all_instances.domain.classVar, 
                                    all_instances)
            # Cover all positive examples! 
            # Necessary because coverage algorithm 
            # removes covered positive examples.
            rule.filterAndStore(all_instances, 0, 0)
            # re-evaluate rule
            fin_quality = self.rule_finder.evaluator(
                                    rule, all_instances, weight_id, 
                                    self.target_class, apriori)
            print (Orange.classification.rules.rule_to_string(rule), 
                  rule.quality, rule.coverage, fin_quality, 
                  getattr(rule, "pdist", None))
            #print ("seed traces: ", rule.seed_traces)
            rule.quality = fin_quality
            rules.append(rule)
            self.rule_finder.refiner.remove_cover(rule)
            # remove covered examples
            instances, weight = self.cover_and_remove(
                                    rule, instances, weight_id, 
                                    self.target_class)
        rules = sorted(rules, key = lambda r : r.quality, reverse = True)
        return rules
    
    def add_conditions_method(self, rules, weight_id, target_class):
        """ This method adds implicit conditions to rules. """
        for rule in rules:
            print "dist before adding implicit conditions: ", rule.classDistribution
            positions = set(c.position for c in rule.filter.conditions)
            # for each rule find best condition for each attribute
            self.rule_finder.refiner.must_cover = None
            spec_rules = self.rule_finder.refiner(rule, rule.examples, 
                                                  weight_id, target_class)
            spec_rules = [r for r in spec_rules 
                          if r.classDistribution[target_class] == rule.classDistribution[target_class] and 
                          len(r.filter.conditions) > len(rule.filter.conditions) and
                          (not self.only_positive_implicit or self.check_only_positive(r))]
            spec_rules = sorted(spec_rules, key = lambda r : r.classDistribution.abs)
            for sr in spec_rules:
                cond = sr.filter.conditions[-1]
                if cond.position not in positions:
                    rule.filter.conditions.append(cond)
                    positions.add(cond.position)
            rule.filterAndStore(rule.examples, 0, 0)
            print "dist after adding implicit conditions: ", rule.classDistribution
        return rules  
    
    def check_only_positive(self, rule):
        """ Check if rule contains only positive conditions. """
        for c in rule.filter.conditions:
            if isinstance(c, Orange.data.filter.ValueFilterDiscrete):
                for v in c.values:
                    if str(rule.examples.domain[c.position].values[int(v)]) != "yes":
                        return False        
            if isinstance(c, Orange.data.filter.ValueFilterContinuous):
                if (c.oper != Orange.data.filter.ValueFilter.Less and 
                    c.oper != Orange.data.filter.ValueFilter.LessEqual):
                    return False
        return True
